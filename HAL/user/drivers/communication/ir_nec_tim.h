/**
 * @file ir_nec_tim.h
 * @brief NEC IR Decoder using Timer Input Capture (High Precision)
 * 
 * =================================================================================
 *                       >>> INTEGRATION GUIDE <<<
 * =================================================================================
 * 1. CubeMX Config (e.g., TIM2):
 *    - Clock Source: Internal Clock
 *    - Channel 1 (or any): Input Capture direct mode
 *    - Parameter Settings:
 *       - Prescaler: 71 (for 72MHz) -> Counter Clock = 1MHz (1us tick) <--- CRITICAL
 *       - Polarity: Falling Edge
 *       - Selection: Direct
 *       - Filter: 0-15 (optional for de-bouncing)
 *    - NVIC: Enable "TIM2 global interrupt"
 * 
 * 2. In your code (main.c or stm32xxxx_it.c):
 *    
 *    // Include Header
 *    #include "drivers/ir_nec_tim.h"
 *    extern IR_NEC_TIM_Handle_t my_ir_tim;
 * 
 *    // Implement/Modify Callback
 *    void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
 *        // Call the Driver Logic
 *        IR_NEC_TIM_Callback(&my_ir_tim, htim);
 *    }
 * 
 * 3. Init:
 *    IR_NEC_TIM_Init(&my_ir_tim, &htim2, TIM_CHANNEL_1);
 * =================================================================================
 */

#ifndef IR_NEC_TIM_H
#define IR_NEC_TIM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"
#include <stdbool.h>

typedef struct {
    TIM_HandleTypeDef *htim;
    uint32_t           channel;
    
    // State
    volatile uint8_t   state;
    volatile uint32_t  raw_data;
    volatile uint8_t   bit_count;
    
    // Result
    volatile bool      data_ready;
    uint16_t           decoded_cmd;
} IR_NEC_TIM_Handle_t;

/**
 * @brief Initialize
 * @param htim Handle to Timer configured in Input Capture Direct Mode, 
 *             Falling Edge, Prescaler set to 1us (1MHz).
 */
void IR_NEC_TIM_Init(IR_NEC_TIM_Handle_t *h, TIM_HandleTypeDef *htim, uint32_t channel);

/**
 * @brief IC Callback
 * @note  Call inside HAL_TIM_IC_CaptureCallback
 */
void IR_NEC_TIM_Callback(IR_NEC_TIM_Handle_t *h, TIM_HandleTypeDef *htim);

bool IR_NEC_TIM_Available(IR_NEC_TIM_Handle_t *h);
uint16_t IR_NEC_TIM_GetCommand(IR_NEC_TIM_Handle_t *h);

#ifdef __cplusplus
}
#endif

#endif
