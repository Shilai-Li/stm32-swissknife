#include "uart_driver.h"
#include "servo.h"
#include "tim.h"

void User_Entry(void)
{
    UART_Init();

    UART_Debug_Printf("\r\n=== Motor Position Control System ===\r\n");
    UART_Debug_Printf("Encoder: 360 pulses per revolution\r\n");
    UART_Debug_Printf("Conversion: 1 degree = 1 encoder pulse\r\n\r\n");

    // Initialize Servo System
    static Servo_Handle_t servo0;

    servo0.debug_counter = &debug_counter;
    servo0.debug_last_pwm = &debug_last_pwm;
    servo0.enabled = &servo_enabled;
    servo0.error_state = &servo_error_state;

    Servo_Config_t cfg;
    cfg.pwm_htim = &htim1;
    cfg.pwm_channel = TIM_CHANNEL_1;
    cfg.pwm_period = 99;
    cfg.en_port = GPIOC;
    cfg.en_pin = GPIO_PIN_15;
    cfg.dir_port = GPIOB;
    cfg.dir_pin = GPIO_PIN_13;
    cfg.enc_htim = &htim2;
    cfg.kp = servo_kp;
    cfg.ki = servo_ki;
    cfg.kd = servo_kd;
    cfg.pid_limit = servo_pid_limit;
    cfg.pid_ramp = 1000.0f;
    cfg.auto_start = 1;

    (void)Servo_InitInstance(&servo0, &myMotor, &posPID, &servo, &cfg);

    // Show help on startup
    UART_Debug_Printf("=== System Ready ===\r\n");
    UART_Debug_Printf("Type 'H' for help, or enter a command:\r\n");
    UART_Debug_Printf("> ");

    uint32_t last_status_print = 0;
    uint8_t was_moving = 0;

    while (1) {
        // Check for error state
        if (servo_error_state) {
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
        Poll_UART_Commands();
        
        // Print status when movement completes
        if (!servo.is_at_target) {
            was_moving = 1;
        }
        else if (was_moving) {
            was_moving = 0;
            UART_Debug_Printf("[OK] Reached target: %ld deg\r\n> ", 
                (int32_t)servo.actual_pos);
        }
        
        // Periodic status update (every 500ms, only when moving)
        if (!servo.is_at_target && (HAL_GetTick() - last_status_print > 500)) {
            last_status_print = HAL_GetTick();
            // Use integer format for embedded printf compatibility
            UART_Debug_Printf("[MOVING] Pos:%ld Tgt:%ld Vel:%ld PWM:%ld\r\n", 
                (int32_t)servo.actual_pos, 
                (int32_t)servo.target_pos,
                (int32_t)servo.setpoint_vel,
                (int32_t)debug_last_pwm);
        }
    }
}