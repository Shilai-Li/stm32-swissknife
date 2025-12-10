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

    // 3. Initialize Drivers
    Motor_Init(&myMotor);
    Motor_Encoder_Init(&myMotor);
    Motor_ResetEncoderCount(&myMotor, 0);
    Motor_Start(&myMotor);

    // 4. Initialize PID
    PID_Init(&posPID, servo_kp, servo_ki, servo_kd, servo_pid_limit, 1000.0f);
    
    // 5. Start Control Loop Interrupt
    HAL_TIM_Base_Start_IT(&htim3);
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
        // Optional: Disable integral windup or relax motor if needed?
        // For Servo, we usually want active holding, so we keep PID active.
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
 * MAIN ENTRY POINT (Test Loop)
 ******************************************************************************/
void User_Entry(void)
{
    UART_Debug_Printf("=== Servo Mode Initializing ===\r\n");

    // Initialize Servo System
    // (GPIO Init is assumed done by main.c or we need to ensure local init safely)
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    // Re-verify these pins! 
    // DIR: PB13, EN: PC15
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_15;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    Servo_Init();

    UART_Debug_Printf("=== Servo Mode Ready ===\r\n");

    uint32_t last_print = 0;
    
    // Test Sequence Logic
    uint8_t step = 0;
    uint32_t wait_start = 0;

    while (1) {
        // Simple Debug Telemetry (10Hz)
        if (HAL_GetTick() - last_print > 100) {
            last_print = HAL_GetTick();
            UART_Debug_Printf("Tgt:%ld Act:%ld Set:%.1f Vel:%.1f PWM:%.1f\r\n", 
                servo.target_pos, servo.actual_pos, servo.setpoint_pos, servo.setpoint_vel, debug_last_pwm);
        }

        // Servo Test Sequence: 0 -> 1000 -> 0 -> -1000 -> ...
        switch (step) {
            case 0:
                UART_Debug_Printf(">>> Command: Go to 2000\r\n");
                Servo_SetTarget(2000);
                step++;
                break;
            case 1:
                if (Servo_IsAtTarget()) {
                    UART_Debug_Printf(">>> Reached 2000. Holding...\r\n");
                    wait_start = HAL_GetTick();
                    step++;
                }
                break;
            case 2:
                if (HAL_GetTick() - wait_start > 2000) { // Hold for 2s
                    UART_Debug_Printf(">>> Command: Go to 0\r\n");
                    Servo_SetTarget(0);
                    step++;
                }
                break;
            case 3:
                if (Servo_IsAtTarget()) {
                     UART_Debug_Printf(">>> Reached 0. Holding...\r\n");
                    wait_start = HAL_GetTick();
                    step++;
                }
                break;
            case 4:
                if (HAL_GetTick() - wait_start > 2000) {
                     UART_Debug_Printf(">>> Command: Go to -2000\r\n");
                    Servo_SetTarget(-2000);
                    step++;
                }
                break;
            case 5:
                if (Servo_IsAtTarget()) {
                    wait_start = HAL_GetTick();
                    step++;
                }
                break;
            case 6:
                if (HAL_GetTick() - wait_start > 2000) {
                    step = 0; // Loop back
                }
                break;
        }
    }
}