/**
 * @file ir_nec_exti.c
 * @brief NEC IR Decoder Implementation (EXTI)
 * @details See .h file for Integration Guide
 */

#include "ir_nec_exti.h"
#include "delay.h" // Need micros()

// NEC Protocol Constants (us)
#define NEC_HDR_MARK    9000
#define NEC_HDR_SPACE   4500
#define NEC_RPT_SPACE   2250
#define NEC_BIT_MARK    560
#define NEC_ONE_SPACE   1690
#define NEC_ZERO_SPACE  560

// Tolerance
#define TOLERANCE       200 // +/- us

#define CHECK_RANGE(val, target) ((val >= (target - TOLERANCE)) && (val <= (target + TOLERANCE)))

void IR_NEC_EXTI_Init(IR_NEC_EXTI_Handle_t *h, uint16_t pin) {
    if(!h) return;
    h->gpio_pin = pin;
    h->state = IR_STATE_IDLE;
    h->last_frame.received = false;
    h->last_tick_us = 0;
}

void IR_NEC_EXTI_Callback(IR_NEC_EXTI_Handle_t *h, uint16_t GPIO_Pin) {
    if (GPIO_Pin != h->gpio_pin) return;
    
    // We measure time between Falling Edges
    uint32_t now = micros();
    uint32_t delta = now - h->last_tick_us;
    h->last_tick_us = now;
    
    // State Machine
    switch (h->state) {
        case IR_STATE_IDLE:
            // Waiting for a long gap or initial fall?
            // Actually, NEC starts with 9ms Low + 4.5ms High.
            // The falling edge comes at the start of 9ms Low, and next at start of first bit?
            // Wait, EXTI usually Configured on Falling Edge.
            
            // Pulse logic:
            // 1. Fall (Start of 9ms Mark) -> Timer Start
            // 2. Fall (End of 4.5ms Space) -> Delta = 13.5ms
            
            if (delta > 13000 && delta < 14000) {
                 // 9000 + 4500 = 13500 us. This is Start Frame.
                 h->state = IR_STATE_DATA;
                 h->bit_index = 0;
                 h->raw_data = 0;
            } else if (delta > 11000 && delta < 12000) {
                 // 9000 + 2250 = 11250 us. This is Repeat Frame.
                 h->last_frame.is_repeat = true;
                 h->last_frame.received = true;
                 h->state = IR_STATE_IDLE;
            } else {
                 // Noise or reset
            }
            break;
            
        case IR_STATE_DATA:
            // Data Bits: Mark (560) + Space (560 or 1690)
            // Delta is time from one Falling Edge to next.
            // Logic 0: 560 + 560 = 1120 us
            // Logic 1: 560 + 1690 = 2250 us
            
            if (CHECK_RANGE(delta, 1120)) {
                // Bit 0
                // raw_data |= (0 << index) -> No op
                h->bit_index++;
            } else if (CHECK_RANGE(delta, 2250)) {
                // Bit 1
                h->raw_data |= (1UL << h->bit_index);
                h->bit_index++;
            } else {
                // Error / Noise
                h->state = IR_STATE_IDLE;
            }
            
            if (h->bit_index >= 32) {
                // Done
                h->last_frame.address = (h->raw_data & 0x0000FFFF);         // Usually Addr + InvAddr
                h->last_frame.command = (h->raw_data & 0xFFFF0000) >> 16;  // Usually Cmd + InvCmd
                // Usually NEC sends LSB first. But raw_data constructed LSB first above.
                
                h->last_frame.is_repeat = false;
                h->last_frame.received = true;
                h->state = IR_STATE_IDLE;
            }
            break;
            
        default:
            h->state = IR_STATE_IDLE;
            break;
    }
}

bool IR_NEC_EXTI_Available(IR_NEC_EXTI_Handle_t *h) {
    if (h->last_frame.received) {
        h->last_frame.received = false;
        return true;
    }
    return false;
}

uint16_t IR_NEC_EXTI_GetCommand(IR_NEC_EXTI_Handle_t *h) {
    // Standard NEC: Command is 2nd byte. 4th byte is inverse of 2nd.
    // Our raw_data: [Addr][~Addr][Cmd][~Cmd] or [Addr][Addr][Cmd][~Cmd]?
    // It depends on remote.
    // We just return upper 16 for now or parse specifically.
    // Return low 8 bits of command word?
    return (h->last_frame.command >> 8); 
}
