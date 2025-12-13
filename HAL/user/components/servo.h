#ifndef SERVO_H
#define SERVO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <math.h>

/*******************************************************************************
 * CONSTANTS & DEFAULTS
 ******************************************************************************/
#define SERVO_MAX_VELOCITY      10000.0f  // Pulses/sec
#define SERVO_ACCELERATION      50000.0f  // Pulses/sec^2
#define SERVO_DECELERATION      50000.0f  
#define SERVO_POS_TOLERANCE     2         // Deadband (pulses)
#define SERVO_LOOP_FREQ         1000.0f   // 1kHz
#define SERVO_DT                (1.0f / SERVO_LOOP_FREQ)

/*******************************************************************************
 * ENUMS & TYPES
 ******************************************************************************/

typedef enum {
    SERVO_OK = 0,
    SERVO_ERROR = 1
} Servo_Status;

/*******************************************************************************
 * INTERFACES
 ******************************************************************************/

/* Abstract Motor Interface */
typedef struct {
    void    (*init)(void *ctx);
    void    (*start)(void *ctx);
    void    (*stop)(void *ctx);
    void    (*set_speed)(void *ctx, uint8_t speed);
    void    (*set_direction)(void *ctx, uint8_t dir);
    int32_t (*get_encoder)(void *ctx);
    void    (*reset_encoder)(void *ctx, int32_t val);
} Servo_MotorInterface_t;

/* Abstract PID Interface */
typedef struct {
    void  (*init)(void *ctx, float p, float i, float d, float limit, float ramp);
    void  (*reset)(void *ctx);
    float (*compute)(void *ctx, float error, float dt);
    void  (*set_limit)(void *ctx, float limit); 
} Servo_PIDInterface_t;

/*******************************************************************************
 * DATA STRUCTURES
 ******************************************************************************/

/* Runtime State (Read-Only for User) */
typedef struct {
    int32_t target_pos;         // Target position (pulses)
    float   setpoint_pos;       // Trajectory generator output
    float   setpoint_vel;       // Current trajectory velocity
    int32_t actual_pos;         // Current encoder reading
    bool    is_at_target;       // Position reached flag
} Servo_State_t;

/* Configuration */
typedef struct {
    float   kp;
    float   ki;
    float   kd;
    float   output_limit;
    float   ramp_rate;
    bool    auto_start;
} Servo_Config_t;

/* Abstract System/IO Interface (Decoupled from specific Drivers) */
typedef struct {
    void (*log)(const char *fmt, ...); // printf-style logger
    bool (*read_char)(uint8_t *c);     // Non-blocking char read
    void (*delay_ms)(uint32_t ms);     // Blocking delay
} Servo_SystemInterface_t;

/* Main Servo Handle */
typedef struct {
    // Dependencies (Injected)
    void *motor_ctx;
    const Servo_MotorInterface_t *motor_if;
    
    void *pid_ctx;
    const Servo_PIDInterface_t *pid_if;

    const Servo_SystemInterface_t *sys_if; // System resources
    
    // Internal Data
    Servo_Config_t config;
    volatile Servo_State_t state;
    
    // Status & Safety
    volatile bool enabled;
    volatile bool error;
    
    // Diagnostics / Internal Logic
    uint32_t debug_counter;
    float    debug_last_output;
    uint32_t overload_counter;
    int32_t  overload_start_pos;
} Servo_Handle_t;

/*******************************************************************************
 * PUBLIC API
 ******************************************************************************/

/**
 * @brief Initialize the Servo instance.
 */
Servo_Status Servo_Init(Servo_Handle_t *handle,
                             const Servo_MotorInterface_t *motor_if, void *motor_ctx,
                             const Servo_PIDInterface_t *pid_if, void *pid_ctx,
                             const Servo_SystemInterface_t *sys_if,
                             const Servo_Config_t *config);

/**
 * @brief Set a new target position (absolute pulses).
 */
void Servo_SetTarget(Servo_Handle_t *handle, int32_t position);

/**
 * @brief Check if servo has reached the target.
 */
bool Servo_IsAtTarget(Servo_Handle_t *handle);

/**
 * @brief Trigger a 1kHz Control Loop Step (Call this from your Timer ISR).
 */
void Servo_Scheduler_Tick(void);

/**
 * @brief Main Control Loop Update (Single Instance).
 */
void Servo_Update(Servo_Handle_t *handle);

/**
 * @brief Stop motor and disable control loop.
 */
void Servo_Stop(Servo_Handle_t *handle);

/**
 * @brief Start/Enable control loop.
 */
void Servo_Start(Servo_Handle_t *handle);

/* Utilities */
int32_t Servo_DegreesToPulses(float degrees);
float   Servo_PulsesToDegrees(int32_t pulses);

/* CLI / Debug Support */
void Servo_ProcessCommand(Servo_Handle_t *handle, char* cmd);
void Servo_RunEncoderTest(Servo_Handle_t *handle);
void Poll_UART_Commands(void);

#ifdef __cplusplus
}
#endif

#endif /* SERVO_H */
