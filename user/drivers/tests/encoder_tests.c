/**
 * @file encoder_tests.c
 * @brief Encoder Drivers Test Code (Knob + Motor)
 * 
 * CubeMX Config:
 * 1. TIM4 -> Encoder Mode (Combined Channels 1+2).
 *    - Used for Motor Encoder typically.
 * 2. TIM2 -> Encoder Mode.
 *    - Used for Knob.
 */

#include "encoder_knob.h"
#include "encoder_motor.h"
#include "uart.h"

// --- Configuration ---
extern TIM_HandleTypeDef htim1; // Trying TIM1 for Knob for validation 
extern TIM_HandleTypeDef htim3; // Trying TIM3 for Motor

// Instances
Encoder_Knob_HandleTypeDef hknob;
Encoder_Motor_HandleTypeDef hmotor;

void User_Entry(void)
{
    UART_Init();
    UART_Debug_Printf("\r\n--- Encoder Test Start ---\r\n");

    // 1. Init Knob (UI)
    // Assume TIM1 is connected to EC11
    UART_Debug_Printf("Init Knob on TIM1...\r\n");
    Encoder_Knob_Init(&hknob, &htim1);
    hknob.UseVelocity = 1; // Enable acceleration

    // 2. Init Motor (Motion)
    // Assume TIM3 connected to Motor Encoder (CPR=2000 for example)
    UART_Debug_Printf("Init Motor on TIM3 (2000 CPR)...\r\n");
    Encoder_Motor_Init(&hmotor, &htim3, 2000);

    UART_Debug_Printf("Looping... Twist knob or turn motor.\r\n");
    
    while (1) {
        // --- Update Knob --- 
        // Must be called reasonably fast for smooth feel
        int16_t delta = Encoder_Knob_Update(&hknob);
        if (delta != 0) {
            UART_Debug_Printf("Knob: Delta=%d Pos=%ld\r\n", delta, hknob.Position);
        }

        // --- Update Motor ---
        // Typically called in 1ms/10ms interrupt, but polling here for test
        Encoder_Motor_Update(&hmotor);
        
        // Print Motor Stats every 500ms
        static uint32_t last_print = 0;
        if (HAL_GetTick() - last_print > 500) {
            float rpm = Encoder_Motor_GetSpeed(&hmotor);
            int64_t count = Encoder_Motor_GetCount(&hmotor);
            
            // Only print if moving or count non-zero
            if (count != 0 || rpm > 1.0f || rpm < -1.0f) {
                // Print float via int parts
                int r_i = (int)rpm;
                int r_d = abs((int)((rpm - r_i) * 10));
                
                // Print long long? %lld might not be supported in minimal libc. 
                // Cast to long (32-bit) for display if small range, or split.
                UART_Debug_Printf("Motor: Count=%ld RPM=%d.%d\r\n", (long)count, r_i, r_d);
            }
            last_print = HAL_GetTick();
        }
        
        HAL_Delay(10); // 100Hz Loop
    }
}
