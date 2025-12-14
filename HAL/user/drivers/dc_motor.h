/**
 * @file dc_motor.h
 * @brief DC Motor Driver (H-Bridge / PWM)
 * 
 * =================================================================================
 *                       >>> INTEGRATION GUIDE <<<
 * =================================================================================
 * 1. CubeMX Config (Timers -> PWM Generation):
 *    - Channel x: PWM Generation CHx
 *    - Prescaler/Period: Set for 10kHz - 20kHz PWM freq.
 *      * Example 72MHz Class: PSC=0, ARR=3599 -> 20kHz.
 * 
 * 2. GPIO:
 *    - DIR / EN pins: Output Push-Pull.
 * 
 * 3. Usage:
 *    Motor_Init(&hmotor, &htim1, TIM_CHANNEL_1, GPIOA, PIN_1, GPIOA, PIN_2);
 * =================================================================================
 */
#ifndef MOTOR_DRIVER_H
#define MOTOR_DRIVER_H

#include "stm32f1xx_hal.h" // Include the corresponding HAL library for your chip model

// Define motor structure for managing pins and timers
typedef struct {
    TIM_HandleTypeDef *htim;    // Timer handle for PWM
    uint32_t channel;           // PWM channel
    GPIO_TypeDef *en_port;      // Enable pin port
    uint16_t en_pin;            // Enable pin number
    GPIO_TypeDef *dir_port;     // Direction pin port
    uint16_t dir_pin;           // Direction pin number
    uint32_t pwm_period;        // PWM period, configured by timer
    // Encoder interface (using hardware timer encoder mode)
    TIM_HandleTypeDef *htim_enc; // Encoder timer handle (e.g., TIM2)
    int32_t total_count;         // Total count value (after overflow handling)
    uint16_t last_counter;       // Last read timer value
} Motor_Handle_t;

// --- Function declarations ---

// Initialize motor
void Motor_Init(Motor_Handle_t *motor);

// Initialize encoder (configure interrupt)
void Motor_Encoder_Init(Motor_Handle_t *motor);

// Get encoder count value
int32_t Motor_GetEncoderCount(Motor_Handle_t *motor);

// Reset encoder count value
void Motor_ResetEncoderCount(Motor_Handle_t *motor, int32_t value);

// Start motor
void Motor_Start(Motor_Handle_t *motor);

// Stop motor
void Motor_Stop(Motor_Handle_t *motor);

// Set speed (0 - 100%)
void Motor_SetSpeed(Motor_Handle_t *motor, uint8_t duty_percent);

// Set direction (1: forward, 0: reverse)
void Motor_SetDirection(Motor_Handle_t *motor, uint8_t direction);

#endif

