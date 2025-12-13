#ifndef SERVO_H
#define SERVO_H

#include "stm32f1xx_hal.h"

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
#define SERVO_KP_DEFAULT        2.5f
#define SERVO_KI_DEFAULT        0.05f
#define SERVO_KD_DEFAULT        0.10f
#define SERVO_PID_LIMIT_DEFAULT 100.0f    // Max PWM Duty %

/*******************************************************************************
 * ENCODER CONFIGURATION
 ******************************************************************************/
#define ENCODER_PULSES_PER_REV  360
#define DEGREES_TO_PULSES(deg)  (deg)  // 1:1 mapping for 360 pulse encoder
#define PULSES_TO_DEGREES(pulses) ((float)(pulses))  // Direct 1:1 conversion

/*******************************************************************************
 * INTERFACE DEFINITIONS
 ******************************************************************************/

/* Motor Interface */
typedef void (*Servo_Motor_Init_Fn)(void *ctx);
typedef void (*Servo_Motor_Start_Fn)(void *ctx);
typedef void (*Servo_Motor_Stop_Fn)(void *ctx);
typedef void (*Servo_Motor_SetSpeed_Fn)(void *ctx, uint8_t speed);
typedef void (*Servo_Motor_SetDirection_Fn)(void *ctx, uint8_t dir);
typedef int32_t (*Servo_Motor_GetEncoder_Fn)(void *ctx);
typedef void (*Servo_Motor_ResetEncoder_Fn)(void *ctx, int32_t val);

typedef struct {
    Servo_Motor_Init_Fn init;
    Servo_Motor_Start_Fn start;
    Servo_Motor_Stop_Fn stop;
    Servo_Motor_SetSpeed_Fn set_speed;
    Servo_Motor_SetDirection_Fn set_direction;
    Servo_Motor_GetEncoder_Fn get_encoder;
    Servo_Motor_ResetEncoder_Fn reset_encoder;
} Servo_MotorInterface_t;

/* PID Interface */
typedef void (*Servo_PID_Init_Fn)(void *ctx, float p, float i, float d, float limit, float ramp);
typedef void (*Servo_PID_Reset_Fn)(void *ctx);
typedef float (*Servo_PID_Compute_Fn)(void *ctx, float error, float dt);
typedef void (*Servo_PID_SetLimit_Fn)(void *ctx, float limit); // Optional helper if needed

typedef struct {
    Servo_PID_Init_Fn init;
    Servo_PID_Reset_Fn reset;
    Servo_PID_Compute_Fn compute;
    Servo_PID_SetLimit_Fn set_limit; // Added to support 'L' command
} Servo_PIDInterface_t;

/*******************************************************************************
 * SERVO STATE STRUCTURE
 ******************************************************************************/
typedef struct {
    int32_t target_pos;         // Final desired position
    float   setpoint_pos;       // Current internal setpoint (Trajectory generator output)
    float   setpoint_vel;       // Current internal velocity
    int32_t actual_pos;         // Actual encoder position
    uint8_t is_at_target;       // Flag 1=Yes, 0=No
} Servo_State_t;

typedef struct {
    // Hardware config is now largely handled by the context/adapter, 
    // but these values might still be used for PID Init or logic.
    float kp;
    float ki;
    float kd;
    float pid_limit;
    float pid_ramp;
    uint8_t auto_start;
} Servo_Config_t;

typedef struct {
    void *motor_context;
    const Servo_MotorInterface_t *motor_if;
    
    void *pid_context;
    const Servo_PIDInterface_t *pid_if;
    
    volatile Servo_State_t *state;
    volatile uint32_t *debug_counter;
    volatile float *debug_last_pwm;
    volatile uint8_t *enabled;
    volatile uint8_t *error_state;
    volatile uint32_t debug_counter_storage;
    volatile float debug_last_pwm_storage;
    volatile uint8_t enabled_storage;
    volatile uint8_t error_state_storage;
    uint32_t high_load_duration;
    int32_t load_start_pos;
    float kp;
    float ki;
    float kd;
    float pid_limit;
} Servo_Handle_t;

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/
extern volatile Servo_State_t servo;
extern volatile float servo_kp;
extern volatile float servo_ki;
extern volatile float servo_kd;
extern volatile float servo_pid_limit;
extern volatile uint8_t servo_enabled;
extern volatile uint8_t servo_error_state;
extern volatile uint32_t debug_counter;
extern volatile float debug_last_pwm;

// Removed direct externs to Motor/PID objects
// extern Motor_Handle_t myMotor;
// extern PIDController posPID;

/*******************************************************************************
 * FUNCTION PROTOTYPES
 ******************************************************************************/

HAL_StatusTypeDef Servo_InitInstance(Servo_Handle_t *handle,
                                      void *motor_ctx,
                                      const Servo_MotorInterface_t *motor_if,
                                      void *pid_ctx,
                                      const Servo_PIDInterface_t *pid_if,
                                      volatile Servo_State_t *state,
                                      const Servo_Config_t *cfg);
void Servo_SetTargetInstance(Servo_Handle_t *handle, int32_t position);
uint8_t Servo_IsAtTargetInstance(Servo_Handle_t *handle);
void Servo_UpdateInstance_1kHz(Servo_Handle_t *handle);

/* Command Processing */
void Process_Command(char* cmd);
void Poll_UART_Commands(void);

void Test_Encoder_Readings(void);

#endif /* SERVO_H */