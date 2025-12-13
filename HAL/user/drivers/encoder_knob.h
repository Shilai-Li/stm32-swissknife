/**
 * @file encoder_knob.h
 * @brief EC11 Rotary Encoder Knob Driver (HMI)
 * @author Standard Implementation
 * @date 2024
 */

#ifndef __ENCODER_KNOB_H
#define __ENCODER_KNOB_H

#include "main.h"

#ifndef __STM32F1xx_HAL_TIM_H
#include "stm32f1xx_hal.h"
#endif

typedef struct {
    TIM_HandleTypeDef *htim;       // Timer Handle (Encoder Mode)
    
    // Config
    uint8_t            Inverted;   // 1 to invert direction
    uint8_t            UseVelocity; // 1 to enable variable step size based on speed
    
    // State
    int16_t            CountRaw;   // Raw hardware counter value
    int16_t            CountPrev;  // Previous raw value
    int32_t            Position;   // Logic Position (User view)
    
    // Velocity/Acceleration
    uint32_t           LastTick;
    uint32_t           Velocity;   // Steps per second approx
} Encoder_Knob_HandleTypeDef;

/* Function Prototypes */

/**
 * @brief Initialize the Knob Encoder
 * @param hknob Handle
 * @param htim Timer handle configured in Encoder Mode (TI1+TI2)
 */
void Encoder_Knob_Init(Encoder_Knob_HandleTypeDef *hknob, TIM_HandleTypeDef *htim);

/**
 * @brief Update the encoder state. Call this periodically (e.g., 1ms-10ms)
 *        or in the main loop.
 * @return The incremental change since last update (Delta)
 */
int16_t Encoder_Knob_Update(Encoder_Knob_HandleTypeDef *hknob);

/**
 * @brief Get absolute logical position
 */
int32_t Encoder_Knob_GetPosition(Encoder_Knob_HandleTypeDef *hknob);

/**
 * @brief Reset position to 0
 */
void Encoder_Knob_Reset(Encoder_Knob_HandleTypeDef *hknob);

#endif // __ENCODER_KNOB_H
