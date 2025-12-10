#include "motor_driver.h"
#include "tim.h"
#include "uart_driver.h"
#include "pid.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "usart.h"
#include "delay_driver.h"

/*******************************************************************************
 * SERVO CONTROL PARAMETERS
 ******************************************************************************/
// Mechanical/Control Constraints
#define SERVO_MAX_VELOCITY      10000.0f  // Pulses per second
#define SERVO_ACCELERATION      50000.0f  // Pulses per second^2
#define SERVO_DECELERATION      50000.0f  
#define SERVO_POS_TOLERANCE     2         // Deadband in pulses
#define CONTROL_LOOP_FREQ       1000.0f   // 1kHz
#define CONTROL_DT              (1.0f/CONTROL_LOOP_FREQ)

// PID Gains (Position Loop)
// Note: These need tuning based on motor voltage and load
volatile float servo_kp = 2.5f;   
volatile float servo_ki = 0.05f;
volatile float servo_kd = 0.10f;
volatile float servo_pid_limit = 100.0f; // Max PWM Duty %

/*******************************************************************************
 * GLOBAL OBJECTS
 ******************************************************************************/
Motor_Handle_t myMotor;
PIDController posPID;

// Servo State
typedef struct {
    int32_t target_pos;         // Final desired position
    float   setpoint_pos;       // Current internal setpoint (Trajectory generator output)
    float   setpoint_vel;       // Current internal velocity
    int32_t actual_pos;         // Actual encoder position
    uint8_t is_at_target;       // Flag 1=Yes, 0=No
} Servo_State_t;

volatile Servo_State_t servo;

/* External Handles from main.c/tim.c */
extern TIM_HandleTypeDef htim1; // PWM (PA8)
extern TIM_HandleTypeDef htim2; // Encoder (PA0/PA1)
extern TIM_HandleTypeDef htim3; // 1ms Interrupt

/* Debug / Telemetry */
volatile uint32_t debug_counter = 0;
volatile float debug_last_pwm = 0.0f;

/* Servo Enable Flag - prevents motor output until fully initialized */
volatile uint8_t servo_enabled = 0;

/*******************************************************************************
 * HELPER FUNCTIONS
 ******************************************************************************/
void Servo_Init(void);
void Servo_SetTarget(int32_t position);
uint8_t Servo_IsAtTarget(void);
void Servo_Update_1kHz(void);
float Servo_ComputeTrajectory(void);

/*******************************************************************************
 * IMPLEMENTATION
 ******************************************************************************/

void Servo_Init(void) {
    // 0. FIRST: Ensure motor is disabled and PWM is 0 before doing anything
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, GPIO_PIN_RESET);  // EN pin LOW = motor disabled
    
    // 1. Initialize Global State
    memset((void*)&servo, 0, sizeof(Servo_State_t));
    
    // 2. Configure Hardware Abstraction Layer
    // PWM: TIM1 CH1 (PA8)
    // Encoder: TIM2 (PA0, PA1)
    
    myMotor.htim = &htim1;
    myMotor.channel = TIM_CHANNEL_1; 
    
    // Assumed Control Pins (Verify these match your board!)
    // If different, these are the only lines you need to change for GPIOs
    myMotor.dir_port = GPIOB;
    myMotor.dir_pin = GPIO_PIN_13;
    myMotor.en_port = GPIOC;
    myMotor.en_pin = GPIO_PIN_15;
    
    myMotor.pwm_period = 99; // Standard 0-100 scale (Assuming timer ARR=99)
    myMotor.htim_enc = &htim2;

    // 3. Initialize Drivers (motor stays disabled)
    Motor_Init(&myMotor);
    Motor_Encoder_Init(&myMotor);
    Motor_ResetEncoderCount(&myMotor, 0);  // Reset encoder to 0
    Motor_SetSpeed(&myMotor, 0);  // CRITICAL: Ensure PWM is 0

    // 4. Initialize PID and reset any accumulated state
    PID_Init(&posPID, servo_kp, servo_ki, servo_kd, servo_pid_limit, 1000.0f);
    PID_Reset(&posPID);  // Clear any residual integral
    
    // 5. CRITICAL: Set initial target to current position (should be 0 after reset)
    //    This ensures motor stays still at power-on
    int32_t initial_pos = Motor_GetEncoderCount(&myMotor);
    servo.target_pos = initial_pos;
    servo.setpoint_pos = (float)initial_pos;
    servo.actual_pos = initial_pos;
    servo.is_at_target = 1;  // Start as "at target" so motor doesn't move
    
    // 6. Enable servo_enabled BEFORE starting interrupt
    //    This way the ISR will see servo_enabled=1 from its first run
    servo_enabled = 1;
    
    // 7. Start Control Loop Interrupt
    HAL_TIM_Base_Start_IT(&htim3);
    
    // 8. Short delay to let a few interrupt cycles run with 0 error
    HAL_Delay(10);
    
    // 9. NOW enable the motor hardware (EN pin HIGH)
    Motor_Start(&myMotor);
}

void Servo_SetTarget(int32_t position) {
    servo.target_pos = position;
    servo.is_at_target = 0;
}

uint8_t Servo_IsAtTarget(void) {
    return servo.is_at_target;
}

/**
 * @brief  Time-Optimal Trajectory Generator (Square Root Controller)
 * @return New Position Setpoint
 */
float Servo_ComputeTrajectory(void) {
    // 1. Calculate Error Distance from Setpoint to Target
    float error = (float)servo.target_pos - servo.setpoint_pos;
    
    // 2. Determine Optimal Velocity to stop exactly at target
    // V_optimal = sqrt(2 * a * d) * sign(d)
    // We limit this by Max Velocity
    float desired_vel;
    float stop_dist_required = (servo.setpoint_vel * servo.setpoint_vel) / (2.0f * SERVO_DECELERATION);
    
    // Simple Trapezoidal Logic:
    // If we are close enough to start braking, we aim for V=0
    // A robust way is: TargetVel = K * Error, but clamped by Physics.
    
    // Let's use the robust "Square Root" method for time-optimality:
    // This allows max acceleration until we MUST brake.
    if (error == 0.0f) return servo.setpoint_pos;

    float direction = (error > 0.0f) ? 1.0f : -1.0f;
    float abs_error = fabsf(error);

    // Calculate max allowed velocity at this distance to be able to stop in time
    float max_vel_at_dist = sqrtf(2.0f * SERVO_DECELERATION * abs_error);
    
    // Clamp to global max velocity
    float target_vel = max_vel_at_dist;
    if (target_vel > SERVO_MAX_VELOCITY) target_vel = SERVO_MAX_VELOCITY;
    
    target_vel *= direction;

    // 3. Ramp current velocity towards target velocity
    float vel_diff = target_vel - servo.setpoint_vel;
    float max_vel_change = SERVO_ACCELERATION * CONTROL_DT;

    if (vel_diff > max_vel_change) {
        servo.setpoint_vel += max_vel_change;
    } else if (vel_diff < -max_vel_change) {
        servo.setpoint_vel -= max_vel_change;
    } else {
        servo.setpoint_vel = target_vel;
    }

    // 4. Integrate Velocity to get Position
    return servo.setpoint_pos + (servo.setpoint_vel * CONTROL_DT);
}

/**
 * @brief  1kHz Control Loop (Called from TIM3 ISR)
 */
void Servo_Update_1kHz(void) {
    // 0. Check if servo is enabled
    if (!servo_enabled) {
        Motor_SetSpeed(&myMotor, 0);  // Ensure motor stays stopped
        return;
    }
    
    // 1. Read Actual Position
    servo.actual_pos = Motor_GetEncoderCount(&myMotor);

    // 2. Update Trajectory Setpoint
    servo.setpoint_pos = Servo_ComputeTrajectory();

    // 3. Compute Position Error (Setpoint vs Actual)
    float pos_error = servo.setpoint_pos - (float)servo.actual_pos;

    // 4. Check On Target Condition
    // We are at target if both error is low AND velocity is near zero
    if (fabsf(pos_error) < SERVO_POS_TOLERANCE && fabsf(servo.setpoint_vel) < 1.0f) {
        servo.is_at_target = 1;
        // When at target with small error, output 0 and skip PID
        Motor_SetSpeed(&myMotor, 0);
        return;  // Exit early - no need to run PID
    } 

    // 5. Compute PID Output
    // Check Deadband
    if (fabsf(pos_error) < 1.0f) pos_error = 0.0f;
    
    float pid_out = PID_Compute(&posPID, pos_error);
    debug_last_pwm = pid_out;

    // 6. Feedforward (Static Friction Compensation) - Optional
    // if (pos_error > 0) pid_out += 2.0f;
    // if (pos_error < 0) pid_out -= 2.0f;
    
    // 7. Apply to Motor
    uint8_t pwm_val = 0;
    float abs_out = fabsf(pid_out);
    
    if (abs_out > 100.0f) abs_out = 100.0f;
    pwm_val = (uint8_t)abs_out;
    
    // Deadzone check for motor hardware protection (prevent high freq PWM switching at 0-1%)
    if (pwm_val < 2) pwm_val = 0; 
    
    if (pid_out >= 0) {
        Motor_SetDirection(&myMotor, 1);
    } else {
        Motor_SetDirection(&myMotor, 0);
    }
    
    Motor_SetSpeed(&myMotor, pwm_val);
}


/* Interrupt Callback */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    // Ensure we only run this for TIM3
    if (htim->Instance == TIM3) {
        Servo_Update_1kHz();
        debug_counter++;
    }
}


/*******************************************************************************
 * ENCODER CONFIGURATION
 * Encoder: 360 pulses per revolution
 * Conversion: 1 degree = 1 pulse (360 pulses / 360 degrees)
 ******************************************************************************/
#define ENCODER_PULSES_PER_REV  360
#define DEGREES_TO_PULSES(deg)  (deg)  // 1:1 mapping for 360 pulse encoder

/*******************************************************************************
 * UART COMMAND PARSER
 ******************************************************************************/
#define CMD_BUFFER_SIZE 64
static char cmd_buffer[CMD_BUFFER_SIZE];
static uint8_t cmd_index = 0;

/**
 * @brief  Convert degrees to encoder pulses
 * @param  degrees: Target angle in degrees (can be negative)
 * @return Number of encoder pulses
 */
static inline int32_t Degrees_To_Pulses(int32_t degrees) {
    return degrees;  // 360 pulses/rev, so 1 degree = 1 pulse
}

/**
 * @brief  Convert encoder pulses to degrees
 * @param  pulses: Encoder pulse count
 * @return Angle in degrees
 */
static inline float Pulses_To_Degrees(int32_t pulses) {
    return (float)pulses;  // Direct 1:1 conversion
}

/**
 * @brief  Parse and execute UART commands
 * @note   Commands:
 *         - "G<degrees>" or "g<degrees>": Go to absolute position (e.g., G90, G-45)
 *         - "R<degrees>" or "r<degrees>": Rotate relative amount (e.g., R360, R-90)
 *         - "Z" or "z": Zero/reset encoder position
 *         - "S" or "s": Stop and hold current position
 *         - "H" or "h": Show help
 *         - "P" or "p": Print current position
 */
static void Process_Command(char* cmd) {
    if (cmd[0] == '\0') return;
    
    char cmd_type = cmd[0];
    int32_t value = 0;
    
    switch (cmd_type) {
        case 'G':
        case 'g':
            // Go to absolute position (degrees)
            value = atoi(&cmd[1]);
            UART_Debug_Printf("[CMD] Go to %ld degrees (%ld pulses)\r\n", value, Degrees_To_Pulses(value));
            Servo_SetTarget(Degrees_To_Pulses(value));
            break;
            
        case 'R':
        case 'r':
            // Rotate relative amount (degrees)
            value = atoi(&cmd[1]);
            {
                int32_t new_target = servo.target_pos + Degrees_To_Pulses(value);
                UART_Debug_Printf("[CMD] Rotate %ld degrees (new target: %ld pulses)\r\n", value, new_target);
                Servo_SetTarget(new_target);
            }
            break;
            
        case 'Z':
        case 'z':
            // Zero encoder
            UART_Debug_Printf("[CMD] Zeroing encoder position\r\n");
            Motor_ResetEncoderCount(&myMotor, 0);
            servo.target_pos = 0;
            servo.setpoint_pos = 0.0f;
            servo.setpoint_vel = 0.0f;
            servo.is_at_target = 1;
            break;
            
        case 'S':
        case 's':
            // Stop and hold current position
            {
                int32_t current = Motor_GetEncoderCount(&myMotor);
                UART_Debug_Printf("[CMD] Stop and hold at %ld pulses (%.1f degrees)\r\n", 
                    current, Pulses_To_Degrees(current));
                Servo_SetTarget(current);
            }
            break;
            
        case 'H':
        case 'h':
        case '?':
            // Help
            UART_Debug_Printf("\r\n=== Motor Position Control ===\r\n");
            UART_Debug_Printf("Encoder: 360 pulses/rev (1 degree = 1 pulse)\r\n");
            UART_Debug_Printf("Commands:\r\n");
            UART_Debug_Printf("  G<deg>  - Go to absolute position (e.g., G90, G-45, G360)\r\n");
            UART_Debug_Printf("  R<deg>  - Rotate relative (e.g., R90, R-180)\r\n");
            UART_Debug_Printf("  Z       - Zero encoder (set current position as 0)\r\n");
            UART_Debug_Printf("  S       - Stop and hold current position\r\n");
            UART_Debug_Printf("  P       - Print current position\r\n");
            UART_Debug_Printf("  H/?     - Show this help\r\n");
            UART_Debug_Printf("Examples: G90 (go to 90°), R360 (rotate one full turn)\r\n\r\n");
            break;
            
        case 'P':
        case 'p':
            // Print current position
            UART_Debug_Printf("[INFO] Position: %ld pulses = %.1f degrees\r\n", 
                servo.actual_pos, Pulses_To_Degrees(servo.actual_pos));
            UART_Debug_Printf("[INFO] Target: %ld pulses = %.1f degrees\r\n", 
                servo.target_pos, Pulses_To_Degrees(servo.target_pos));
            UART_Debug_Printf("[INFO] At target: %s\r\n", servo.is_at_target ? "Yes" : "No");
            break;
            
        default:
            UART_Debug_Printf("[ERROR] Unknown command: %c. Type 'H' for help.\r\n", cmd_type);
            break;
    }
}

/**
 * @brief  Non-blocking UART receive and command processing
 */
static void Poll_UART_Commands(void) {
    uint8_t rx_byte;
    
    // Try to receive a byte (non-blocking)
    if (HAL_UART_Receive(&huart2, &rx_byte, 1, 0) == HAL_OK) {
        // Echo the character
        HAL_UART_Transmit(&huart2, &rx_byte, 1, 10);
        
        if (rx_byte == '\r' || rx_byte == '\n') {
            // End of command
            if (cmd_index > 0) {
                cmd_buffer[cmd_index] = '\0';
                UART_Debug_Printf("\r\n");
                Process_Command(cmd_buffer);
                cmd_index = 0;
            }
        } else if (rx_byte == 0x7F || rx_byte == 0x08) {
            // Backspace
            if (cmd_index > 0) {
                cmd_index--;
                UART_Debug_Printf("\b \b");  // Erase character on terminal
            }
        } else if (cmd_index < CMD_BUFFER_SIZE - 1) {
            // Add to buffer
            cmd_buffer[cmd_index++] = (char)rx_byte;
        }
    }
}

/*******************************************************************************
 * MAIN ENTRY POINT
 ******************************************************************************/
void User_Entry(void)
{
    UART_Debug_Printf("\r\n=== Motor Position Control System ===\r\n");
    UART_Debug_Printf("Encoder: 360 pulses per revolution\r\n");
    UART_Debug_Printf("Conversion: 1 degree = 1 encoder pulse\r\n\r\n");

    // Initialize GPIO
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    // DIR: PB13, EN: PC15
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_15;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    // Initialize Servo System
    Servo_Init();

    // Show help on startup
    UART_Debug_Printf("=== System Ready ===\r\n");
    UART_Debug_Printf("Type 'H' for help, or enter a command:\r\n");
    UART_Debug_Printf("> ");

    uint32_t last_status_print = 0;
    uint8_t was_moving = 0;

    while (1) {
        // Poll for UART commands
        Poll_UART_Commands();
        
        // Print status when movement completes
        if (!servo.is_at_target) {
            was_moving = 1;
        } else if (was_moving) {
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