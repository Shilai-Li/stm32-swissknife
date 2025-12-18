#include "../servo_port.h"
#include "tim.h"
#include "uart.h"
#include "delay.h"
#include <stdarg.h>
#include <stdio.h>

/* Global Hardware Instances */
Motor_Handle_t myMotor;
PIDController posPID;

extern TIM_HandleTypeDef htim3;

static void Adapter_Log(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    // Since UART_Debug_Printf handles fmt internally or uses a buffer, 
    // BUT UART_Debug_Printf assumes usage like printf. 
    // We need to implement a wrapper. 
    // Assuming UART_Debug_Printf has internal buffering/handling.
    // If UART_Debug_Printf is NOT vararg, this will fail.
    // Let's assume UART_Debug_Printf IS vararg (based on previous usages).
    // Note: C standard doesn't allow forwarding va_list to a ... function easily.
    // We should use vsnprintf into a buffer.
    char buf[128];
    vsnprintf(buf, sizeof(buf), fmt, args);
    UART_Debug_Printf("%s", buf);
    va_end(args);
}

static bool Adapter_ReadChar(uint8_t *c) {
    return UART_Read(UART_DEBUG_CHANNEL, c);
}

static void Adapter_DelayMs(uint32_t ms) {
    HAL_Delay(ms);
}

const Servo_SystemInterface_t servo_system_interface = {
    .log = Adapter_Log,
    .read_char = Adapter_ReadChar,
    .delay_ms = Adapter_DelayMs
};

void ServoPort_Init_Hardware_Config(void)
{
    // Default Hardware Configuration for this project (Using STM32 HAL)
    myMotor.htim = &htim1;
    myMotor.channel = TIM_CHANNEL_1;
    myMotor.pwm_period = 99; // 100 counts -> 20kHz if APB2 is 72Mhz with Prescaler
    
    // Enable Pins
    myMotor.en_port = GPIOC;
    myMotor.en_pin = GPIO_PIN_15;
    
    // Direction Pins
    myMotor.dir_port = GPIOB;
    myMotor.dir_pin = GPIO_PIN_13;
    
    // Encoder Timer
    myMotor.htim_enc = &htim2;
}

// --- Adapter Wrappers for Motor ---
static void Adapter_Motor_Init(void *ctx) { 
    Motor_Init((Motor_Handle_t*)ctx); 
    Motor_Encoder_Init((Motor_Handle_t*)ctx);
}
static void Adapter_Motor_Start(void *ctx) { Motor_Start((Motor_Handle_t*)ctx); }
static void Adapter_Motor_Stop(void *ctx) { Motor_Stop((Motor_Handle_t*)ctx); }
static void Adapter_Motor_SetSpeed(void *ctx, uint8_t speed) { Motor_SetSpeed((Motor_Handle_t*)ctx, speed); }
static void Adapter_Motor_SetDirection(void *ctx, uint8_t dir) { Motor_SetDirection((Motor_Handle_t*)ctx, dir); }
static int32_t Adapter_Motor_GetEncoder(void *ctx) { return Motor_GetEncoderCount((Motor_Handle_t*)ctx); }
static void Adapter_Motor_ResetEncoder(void *ctx, int32_t val) { Motor_ResetEncoderCount((Motor_Handle_t*)ctx, val); }

const Servo_MotorInterface_t servo_motor_driver_interface = {
    .init = Adapter_Motor_Init,
    .start = Adapter_Motor_Start,
    .stop = Adapter_Motor_Stop,
    .set_speed = Adapter_Motor_SetSpeed,
    .set_direction = Adapter_Motor_SetDirection,
    .get_encoder = Adapter_Motor_GetEncoder,
    .reset_encoder = Adapter_Motor_ResetEncoder
};

// --- Adapter Wrappers for PID ---
static void Adapter_PID_Init(void *ctx, float p, float i, float d, float limit, float ramp) {
    PID_Init((PIDController*)ctx, p, i, d, limit, ramp);
}
static void Adapter_PID_Reset(void *ctx) { PID_Reset((PIDController*)ctx); }
static float Adapter_PID_Compute(void *ctx, float error, float dt) { return PID_Compute((PIDController*)ctx, error, dt); }
static void Adapter_PID_SetLimit(void *ctx, float limit) { ((PIDController*)ctx)->limit = limit; }

const Servo_PIDInterface_t servo_pid_driver_interface = {
    .init = Adapter_PID_Init,
    .reset = Adapter_PID_Reset,
    .compute = Adapter_PID_Compute,
    .set_limit = Adapter_PID_SetLimit
};

/**
 * @brief  One-stop initialization for the default servo setup
 */
Servo_Status ServoPort_Init(Servo_Handle_t *handle)
{
    if (!handle) return SERVO_ERROR;

    // 1. Configure Hardware Structs
    ServoPort_Init_Hardware_Config();

    // 2. Define Servo Control Config
    Servo_Config_t config = {
        .kp = 2.5f,
        .ki = 0.05f,
        .kd = 0.05f, // Reduced D-term
        .output_limit = 100.0f,
        .ramp_rate = 1000.0f, // Reduced ramp
        .auto_start = false
    };

    // 3. Inject Dependencies and Init (Using System Interface)
    Servo_Status status = Servo_Init(handle,
                      &servo_motor_driver_interface, &myMotor,
                      &servo_pid_driver_interface, &posPID,
                      &servo_system_interface,
                      &config);

    if (status == SERVO_OK) {
        // Start the scheduler timer (1kHz)
        // Check if already started to avoid re-triggering or just call it safely
        HAL_TIM_Base_Start_IT(&htim3);
    }

    return status;
}

// Global System Interrupt Handler for Timers
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    Delay_TIM_PeriodElapsedCallback(htim); // Chain existing
    
    if (htim->Instance == TIM3) {
        // Trigger the Servo component's scheduler
        Servo_Scheduler_Tick();
    }
}
