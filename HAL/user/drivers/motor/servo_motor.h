/**
 * @file servo_motor.h
 * @brief RC Servo Motor Driver Header File
 * 
 * =================================================================================
 *                       >>> INTEGRATION GUIDE <<<
 * =================================================================================
 * 1. CubeMX Config (Timers -> PWM Generation):
 *    - Channel x: PWM Generation CHx
 *    - Prescaler (PSC): Set to obtain 1MHz Counter Clock (1us tick).
 *       * Example: 72MHz -> PSC = 71.
 *    - Period (ARR): 20000 - 1 (for 20ms / 50Hz).
 * 
 * 2. Wiring:
 *    - Servo Signal -> TIMx_CHx Pin.
 *    - Servo VCC -> 5V (Not 3.3V!).
 * 
 * 3. Usage:
 *    Servo_Motor_Init(&myservo, &htim2, TIM_CHANNEL_1);
 * =================================================================================
 */

#ifndef __SERVO_MOTOR_H
#define __SERVO_MOTOR_H

#include "main.h"

#ifndef __STM32F1xx_HAL_TIM_H
#include "stm32f1xx_hal.h"
#endif

// Standard RC Servo Constants
#define SERVO_MIN_PULSE_US    500   // 0 degrees (Standard)
#define SERVO_MAX_PULSE_US    2500  // 180 degrees (Standard 2500, some are 2400)
#define SERVO_FREQ_HZ         50    // 20ms period

typedef struct {
    TIM_HandleTypeDef *htim;       // Timer Handle
    uint32_t           Channel;    // Timer Channel (TIM_CHANNEL_x)
    
    // Config
    uint16_t           MinPulse;   // Min Pulse width in us (e.g. 500)
    uint16_t           MaxPulse;   // Max Pulse width in us (e.g. 2500)
    float              MaxAngle;   // Logical Max Angle (e.g. 180.0 or 270.0)
    
    // State
    float              CurrentAngle;
} Servo_Motor_HandleTypeDef;

/* Function Prototypes */

/**
 * @brief Initialize Servo Motor
 * @param hservo Handle
 * @param htim Timer Handle (Must be configured for PWM generation)
 *             Note: Timer Prescaler should ideally be set such that 1 tick = 1us.
 *             OR 1 tick = 0.5us etc. Driver assumes 1us per tick logic for simple calculation if possible,
 *             BUT actually for HAL convenience, we usually relay on Comparison Register.
 *             Driver will assume ARR is set for 20ms (50Hz). 
 *             User must ensure Period(ARR) corresponds to 20ms.
 * @param channel Timer Channel (TIM_CHANNEL_1/2/3/4)
 */
void Servo_Motor_Init(Servo_Motor_HandleTypeDef *hservo, TIM_HandleTypeDef *htim, uint32_t channel);

/**
 * @brief Set Custom Pulse Width Limits
 */
void Servo_Motor_SetLimits(Servo_Motor_HandleTypeDef *hservo, uint16_t min_us, uint16_t max_us, float max_angle);

/**
 * @brief Set Servo Angle
 * @param angle Angle in degrees (0.0 to MaxAngle)
 */
void Servo_Motor_SetAngle(Servo_Motor_HandleTypeDef *hservo, float angle);

/**
 * @brief Set Raw Pulse Width
 * @param pulse_us Pulse width in microseconds
 */
void Servo_Motor_WriteMicroseconds(Servo_Motor_HandleTypeDef *hservo, uint16_t pulse_us);

#endif // __SERVO_MOTOR_H
