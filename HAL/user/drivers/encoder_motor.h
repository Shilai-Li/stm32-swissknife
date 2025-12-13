/**
 * @file encoder_motor.h
 * @brief Motor Encoder Driver (Motion Control)
 * @author Standard Implementation
 * @date 2024
 */

#ifndef __ENCODER_MOTOR_H
#define __ENCODER_MOTOR_H

#include "main.h"

#ifndef __STM32F1xx_HAL_TIM_H
#include "stm32f1xx_hal.h"
#endif

typedef struct {
    TIM_HandleTypeDef *htim;       // Timer Handle (Encoder Mode)
    
    // Config
    uint8_t            Inverted;   // 1 to invert direction
    uint16_t           CPR;        // Counts Per Revolution (PPR * 4)
    
    // State
    int16_t            CountPrev;  // Previous raw timer value
    int64_t            TotalCount; // Accumulative 64-bit counter (avoids overflow)
    
    // Velocity Measuring
    int64_t            LastCount;  // Count at last speed check
    uint32_t           LastTime;   // Time at last speed check
    float              SpeedRPM;   // Calculated Speed
} Encoder_Motor_HandleTypeDef;

/* Function Prototypes */

/**
 * @brief Initialize Motor Encoder
 * @param hmotor Handle
 * @param htim Timer Handle
 * @param cpr Total counts per revolution (e.g. 500 lines * 4 = 2000)
 */
void Encoder_Motor_Init(Encoder_Motor_HandleTypeDef *hmotor, TIM_HandleTypeDef *htim, uint16_t cpr);

/**
 * @brief Call this frequently (e.g. in 1ms/10ms Timer Interrupt or Main Loop)
 *        to update total count and handle 16-bit timer overflows.
 */
void Encoder_Motor_Update(Encoder_Motor_HandleTypeDef *hmotor);

/**
 * @brief Calculate Speed. Call this at fixed low-frequency intervals (e.g. 10Hz, 50Hz) 
 *        to reduce quantization noise.
 * @return Speed in RPM
 */
float Encoder_Motor_GetSpeed(Encoder_Motor_HandleTypeDef *hmotor);

/**
 * @brief Get Total Accumulated Counts
 */
int64_t Encoder_Motor_GetCount(Encoder_Motor_HandleTypeDef *hmotor);

/**
 * @brief Reset Count to 0
 */
void Encoder_Motor_Reset(Encoder_Motor_HandleTypeDef *hmotor);

#endif // __ENCODER_MOTOR_H
