#include "motor_driver.h"
#include "tim.h"
#include "uart_driver.h"
#include "pid.h"
#include <stdio.h>
#include <string.h>
#include "usart.h"
#include "delay_driver.h"

/* PID Tuning Globals */
volatile float tune_Kp = 0.01f;
volatile float tune_Ki = 0.0f;
volatile float tune_Kd = 0.0f;
volatile int32_t tune_Target = 0;
volatile int32_t tune_StepAmplitude = 500;   // Default step amplitude
volatile uint16_t tune_StepInterval = 5000;  // Default step interval (ms) - increased to reduce heating
volatile uint8_t need_pid_reset = 0;           // Flag to reset PID state
uint8_t pid_rx_buf[64];

/* Global Control Variables (for ISR) */
Motor_Handle_t myMotor;
PID_Controller_t posPID;
volatile int32_t target_pos = 0;
volatile int32_t current_pos = 0;
volatile float filtered_pos = 0.0f;
volatile float control_output = 0.0f;

/* External Handles */
extern TIM_HandleTypeDef htim2; // Encoder
extern TIM_HandleTypeDef htim3; // 1ms Interrupt

/* Buffer for accumulating partial packets */
#define RX_BUFFER_SIZE 256
uint8_t rx_buffer[RX_BUFFER_SIZE];
uint16_t rx_buffer_head = 0;

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart->Instance == USART2) {
        UART_Debug_Printf(">>> RX Size: %d\r\n", Size);
        
        // 1. Append new data to rx_buffer
        if (rx_buffer_head + Size <= RX_BUFFER_SIZE) {
            memcpy(&rx_buffer[rx_buffer_head], pid_rx_buf, Size);
            rx_buffer_head += Size;
        } else {
            // Buffer overflow - reset
            UART_Debug_Printf("!!! Buffer overflow, resetting\r\n");
            rx_buffer_head = 0;
        }

        // 2. Parse packets
        // Protocol: Header(2) + Kp(4) + Ki(4) + Kd(4) + Target(4) + StepAmp(4) + StepInt(2) + Checksum(1) = 25 bytes
        int i = 0;
        int packets_found = 0;
        while (i <= (int)rx_buffer_head - 25) {
            if (rx_buffer[i] == 0xAA && rx_buffer[i+1] == 0xBB) {
                uint8_t *data = &rx_buffer[i+2];
                uint8_t checksum = 0;
                for (int j = 0; j < 22; j++) checksum += data[j];
                
                if (checksum == rx_buffer[i+24]) {
                    packets_found++;
                    UART_Debug_Printf(">>> Valid packet found at offset %d\r\n", i);
                    
                    // Valid Packet
                    // Use temporary variables for atomic-like assignment to volatiles
                    float temp_Kp, temp_Ki, temp_Kd;
                    int32_t temp_Target, temp_StepAmp;
                    uint16_t temp_StepInt;

                    memcpy(&temp_Kp, &data[0], 4);
                    memcpy(&temp_Ki, &data[4], 4);
                    memcpy(&temp_Kd, &data[8], 4);
                    memcpy(&temp_Target, &data[12], 4);
                    memcpy(&temp_StepAmp, &data[16], 4);
                    memcpy(&temp_StepInt, &data[20], 2);

                    tune_Kp = temp_Kp;
                    tune_Ki = temp_Ki;
                    tune_Kd = temp_Kd;
                    tune_Target = temp_Target;
                    tune_StepAmplitude = temp_StepAmp;
                    tune_StepInterval = temp_StepInt;
                    
                    need_pid_reset = 1; // Request PID reset in main loop/interrupt

                    
                    int kp_int = (int)tune_Kp, kp_dec = (int)((tune_Kp - kp_int) * 100);
                    int ki_int = (int)tune_Ki, ki_dec = (int)((tune_Ki - ki_int) * 100);
                    int kd_int = (int)tune_Kd, kd_dec = (int)((tune_Kd - kd_int) * 100);
                    if (kp_dec < 0) kp_dec = -kp_dec;
                    if (ki_dec < 0) ki_dec = -ki_dec;
                    if (kd_dec < 0) kd_dec = -kd_dec;
                    UART_Debug_Printf(">>> PID Updated: Kp=%d.%02d Ki=%d.%02d Kd=%d.%02d Tgt=%ld\r\n", 
                        kp_int, kp_dec, ki_int, ki_dec, kd_int, kd_dec, tune_Target);

                    // Move index past this packet
                    i += 25;
                    continue; 
                } else {
                    UART_Debug_Printf("!!! Checksum fail at %d (calc=%d, recv=%d)\r\n", 
                        i, checksum, rx_buffer[i+24]);
                }
            }
            i++;
        }

        if (packets_found == 0) {
            UART_Debug_Printf("!!! No valid packets in buffer\r\n");
        }

        // 3. Compact buffer
        if (i > 0) {
            int remaining = rx_buffer_head - i;
            if (remaining > 0) {
                memmove(rx_buffer, &rx_buffer[i], remaining);
            }
            rx_buffer_head = remaining;
        }

        // 4. Restart Reception
        HAL_UARTEx_ReceiveToIdle_DMA(huart, pid_rx_buf, sizeof(pid_rx_buf));
    }
}

/* Debug snapshot variables (written by ISR, read by main loop) */
volatile float debug_tune_kp_snapshot = 0.0f;
volatile float debug_pid_kp_snapshot = 0.0f;
volatile uint8_t debug_reset_executed = 0;
volatile uint8_t debug_snapshot_ready = 0;

/* 1ms Timer Interrupt Callback */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    // Call Delay Driver Callback (for micros() support)
    Delay_TIM_PeriodElapsedCallback(htim);

    if (htim->Instance == TIM3) {
        static uint32_t debug_counter = 0;
        static int32_t last_target = 0;
        debug_counter++;
        
        // Update PID params
        posPID.Kp = tune_Kp;
        posPID.Ki = tune_Ki;
        posPID.Kd = tune_Kd;
        
        // Reset PID when parameters change
        if (need_pid_reset) {
            PID_Reset(&posPID);
            Motor_ResetEncoderCount(&myMotor, 0);  // Clear encoder accumulation
            current_pos = 0;
            filtered_pos = 0.0f;
            target_pos = 0;
            last_target = 0;
            need_pid_reset = 0;
            debug_reset_executed = 1;
        }
        
        // Manual Target Override
        if (tune_Target != 0) {
            target_pos = tune_Target;
        }
        
        // Reset encoder when target changes significantly (prevent unbounded drift)
        if (target_pos != last_target) {
            int32_t target_change = target_pos - last_target;
            if (target_change > 100 || target_change < -100) {
                // Reset encoder to current target
                Motor_ResetEncoderCount(&myMotor, target_pos);
                current_pos = target_pos;
                filtered_pos = (float)target_pos;
                posPID.integral = 0.0f;  // Clear integral on large target changes
                posPID.prevMeasurement = (float)target_pos;  // Update derivative baseline
            }
            last_target = target_pos;
        }
        
        // Create debug snapshot every 1000ms (don't print in ISR!)
        if (debug_counter % 1000 == 0) {
            debug_tune_kp_snapshot = tune_Kp;
            debug_pid_kp_snapshot = posPID.Kp;
            debug_snapshot_ready = 1;
        }

        // Read Encoder
        current_pos = Motor_GetEncoderCount(&myMotor);
        
        // Filter Position (Optional, kept 1.0 for now)
        const float FILTER_ALPHA = 1.0f;
        filtered_pos = FILTER_ALPHA * current_pos + (1.0f - FILTER_ALPHA) * filtered_pos;

        // Compute PID (dt = 0.001s)
        control_output = PID_Compute(&posPID, (float)target_pos, filtered_pos, 0.001f);
        
        // Apply Output
        if (control_output >= 0) {
            Motor_SetDirection(&myMotor, 1);
            Motor_SetSpeed(&myMotor, (uint8_t)control_output);
        } else {
            Motor_SetDirection(&myMotor, 0);
            Motor_SetSpeed(&myMotor, (uint8_t)(-control_output));
        }
    }
}

void User_Entry(void)
{
    // UART_Init(); // Conflict with DMA mode

    
    extern UART_HandleTypeDef huart2;
    HAL_UART_AbortReceive(&huart2);
    HAL_UARTEx_ReceiveToIdle_DMA(&huart2, pid_rx_buf, sizeof(pid_rx_buf));

    UART_Debug_Printf("=== PID 1ms Interrupt Mode ===\r\n");

    // GPIO Init
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    // PB13 - DIR
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // PC15 - EN
    GPIO_InitStruct.Pin = GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    // Motor Config
    myMotor.htim = &htim1;
    myMotor.channel = TIM_CHANNEL_1;
    myMotor.en_port = GPIOC;
    myMotor.en_pin = GPIO_PIN_15;
    myMotor.dir_port = GPIOB;
    myMotor.dir_pin = GPIO_PIN_13;
    myMotor.pwm_period = 99;
    myMotor.htim_enc = &htim2;

    Motor_Init(&myMotor);
    Motor_Encoder_Init(&myMotor);
    Motor_ResetEncoderCount(&myMotor, 0);
    Motor_Start(&myMotor);

    // PID Init
    PID_Init(&posPID, tune_Kp, tune_Ki, tune_Kd, 95.0f, 100.0f, 2.0f);  // Reduced deadzone
    posPID.lpfBeta = 0.1f;

    // Start 1ms Interrupt
    HAL_TIM_Base_Start_IT(&htim3);

    uint32_t print_time = 0;
    uint32_t step_change_time = 0;

    while (1) {
        uint32_t now = HAL_GetTick();

        // Print Debug Info (50ms)
        if (now - print_time >= 50) {
            print_time = now;
            
            char buf[200];
            int val_int = (int)control_output;
            int val_dec = (int)((control_output - val_int) * 100);
            if (val_dec < 0) val_dec = -val_dec;

            // PID Coefficients with 3 decimal places
            int kp_int = (int)posPID.Kp;
            int kp_dec = (int)((posPID.Kp - kp_int) * 1000);
            if (kp_dec < 0) kp_dec = -kp_dec;
            
            int ki_int = (int)posPID.Ki;
            int ki_dec = (int)((posPID.Ki - ki_int) * 1000);
            if (ki_dec < 0) ki_dec = -ki_dec;
            
            int kd_int = (int)posPID.Kd;
            int kd_dec = (int)((posPID.Kd - kd_int) * 1000);
            if (kd_dec < 0) kd_dec = -kd_dec;
            
            // Error and PID components
            int32_t error = target_pos - current_pos;
            float p_term = posPID.Kp * error;
            float i_term = posPID.Ki * posPID.integral;
            float d_term = posPID.Kd * posPID.dTerm;
            
            int p_int = (int)p_term;
            int p_frac = (int)((p_term - p_int) * 100);
            if (p_frac < 0) p_frac = -p_frac;
            
            int i_int = (int)i_term;
            int i_frac = (int)((i_term - i_int) * 100);
            if (i_frac < 0) i_frac = -i_frac;
            
            int integ_int = (int)posPID.integral;
            int integ_frac = (int)((posPID.integral - integ_int) * 10);
            if (integ_frac < 0) integ_frac = -integ_frac;

            snprintf(buf, sizeof(buf), "T:%ld C:%ld E:%ld | Kp:%d.%03d Ki:%d.%03d Kd:%d.%03d | P:%d.%02d I:%d.%02d Int:%d.%01d | O:%d.%02d\r\n", 
                target_pos, current_pos, error,
                kp_int, kp_dec, ki_int, ki_dec, kd_int, kd_dec,
                p_int, p_frac, i_int, i_frac, integ_int, integ_frac,
                val_int, val_dec);
            UART_Send(UART_CHANNEL_2, (uint8_t*)buf, strlen(buf));
        }
        
        // Print debug snapshot from ISR
        if (debug_snapshot_ready) {
            debug_snapshot_ready = 0;
            int tune_int = (int)debug_tune_kp_snapshot;
            int tune_dec = (int)((debug_tune_kp_snapshot - tune_int) * 100);
            if (tune_dec < 0) tune_dec = -tune_dec;
            
            int pid_int = (int)debug_pid_kp_snapshot;
            int pid_dec = (int)((debug_pid_kp_snapshot - pid_int) * 100);
            if (pid_dec < 0) pid_dec = -pid_dec;
            
            UART_Debug_Printf("### ISR: tune_Kp=%d.%02d -> posPID.Kp=%d.%02d\r\n",
                tune_int, tune_dec, pid_int, pid_dec);
        }
        
        // Print reset notification
        if (debug_reset_executed) {
            debug_reset_executed = 0;
            UART_Debug_Printf("### PID RESET executed\r\n");
        }

        // Step Response Test (configurable interval and amplitude)
        if (tune_Target == 0 && tune_StepInterval > 0 && now - step_change_time > tune_StepInterval) {
            step_change_time = now;
            if (target_pos == 0) {
                target_pos = tune_StepAmplitude;
                UART_Debug_Printf(">>> Step UP to %ld\r\n", tune_StepAmplitude);
            } else {
                target_pos = 0;
                UART_Debug_Printf(">>> Step DOWN\r\n");
            }
        }
    }
}