#include "motor_driver.h"
#include "tim.h"
#include "uart_driver.h"
#include "pid.h"
#include <stdio.h>
#include <string.h>
#include "usart.h"
#include "delay_driver.h"

/* PID Tuning Globals */
volatile float tune_Kp = 0.02f;
volatile float tune_Ki = 0.0f;
volatile float tune_Kd = 0.1f;
volatile int32_t tune_Target = 0;
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

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart->Instance == USART2) {
        // Search for header 0xAA 0xBB
        for (int i = 0; i <= (int)Size - 19; i++) {
            if (pid_rx_buf[i] == 0xAA && pid_rx_buf[i+1] == 0xBB) {
                uint8_t *data = &pid_rx_buf[i+2];
                uint8_t checksum = 0;
                for (int j = 0; j < 16; j++) checksum += data[j];
                
                if (checksum == pid_rx_buf[i+18]) {
                    memcpy((void*)&tune_Kp, &data[0], 4);
                    memcpy((void*)&tune_Ki, &data[4], 4);
                    memcpy((void*)&tune_Kd, &data[8], 4);
                    memcpy((void*)&tune_Target, &data[12], 4);
                }
            }
        }
        HAL_UARTEx_ReceiveToIdle_DMA(huart, pid_rx_buf, sizeof(pid_rx_buf));
    }
}

/* 1ms Timer Interrupt Callback */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    // Call Delay Driver Callback (for micros() support)
    Delay_TIM_PeriodElapsedCallback(htim);

    if (htim->Instance == TIM3) {
        // Update PID params
        posPID.Kp = tune_Kp;
        posPID.Ki = tune_Ki;
        posPID.Kd = tune_Kd;
        
        // Manual Target Override
        if (tune_Target != 0) {
            target_pos = tune_Target;
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
    UART_Init();
    
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
    PID_Init(&posPID, tune_Kp, tune_Ki, tune_Kd, 95.0f, 100.0f, 10.0f);
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
            uint32_t raw_cnt = __HAL_TIM_GET_COUNTER(&htim2);
            
            char buf[96];
            int val_int = (int)control_output;
            int val_dec = (int)((control_output - val_int) * 100);
            if (val_dec < 0) val_dec = -val_dec;

            snprintf(buf, sizeof(buf), "T:%ld C:%ld RAW:%lu Out:%d.%02d\r\n", 
                target_pos, current_pos, raw_cnt,
                val_int, val_dec);
            UART_Send(UART_CHANNEL_2, (uint8_t*)buf, strlen(buf));
        }

        // Step Response Test (1s interval)
        if (tune_Target == 0 && now - step_change_time > 1000) {
            step_change_time = now;
            if (target_pos == 0) {
                target_pos = 500;
                UART_Debug_Printf(">>> Step UP\r\n");
            } else {
                target_pos = 0;
                UART_Debug_Printf(">>> Step DOWN\r\n");
            }
        }
    }
}