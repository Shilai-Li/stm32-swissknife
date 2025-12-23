/**
 * @file ir_nec_tim.c
 * @brief NEC IR Decoder Implementation (Timer IC)
 * @details See .h file for Integration Guide
 */

#include "ir_nec_tim.h"

// States
#define NEC_STATE_IDLE  0
#define NEC_STATE_START 1
#define NEC_STATE_DATA  2

// Tolerances same as EXTI
#define TOLERANCE       200
#define CHECK_RANGE(val, target) ((val >= (target - TOLERANCE)) && (val <= (target + TOLERANCE)))

void IR_NEC_TIM_Init(IR_NEC_TIM_Handle_t *h, TIM_HandleTypeDef *htim, uint32_t channel) {
    h->htim = htim;
    h->channel = channel;
    h->state = NEC_STATE_IDLE;
    h->data_ready = false;
    
    // Start IC
    HAL_TIM_IC_Start_IT(htim, channel);
}

void IR_NEC_TIM_Callback(IR_NEC_TIM_Handle_t *h, TIM_HandleTypeDef *htim) {
    if (htim != h->htim) return;
    
    // Read Capture Value
    // Ideally we reset counter to 0 each time to get delta easily, 
    // or calculate diff. Resetting is cleaner for protocol decoding.
    
    uint32_t val = HAL_TIM_ReadCapturedValue(htim, h->channel);
    __HAL_TIM_SET_COUNTER(htim, 0); // Reset for next measurement
    
    switch (h->state) {
        case NEC_STATE_IDLE:
            // First Fall -> triggered (val is time since last unknown event, ignore)
            // But we actually trigger on FALLING.
            // Wait, standard NEC:
            // 9ms Low -> Rise -> 4.5ms High -> Fall.
            // If we capture Falling Edge to Falling Edge:
            // Start = 13.5ms.
            
            // If first time, Counter might be huge.
            if (val > 13000 && val < 14000) {
                h->state = NEC_STATE_DATA;
                h->bit_count = 0;
                h->raw_data = 0;
            } else if (val > 11000 && val < 12000) {
                 // Repeat
                 // Handle repeat...
                 h->state = NEC_STATE_IDLE;
            }
            break;
            
        case NEC_STATE_DATA:
            if (CHECK_RANGE(val, 1125)) {
                // 0
                h->bit_count++;
            } else if (CHECK_RANGE(val, 2250)) {
                // 1
                h->raw_data |= (1UL << h->bit_count);
                h->bit_count++;
            } else {
                h->state = NEC_STATE_IDLE;
            }
            
            if (h->bit_count >= 32) {
                h->decoded_cmd = (h->raw_data >> 16) & 0xFF; // Simplified
                h->data_ready = true;
                h->state = NEC_STATE_IDLE;
            }
            break;
    }
}

bool IR_NEC_TIM_Available(IR_NEC_TIM_Handle_t *h) {
    if (h->data_ready) {
        h->data_ready = false;
        return true;
    }
    return false;
}

uint16_t IR_NEC_TIM_GetCommand(IR_NEC_TIM_Handle_t *h) {
    return h->decoded_cmd;
}
