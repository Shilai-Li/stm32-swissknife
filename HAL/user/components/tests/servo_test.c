#include <stdlib.h>

#include "uart_driver.h"
#include "servo_port.h"

void User_Entry(void)
{
    UART_Init();

    UART_Debug_Printf("\r\n=== Motor Position Control System ===\r\n");
    UART_Debug_Printf("Encoder: 360 pulses per revolution\r\n");
    UART_Debug_Printf("Conversion: 1 degree = 1 encoder pulse\r\n\r\n");

    // Initialize Servo System
    static Servo_Handle_t myServo;

    // Use High-Level Port Init
    if (ServoPort_Init(&myServo) != SERVO_OK) {
        UART_Debug_Printf("[FATAL] Servo Init Failed!\r\n");
        while(1);
    }
    
    // Explicitly auto-start if needed (ServoPort_Init defaults to false usually to be safe, or we can set it there)
    Servo_Start(&myServo);

    // Show help on startup
    UART_Debug_Printf("=== System Ready ===\r\n");
    UART_Debug_Printf("Type 'H' for help, or enter a command:\r\n");
    UART_Debug_Printf("> ");

    uint32_t last_status_print = 0;
    uint32_t last_ok_print = 0;
    bool was_moving = false;
    int32_t last_reported_target = 0;
    bool has_reported_target = false;

    while (1) {
        // Check for error state
        if (myServo.error) {
            UART_Debug_Printf("\r\n[CRITICAL] MOTOR STALL/RUNAWAY DETECTED!\r\n");
            UART_Debug_Printf("PWM > 80%% but Position did not change for 500ms.\r\n");
            UART_Debug_Printf("Possible causes: 1. Motor unconnected 2. Encoder disconnected 3. Mechanical jam\r\n");
            UART_Debug_Printf("System Halted. Reset board to restart.\r\n");
            
            while(1) {
                // Blink LED or just hang
                HAL_Delay(1000);
            }
        }

        // Poll for UART commands
        // Note: Poll_UART_Commands internally uses the registered default instance (myServo)
        Poll_UART_Commands();
        
        // Check Status
        bool is_at_target = Servo_IsAtTarget(&myServo);
        
        // Print status when movement completes
        if (!is_at_target) {
            was_moving = true;
        }
        else if (was_moving) {
            int32_t tgt = myServo.state.target_pos;
            int32_t pos = myServo.state.actual_pos;
            int32_t diff = abs(pos - tgt);

            if (diff <= SERVO_POS_TOLERANCE) {
                if ((!has_reported_target || tgt != last_reported_target) && (HAL_GetTick() - last_ok_print > 200)) {
                    last_ok_print = HAL_GetTick();
                    last_reported_target = tgt;
                    has_reported_target = true;
                    was_moving = false;
                    UART_Debug_Printf("[OK] Target:%ld Pos:%ld\r\n> ", tgt, pos);
                }
            }
        }
        
        // Periodic status update (only when moving)
        if (!is_at_target && (HAL_GetTick() - last_status_print > 1500)) {
            last_status_print = HAL_GetTick();
            UART_Debug_Printf("[MOVING] Pos:%ld Tgt:%ld\r\n",
                myServo.state.actual_pos,
                myServo.state.target_pos);
        }
    }
}
