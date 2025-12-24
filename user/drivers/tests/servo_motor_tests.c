/**
 * @file servo_tests.c
 * @brief Servo Driver Test Code
 */

#include "servo_motor.h"
#include "uart.h"

// --- Configuration ---
// Requires a Timer (e.g. TIM1 or TIM2)
// Prescaler: F_BUS(MHz) - 1  -> 1MHz (1us tick)
// ARR: 20000 - 1             -> 20ms period (50Hz)
extern TIM_HandleTypeDef htim1; 

Servo_Motor_HandleTypeDef hservo1;

void user_main(void)
{
    UART_Init();
    UART_Debug_Printf("\r\n--- Servo Motor Test Start ---\r\n");

    // 1. Initialize
    // Assume Servo on TIM1 Channel 1 (PA8)
    UART_Debug_Printf("Init Servo on TIM1 CH1...\r\n");
    Servo_Motor_Init(&hservo1, &htim1, TIM_CHANNEL_1);
    
    // Optional: Calibrate for specific servo (e.g. MG996R often needs 500-2500)
    // Servo_Motor_SetLimits(&hservo1, 500, 2500, 180.0f);
    
    UART_Debug_Printf("Sweep Test...\r\n");
    
    while (1) {
        // 0 Degrees
        UART_Debug_Printf("0 deg\r\n");
        Servo_Motor_SetAngle(&hservo1, 0.0f);
        HAL_Delay(1000);
        
        // 90 Degrees
        UART_Debug_Printf("90 deg\r\n");
        Servo_Motor_SetAngle(&hservo1, 90.0f);
        HAL_Delay(1000);
        
        // 180 Degrees
        UART_Debug_Printf("180 deg\r\n");
        Servo_Motor_SetAngle(&hservo1, 180.0f);
        HAL_Delay(1000);
        
        // Smooth Sweep
        UART_Debug_Printf("Smooth Sweep...\r\n");
        for (int i=0; i<=180; i+=5) {
             Servo_Motor_SetAngle(&hservo1, (float)i);
             HAL_Delay(50);
        }
        for (int i=180; i>=0; i-=5) {
             Servo_Motor_SetAngle(&hservo1, (float)i);
             HAL_Delay(50);
        }
    }
}
