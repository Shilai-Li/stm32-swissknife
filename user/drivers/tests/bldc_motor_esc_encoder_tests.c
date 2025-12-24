#include "main.h"
#include "drivers/motor/bldc_motor_esc_encoder.h"
#include "drivers/communication/uart.h"
#include "drivers/system/delay.h" 
#include <stdio.h>

/* =========================================================================
 * Hardware Configuration (Adjust for your specific board!)
 * ========================================================================= */
// PWM Timer (e.g., TIM1) should be configured in CubeMX
extern TIM_HandleTypeDef htim1;

// Encoder Timer (e.g., TIM2) should be configured in Encoder Mode in CubeMX
extern TIM_HandleTypeDef htim2; 

#define PWM_TIM_HANDLE      &htim1
#define PWM_TIM_CHANNEL     TIM_CHANNEL_1

#define ENC_TIM_HANDLE      &htim2

// Motor Control Pins
#define MOTOR_EN_PORT       GPIOA
#define MOTOR_EN_PIN        GPIO_PIN_1
#define MOTOR_DIR_PORT      GPIOA
#define MOTOR_DIR_PIN       GPIO_PIN_2

/* =========================================================================
 * Test Logic
 * ========================================================================= */

Motor_Handle_t hMotor;

void user_main(void)
{
    // 1. Initialize System Drivers
    UART_Init();
    Delay_Init();
    
    UART_Debug_Printf("\r\n=== BLDC Motor Test Start ===\r\n");

    // 2. Configure Motor Handle
    hMotor.htim         = PWM_TIM_HANDLE;
    hMotor.channel      = PWM_TIM_CHANNEL;
    
    hMotor.en_port      = MOTOR_EN_PORT;
    hMotor.en_pin       = MOTOR_EN_PIN;
    
    hMotor.dir_port     = MOTOR_DIR_PORT;
    hMotor.dir_pin      = MOTOR_DIR_PIN;
    
    // Assumes CubeMX has configured Period/Prescaler. 
    // We can read it from the handle if initialized, or set manually if we know it.
    // Here we assume ARR is used as period.
    hMotor.pwm_period   = __HAL_TIM_GET_AUTORELOAD(PWM_TIM_HANDLE); 
    
    hMotor.htim_enc     = ENC_TIM_HANDLE;

    // 3. Initialize Motor Driver
    Motor_Init(&hMotor);
    Motor_Encoder_Init(&hMotor); // Start Encoder

    UART_Debug_Printf("Motor Initialized. Period: %d\r\n", hMotor.pwm_period);

    // 4. Start Motor
    Motor_Start(&hMotor);
    Motor_SetDirection(&hMotor, 1); // Forward

    int16_t speed = 0;
    int8_t  dir = 1;
    int32_t enc_count = 0;

    while (1)
    {
        // === Phase 1: Ramp Up ===
        UART_Debug_Printf("--> Ramping Up (Dir: %d)\r\n", dir);
        for (speed = 0; speed <= 50; speed += 5) // Limits to 50% for safety test
        {
            Motor_SetSpeed(&hMotor, speed);
            enc_count = Motor_GetEncoderCount(&hMotor);
            UART_Debug_Printf("Speed: %d%% | Enc: %ld\r\n", speed, enc_count);
            Delay_ms(200);
        }

        Delay_ms(1000);

        // === Phase 2: Ramp Down ===
        UART_Debug_Printf("--> Ramping Down\r\n");
        for (speed = 50; speed >= 0; speed -= 5)
        {
            Motor_SetSpeed(&hMotor, speed);
            enc_count = Motor_GetEncoderCount(&hMotor);
            UART_Debug_Printf("Speed: %d%% | Enc: %ld\r\n", speed, enc_count);
            Delay_ms(200);
        }
        
        Motor_SetSpeed(&hMotor, 0);
        Delay_ms(1000);

        // === Phase 3: Change Direction ===
        dir = !dir;
        UART_Debug_Printf("--> Changing Direction to %d\r\n", dir);
        Motor_SetDirection(&hMotor, dir);
        Delay_ms(500);
    }
}
