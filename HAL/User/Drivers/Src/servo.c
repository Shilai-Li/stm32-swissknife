#include "servo.h"
#include "tim.h"
#include "uart_driver.h"
#include "usart.h"
#include "delay_driver.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>

volatile float servo_kp = SERVO_KP_DEFAULT;
volatile float servo_ki = SERVO_KI_DEFAULT;
volatile float servo_kd = SERVO_KD_DEFAULT;
volatile float servo_pid_limit = SERVO_PID_LIMIT_DEFAULT;

Motor_Handle_t myMotor;
PIDController posPID;
volatile Servo_State_t servo;

/* External Handles from main.c/tim.c */
extern TIM_HandleTypeDef htim1; // PWM (PA8)
extern TIM_HandleTypeDef htim2; // Encoder (PA0/PA1)
extern TIM_HandleTypeDef htim3; // 1ms Interrupt

/* Debug / Telemetry */
volatile uint32_t debug_counter = 0;
volatile float debug_last_pwm = 0.0f;
volatile uint8_t servo_enabled = 0;
volatile uint8_t servo_error_state = 0; // 0=OK, 1=Stall/Runaway Detected

static uint32_t high_load_duration = 0;
static int32_t  load_start_pos = 0;

#define CMD_BUFFER_SIZE 64
static char cmd_buffer[CMD_BUFFER_SIZE];
static uint8_t cmd_index = 0;

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

/**
 * @brief  Set servo target position
 */
void Servo_SetTarget(int32_t position) {
    servo.target_pos = position;
    servo.is_at_target = 0;
}

/**
 * @brief  Check if servo is at target position
 */
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
    
    // Snap to target if error is very small to prevent jitter
    if (fabsf(error) < 0.5f) {
        servo.setpoint_vel = 0.0f;
        return (float)servo.target_pos;
    }

    float direction = (error > 0.0f) ? 1.0f : -1.0f;
    float abs_error = fabsf(error);

    // 2. Determine Optimal Velocity to stop exactly at target
    // Calculate max allowed velocity at this distance to be able to stop in time
    // V = sqrt(2 * a * d)
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
    
    float pid_out = PID_Compute(&posPID, pos_error, CONTROL_DT);
    debug_last_pwm = pid_out;

    // Safety: Check for runaway/stall condition
    Check_Runaway_Condition(pid_out, servo.actual_pos);
    if (servo_error_state) return; // Stop if error triggered

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

/**
 * @brief  Check for motor runaway/stall condition
 */
void Check_Runaway_Condition(float pid_output, int32_t current_pos) {
    // If PWM is high (absolute value > 80%) indicating high effort
    if (fabsf(pid_output) > 80.0f) {
        if (high_load_duration == 0) {
            // First tick of high load, record position
            load_start_pos = current_pos;
        }
        
        high_load_duration++;
        
        // If high load persists for 500ms (500 ticks @ 1kHz)
        if (high_load_duration > 500) {
            int32_t moved = abs(current_pos - load_start_pos);
            
            // If moved less than 10 pulses (about 10 degrees) in 500ms of FULL POWER
            // This clearly indicates a stall or encoder failure (runaway)
            if (moved < 10) {
                // FAILURE DETECTED
                servo_enabled = 0;       // Disable control
                servo_error_state = 1;   // Set error flag
                Motor_Stop(&myMotor);    // Kill hardware output immediately
            }
        }
    } else {
        // Load is normal, reset counter
        high_load_duration = 0;
    }
}

/**
 * @brief  Convert degrees to encoder pulses
 */
static inline int32_t Degrees_To_Pulses(int32_t degrees) {
    return degrees;  // 360 pulses/rev, so 1 degree = 1 pulse
}

/**
 * @brief  Convert encoder pulses to degrees
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
void Process_Command(char* cmd) {
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
            // DEBUG: Print what we are parsing
            UART_Debug_Printf("[DEBUG] Raw cmd: '%s', Parsing substring: '%s'\r\n", cmd, &cmd[1]);
            
            value = atoi(&cmd[1]);
            
            UART_Debug_Printf("[DEBUG] Parsed Value: %ld (0x%08lX)\r\n", value, value);

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
            UART_Debug_Printf("  E       - Test Encoder Readings (Continuous)\r\n");
            UART_Debug_Printf("  L<val>  - Set Max PWM Limit (0-100%%)\r\n");
            UART_Debug_Printf("  H/?     - Show this help\r\n");
            UART_Debug_Printf("Examples: G90 (go to 90°), R360 (rotate one full turn), L80 (limit power to 80%%)\r\n\r\n");
            UART_Debug_Printf("> ");
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

        case 'E':
        case 'e':
            // Enter Encoder Test Mode
            Test_Encoder_Readings();
            break;
            
            break;

        case 'L':
        case 'l':
            // Set Max PWM Limit (PID Limit)
            value = atoi(&cmd[1]);
            if (value < 0) value = 0;
            if (value > 100) value = 100;
            
            servo_pid_limit = (float)value;
            posPID.limit = servo_pid_limit;
            
            UART_Debug_Printf("[CMD] Set PWM Limit to %ld%%\r\n", value);
            break;
            
        default:
            UART_Debug_Printf("[ERROR] Unknown command: %c. Type 'H' for help.\r\n", cmd_type);
            UART_Debug_Printf("> ");
            break;
    }
}

/**
 * @brief  Non-blocking UART receive and command processing
 */
void Poll_UART_Commands(void) {
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

/**
 * @brief  Interrupt Callback - TIM3 Period Elapsed
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    // Ensure we only run this for TIM3
    if (htim->Instance == TIM3) {
        Servo_Update_1kHz();
        debug_counter++;
    }
}

/**
 * @brief Continuous Encoder Test Mode
 *        Loops and prints encoder values until 'q' is pressed.
 */
void Test_Encoder_Readings(void) {
    UART_Debug_Printf("\r\n=== Encoder Test Mode ===\r\n");
    UART_Debug_Printf("Controls:\r\n");
    UART_Debug_Printf("  'w' : Increase PWM (+10%%)\r\n");
    UART_Debug_Printf("  's' : Decrease PWM (-10%%)\r\n");
    UART_Debug_Printf("  ' ' : Stop (PWM 0)\r\n");
    UART_Debug_Printf("  'q' : Exit\r\n\r\n");

    int8_t current_pwm = 0;
    
    // Safety: Disable control loop
    servo_enabled = 0;
    HAL_Delay(10);
    
    // Explicitly enable motor hardware (driver EN pin)
    Motor_Start(&myMotor); 

    uint8_t rx_byte = 0;

    // Flush any pending data
    while(HAL_UART_Receive(&huart2, &rx_byte, 1, 0) == HAL_OK);

    while (1) {
        int32_t count = Motor_GetEncoderCount(&myMotor);
        int32_t deg_int = count; 
        
        UART_Debug_Printf("\r[ENC] PWM: %3d%% | Count: %6ld | Deg: %ld   ", 
            current_pwm, count, deg_int);

        // Blocking receive with very short timeout (1ms) to keep loop responsive
        // but ensure we catch characters.
        if (HAL_UART_Receive(&huart2, &rx_byte, 1, 50) == HAL_OK) {
             if (rx_byte == 'q' || rx_byte == 'Q') {
                Motor_Stop(&myMotor);
                UART_Debug_Printf("\r\nExiting Test Mode.\r\n> ");
                break;
            }
            else if (rx_byte == 'w' || rx_byte == 'W') {
                current_pwm += 10;
                if (current_pwm > 100) current_pwm = 100;
                
                if (current_pwm >= 0) {
                    Motor_SetDirection(&myMotor, 1);
                    Motor_SetSpeed(&myMotor, (uint8_t)current_pwm);
                } else {
                    Motor_SetDirection(&myMotor, 0);
                    Motor_SetSpeed(&myMotor, (uint8_t)(-current_pwm));
                }
            }
            else if (rx_byte == 's' || rx_byte == 'S') {
                current_pwm -= 10;
                if (current_pwm < -100) current_pwm = -100;
                
                if (current_pwm >= 0) {
                    Motor_SetDirection(&myMotor, 1);
                    Motor_SetSpeed(&myMotor, (uint8_t)current_pwm);
                } else {
                    Motor_SetDirection(&myMotor, 0);
                    Motor_SetSpeed(&myMotor, (uint8_t)(-current_pwm));
                }
            }
            else if (rx_byte == ' ') {
                current_pwm = 0;
                Motor_Stop(&myMotor);
            }
        }
    }
    
    // Resume control loop
    int32_t current_pos = Motor_GetEncoderCount(&myMotor);
    servo.target_pos = current_pos;
    servo.setpoint_pos = (float)current_pos;
    servo.is_at_target = 1;
    PID_Reset(&posPID);
    servo_enabled = 1;
}