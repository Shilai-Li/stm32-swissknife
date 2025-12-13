#ifndef SERVO_H
#define SERVO_H

#include "stm32f1xx_hal.h"
#include "motor_driver.h"
#include "algorithms/pid.h"

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
    TIM_HandleTypeDef *pwm_htim;
    uint32_t pwm_channel;
    uint32_t pwm_period;
    GPIO_TypeDef *en_port;
    uint16_t en_pin;
    GPIO_TypeDef *dir_port;
    uint16_t dir_pin;
    TIM_HandleTypeDef *enc_htim;
    float kp;
    float ki;
    float kd;
    float pid_limit;
    float pid_ramp;
    uint8_t auto_start;
} Servo_Config_t;

typedef struct {
    Motor_Handle_t *motor;
    PIDController *pid;
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

extern Motor_Handle_t myMotor;
extern PIDController posPID;

/*******************************************************************************
 * FUNCTION PROTOTYPES
 ******************************************************************************/

HAL_StatusTypeDef Servo_InitInstance(Servo_Handle_t *handle,
                                      Motor_Handle_t *motor,
                                      PIDController *pid,
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