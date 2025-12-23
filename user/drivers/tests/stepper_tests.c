/**
 * @file stepper_tests.c
 * @brief Stepper Driver Test Code
 */

#include "stepper_motor.h"
#include "uart.h"

// --- Configuration ---
// Requires TIM1 for timebase (1us ticks) - Shared with HC-SR04 test if present
extern TIM_HandleTypeDef htim1; 

// Pin Config
#ifndef STEPPER_STEP_PORT
#define STEPPER_STEP_PORT GPIOA
#define STEPPER_STEP_PIN  GPIO_PIN_8
#endif

#ifndef STEPPER_DIR_PORT
#define STEPPER_DIR_PORT  GPIOA
#define STEPPER_DIR_PIN   GPIO_PIN_9
#endif

Stepper_HandleTypeDef hstepper;

void User_Entry(void)
{
    UART_Init();
    UART_Debug_Printf("\r\n--- Stepper Test Start ---\r\n");

    // 1. Initialize
    UART_Debug_Printf("Init Stepper (TIM1, A8/A9)...\r\n");
    Stepper_Init(&hstepper, 
                 STEPPER_STEP_PORT, STEPPER_STEP_PIN,
                 STEPPER_DIR_PORT, STEPPER_DIR_PIN,
                 NULL, 0, // No Enable Pin for test
                 &htim1);
                 
    // 2. Configure Motion
    // Max Speed: 1000 steps/sec
    // Accel: 500 steps/sec^2
    Stepper_SetSpeedConfig(&hstepper, 1000.0f, 500.0f);
    Stepper_Enable(&hstepper, 1);
    
    // 3. Simple Move (Blocking)
    UART_Debug_Printf("Move 1000 steps forward (Blocking)...\r\n");
    Stepper_Move(&hstepper, 1000);
    Stepper_RunToPosition(&hstepper);
    UART_Debug_Printf("Done.\r\n");
    HAL_Delay(500);
    
    // 4. Back and Forth (Non-Blocking)
    UART_Debug_Printf("Entering Bounce Mode (Non-Blocking)...\r\n");
    
    Stepper_MoveTo(&hstepper, 0); // Go back home
    
    while (1) {
        // Run() must be called as fast as possible
        uint8_t running = Stepper_Run(&hstepper);
        
        if (!running) {
             HAL_Delay(500); // Wait at ends
             
             // Flip Target
             if (hstepper.CurrentPos == 0) {
                 UART_Debug_Printf("Going to 2000...\r\n");
                 Stepper_MoveTo(&hstepper, 2000);
             } else {
                 UART_Debug_Printf("Going Home...\r\n");
                 Stepper_MoveTo(&hstepper, 0);
             }
        }
    }
}
