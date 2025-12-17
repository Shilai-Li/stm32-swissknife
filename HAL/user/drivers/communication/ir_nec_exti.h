/**
 * @file ir_nec_exti.h
 * @brief NEC IR Decoder using GPIO EXTI (Easy Mode)
 *
 * =================================================================================
 *                       >>> INTEGRATION GUIDE <<<
 * =================================================================================
 * 1. CubeMX Config:
 *    - Select a GPIO Pin (e.g., PB0).
 *    - Mode: External Interrupt Mode with Falling edge trigger detection.
 *    - NVIC: Enable "EXTI line0 interrupt" (or corresponding line).
 * 
 * 2. In your code (main.c or stm32xxxx_it.c):
 *    
 *    // Include Header
 *    #include "drivers/ir_nec_exti.h"
 *    extern IR_NEC_EXTI_Handle_t my_ir; // Or however you declare it
 * 
 *    // Implement/Modify Callback
 *    void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
 *        // Call the Driver Logic
 *        IR_NEC_EXTI_Callback(&my_ir, GPIO_Pin);
 *    }
 * 
 * 3. Init:
 *    IR_NEC_EXTI_Init(&my_ir, GPIO_PIN_0);
 * =================================================================================
 */

#ifndef IR_NEC_EXTI_H
#define IR_NEC_EXTI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"
#include <stdbool.h>

/**
 * @brief NEC IR Event Structure
 */
typedef struct {
    uint16_t address;
    uint16_t command;
    bool     is_repeat;
    bool     received; // Flag: new data ready
} NEC_Frame_t;

/**
 * @brief Handle
 */
typedef struct {
    uint16_t     gpio_pin;
    NEC_Frame_t  last_frame;
    
    // Internal State Machine
    uint32_t     last_tick_us;
    uint8_t      bit_index;
    uint32_t     raw_data;
    enum {
        IR_STATE_IDLE,
        IR_STATE_START,
        IR_STATE_DATA
    } state;
} IR_NEC_EXTI_Handle_t;

/**
 * @brief Init
 * @note  User MUST configure the GPIO Pin as "External Interrupt Mode with Falling Edge" in CubeMX.
 *        And Enable NVIC for EXTI line.
 */
void IR_NEC_EXTI_Init(IR_NEC_EXTI_Handle_t *handle, uint16_t pin);

/**
 * @brief ISR Handler
 * @note  Call this function inside 'HAL_GPIO_EXTI_Callback'
 */
void IR_NEC_EXTI_Callback(IR_NEC_EXTI_Handle_t *handle, uint16_t GPIO_Pin);

/**
 * @brief Check availability
 * @return true if new code received. Clears flag after reading.
 */
bool IR_NEC_EXTI_Available(IR_NEC_EXTI_Handle_t *handle);

/**
 * @brief Get Last Code
 */
uint16_t IR_NEC_EXTI_GetCommand(IR_NEC_EXTI_Handle_t *handle);

#ifdef __cplusplus
}
#endif

#endif
