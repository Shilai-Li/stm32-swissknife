#ifndef SERVO_H
#define SERVO_H

#include "stm32f1xx_hal.h"
#include "../Drivers/motor_driver.h"
#include "../Middlewares/Algorithms/pid.h"

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

/* Core Servo Functions */
void Servo_Init(void);
void Servo_SetTarget(int32_t position);
uint8_t Servo_IsAtTarget(void);
void Servo_Update_1kHz(void);

/* Trajectory Generation */
float Servo_ComputeTrajectory(void);

/* Safety Functions */
void Check_Runaway_Condition(float pid_output, int32_t current_pos);

/* Utility Functions */
static inline int32_t Degrees_To_Pulses(int32_t degrees);
static inline float Pulses_To_Degrees(int32_t pulses);

/* Command Processing */
void Process_Command(char* cmd);
void Poll_UART_Commands(void);

void Test_Encoder_Readings(void);

#endif /* SERVO_H */