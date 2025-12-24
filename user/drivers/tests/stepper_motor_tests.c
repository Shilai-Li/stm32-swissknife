#include "main.h"
#include "drivers/motor/stepper_motor.h"
#include "drivers/system/delay.h"
#include "drivers/communication/uart.h"
#include <stdio.h>

/* =================================================================
 * Configuration Guide
 * =================================================================
 * 1. Timer: You MUST configure a Hardware Timer in CubeMX (e.g., TIM1 or TIM2).
 *    - Prescaler (PSC): Set so timer ticks at 1MHz (1us).
 *      (e.g., for 72MHz Clock, PSC = 71).
 *    - Counter Mode: Up.
 *    - Period (ARR): Max (65535 or 0xFFFFFFFF).
 * 2. GPIO: Configure STEP, DIR, EN pins as Output Push-Pull.
 * ================================================================= */

// External reference to the Timer Handle defined in tim.c
extern TIM_HandleTypeDef htim1; 
#define STEPPER_TIM_HANDLE  &htim1

// GPIO Configuration (Adjust these to match your board!)
#define STEP_PORT   GPIOA
#define STEP_PIN    GPIO_PIN_1
#define DIR_PORT    GPIOA
#define DIR_PIN     GPIO_PIN_2
#define EN_PORT     GPIOA
#define EN_PIN      GPIO_PIN_3

Stepper_HandleTypeDef hStepper;

void user_main(void)
{
    // Initialize Drivers
    UART_Init();
    Delay_Init();
    
    UART_Debug_Printf("\r\n=== Stepper Motor Test Start ===\r\n");

    // 1. Initialize Stepper Motor
    // Parameters: Handle, StepPort, StepPin, DirPort, DirPin, EnPort, EnPin, TimerHandle
    Stepper_Init(&hStepper, 
                 STEP_PORT, STEP_PIN,
                 DIR_PORT, DIR_PIN,
                 EN_PORT, EN_PIN,
                 STEPPER_TIM_HANDLE);
                 
    // 2. Configure Motion Parameters
    // Max Speed: 1000 steps/sec
    // Encryption: 500 steps/sec^2
    Stepper_SetSpeedConfig(&hStepper, 1000.0f, 500.0f);
    UART_Debug_Printf("Config: MaxSpeed=1000, Accel=500\r\n");
    
    // 3. Enable Motor (Engage Torque)
    Stepper_Enable(&hStepper, 1);
    
    // 4. Test 1: Blocking Move (Simple)
    long target = 2000;
    UART_Debug_Printf("Test 1: Blocking Move to %ld\r\n", target);
    Stepper_MoveTo(&hStepper, target);
    Stepper_RunToPosition(&hStepper); // This function blocks until finished
    UART_Debug_Printf("Arrived!\r\n");
    
    Delay_ms(1000);
    
    // 5. Test 2: Non-Blocking Loop
    UART_Debug_Printf("Test 2: Non-Blocking Loop (Bounce 0 <-> %ld)\r\n", target);
    
    // Move back to 0 first
    Stepper_MoveTo(&hStepper, 0);

    while (1)
    {
        // 核心：必须在主循环中高频调用 Run()
        // Stepper_Run 返回 1 表示还在运动，0 表示已到达
        uint8_t is_running = Stepper_Run(&hStepper);
        
        if (!is_running)
        {
            // 如果到达目标，等待 1 秒
            Delay_ms(1000);
            UART_Debug_Printf("Reverse Target...\r\n");
            
            // 切换目标位置
            if (hStepper.CurrentPos == 0) {
                Stepper_MoveTo(&hStepper, target);
            } else {
                Stepper_MoveTo(&hStepper, 0);
            }
        }
    }
}