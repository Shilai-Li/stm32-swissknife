/**
 * @file hc_sr04_tests.c
 * @brief HC-SR04 Driver Test Code
 * 
 * CubeMX Configuration Guide:
 * 1. Timer: Select a Timer (e.g. TIM1 or TIM3).
 *    - Prescaler: (APB_Freq_MHz) - 1.  Example: 72MHz -> Prescaler 71.
 *    - Result: Timer ticks at 1MHz (1us resolution).
 *    - ARR: Max (0xFFFF).
 * 
 * 2. GPIO:
 *    - TRIG: Output Push Pull
 *    - ECHO: Input
 */

#include "hc_sr04.h"
#include "uart.h"

// --- Configuration ---
extern TIM_HandleTypeDef htim1; // Use TIM1 for 1us timebase

#ifndef HCSR04_TRIG_PORT
#define HCSR04_TRIG_PORT GPIOA
#define HCSR04_TRIG_PIN  GPIO_PIN_1
#endif

#ifndef HCSR04_ECHO_PORT
#define HCSR04_ECHO_PORT GPIOA
#define HCSR04_ECHO_PIN  GPIO_PIN_2
#endif

HCSR04_HandleTypeDef hsonar;

void User_Entry(void)
{
    UART_Init();
    UART_Debug_Printf("\r\n--- HC-SR04 Test Start ---\r\n");

    // 1. Initialize
    // Ensure TIM1 is configured for 1us ticks in CubeMX!
    UART_Debug_Printf("Initializing Sensor...\r\n");
    HCSR04_Init(&hsonar, &htim1, 
                HCSR04_TRIG_PORT, HCSR04_TRIG_PIN,
                HCSR04_ECHO_PORT, HCSR04_ECHO_PIN);
    
    UART_Debug_Printf("Start Measuring (Loop 500ms)...\r\n");
    
    while (1) {
        float dist = HCSR04_Read(&hsonar);
        
        if (dist >= 0) {
            // Integer print trick for floats
            int d_int = (int)dist;
            int d_dec = (int)((dist - d_int) * 100);
            UART_Debug_Printf("Dist: %d.%02d cm\r\n", d_int, d_dec);
        } else {
            UART_Debug_Printf("Dist: Error/Out of Range\r\n");
        }
        
        HAL_Delay(200);
    }
}
