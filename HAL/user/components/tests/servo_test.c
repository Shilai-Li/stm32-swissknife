#include "uart_driver.h"
#include "servo.h"
#include "tim.h"
#include "motor_driver.h"
#include "algorithms/pid.h"

// Define Global Instances Here (moved from servo.c)
Motor_Handle_t myMotor;
PIDController posPID;

// --- Adapter Wrappers for Motor ---
void Adapter_Motor_Init(void *ctx) { 
    Motor_Init((Motor_Handle_t*)ctx); 
    Motor_Encoder_Init((Motor_Handle_t*)ctx);
}
void Adapter_Motor_Start(void *ctx) { Motor_Start((Motor_Handle_t*)ctx); }
void Adapter_Motor_Stop(void *ctx) { Motor_Stop((Motor_Handle_t*)ctx); }
void Adapter_Motor_SetSpeed(void *ctx, uint8_t speed) { Motor_SetSpeed((Motor_Handle_t*)ctx, speed); }
void Adapter_Motor_SetDirection(void *ctx, uint8_t dir) { Motor_SetDirection((Motor_Handle_t*)ctx, dir); }
int32_t Adapter_Motor_GetEncoder(void *ctx) { return Motor_GetEncoderCount((Motor_Handle_t*)ctx); }
void Adapter_Motor_ResetEncoder(void *ctx, int32_t val) { Motor_ResetEncoderCount((Motor_Handle_t*)ctx, val); }

static const Servo_MotorInterface_t motor_adapter = {
    .init = Adapter_Motor_Init,
    .start = Adapter_Motor_Start,
    .stop = Adapter_Motor_Stop,
    .set_speed = Adapter_Motor_SetSpeed,
    .set_direction = Adapter_Motor_SetDirection,
    .get_encoder = Adapter_Motor_GetEncoder,
    .reset_encoder = Adapter_Motor_ResetEncoder
};

// --- Adapter Wrappers for PID ---
void Adapter_PID_Init(void *ctx, float p, float i, float d, float limit, float ramp) {
    PID_Init((PIDController*)ctx, p, i, d, limit, ramp);
}
void Adapter_PID_Reset(void *ctx) { PID_Reset((PIDController*)ctx); }
float Adapter_PID_Compute(void *ctx, float error, float dt) { return PID_Compute((PIDController*)ctx, error, dt); }
void Adapter_PID_SetLimit(void *ctx, float limit) { ((PIDController*)ctx)->limit = limit; }

static const Servo_PIDInterface_t pid_adapter = {
    .init = Adapter_PID_Init,
    .reset = Adapter_PID_Reset,
    .compute = Adapter_PID_Compute,
    .set_limit = Adapter_PID_SetLimit
};

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
    // Note: Hardware config is now populated into the Motor_Handle_t *ctx via user code 
    // OR we need to populate myMotor struct before passing it?
    // Originally Servo_Init populated the Motor struct from cfg.
    // Now Servo_Init calls motor_if->init().
    
    // We need to configure the myMotor struct manually here or pass the config to the adapter?
    // The adapter just blindly calls Motor_Init with the ctx.
    // So 'myMotor' needs to be configured BEFORE Init is called,
    // OR Motor_Init uses the fields values.
    
    // Let's populate myMotor directly here:
    myMotor.htim = &htim1;
    myMotor.channel = TIM_CHANNEL_1;
    myMotor.pwm_period = 99;
    myMotor.en_port = GPIOC;
    myMotor.en_pin = GPIO_PIN_15;
    myMotor.dir_port = GPIOB;
    myMotor.dir_pin = GPIO_PIN_13;
    myMotor.htim_enc = &htim2;
    
    // Config for PID/Logic
    cfg.kp = servo_kp;
    cfg.ki = servo_ki;
    cfg.kd = servo_kd;
    cfg.pid_limit = servo_pid_limit;
    cfg.pid_ramp = 1000.0f;
    cfg.auto_start = 1;

    // Call Init with Adapters
    (void)Servo_InitInstance(&servo0, 
                             &myMotor, &motor_adapter,
                             &posPID, &pid_adapter,
                             &servo, &cfg);

    // Show help on startup
    UART_Debug_Printf("=== System Ready ===\r\n");
    UART_Debug_Printf("Type 'H' for help, or enter a command:\r\n");
    UART_Debug_Printf("> ");

    uint32_t last_status_print = 0;
    uint32_t last_ok_print = 0;
    uint8_t was_moving = 0;
    int32_t last_reported_target = 0;
    uint8_t has_reported_target = 0;

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
            int32_t tgt = (int32_t)servo.target_pos;
            int32_t pos = (int32_t)servo.actual_pos;
            int32_t diff = pos - tgt;
            if (diff < 0) diff = -diff;

            if (diff <= SERVO_POS_TOLERANCE) {
                if ((!has_reported_target || tgt != last_reported_target) && (HAL_GetTick() - last_ok_print > 200)) {
                    last_ok_print = HAL_GetTick();
                    last_reported_target = tgt;
                    has_reported_target = 1;
                    was_moving = 0;
                    UART_Debug_Printf("[OK] Target:%ld Pos:%ld\r\n> ", tgt, pos);
                }
            }
        }
        
        // Periodic status update (only when moving)
        if (!servo.is_at_target && (HAL_GetTick() - last_status_print > 1500)) {
            last_status_print = HAL_GetTick();
            UART_Debug_Printf("[MOVING] Pos:%ld Tgt:%ld\r\n",
                (int32_t)servo.actual_pos,
                (int32_t)servo.target_pos);
        }
    }
}