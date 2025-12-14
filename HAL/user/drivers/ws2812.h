/**
 * @file ws2812.h
 * @brief WS2812B / SK6812 LED Driver (PWM + DMA)
 * @author Standard Implementation
 * @date 2024
 * 
 * =================================================================================
 *                       >>> INTEGRATION GUIDE <<<
 * =================================================================================
 * 1. CubeMX Timer Config (e.g., TIM1, TIM2...):
 *    - Channel X: PWM Generation CHx
 *    - Prescaler (PSC): 0 (assuming 72MHz or similar fast clock)
 *    - Counter Period (ARR): 90-1 (for 72MHz -> 800kHz typical WS2812 freq)
 *      * Calculation: TimerFreq / 800000 - 1. E.g., 72MHz/800kHz = 90. ARR=89.
 * 
 * 2. DMA Settings (Under Timer -> DMA Settings):
 *    - Add Request for TIMx_CHx / TIMx_UP
 *    - Direction: Memory To Peripheral
 *    - Priority: High/Very High
 *    - Mode: Normal (Direct Mode, disable Circular!) <--- CRITICAL
 *    - Data Width: Half Word (16 bit) for both
 * 
 * 3. User Code (stm32xxxx_it.c / main.c):
 *    - Implement 'HAL_TIM_PWM_PulseFinishedCallback'
 *    - Call 'WS2812_DmaCallback(&my_ws2812_handle)' inside it.
 * =================================================================================
 */

#ifndef __WS2812_H
#define __WS2812_H

#include "main.h"

#ifndef __STM32F1xx_HAL_TIM_H
#include "stm32f1xx_hal.h"
#endif

// --- Configuration ---
// Limit max LEDs to control RAM usage.
// Each LED requires: 3 bytes (RGB Buffer) + 24 * 2 bytes (DMA Buffer) = 51 bytes.
// 100 LEDs ~= 5KB RAM. STM32F103C8T6 has 20KB.
#define WS2812_MAX_LEDS     64

// Timing Constants for 72MHz Clock (ARR=90 -> 800kHz)
// User must check TIM clock. Code assumes ARR set correctly for 1.25us period.
// T0H: 0.4us -> ~32% duty
// T1H: 0.8us -> ~64% duty
// These defaults will be calculated dynamically based on htim->Init.Period if logical,
// or we use generic ratios.
#define WS2812_DUTY_0(period)  ((period * 32) / 100)
#define WS2812_DUTY_1(period)  ((period * 64) / 100)

typedef struct {
    TIM_HandleTypeDef *htim;       // Timer Handle
    uint32_t           Channel;    // TIM Channel
    
    uint16_t           NumLEDs;
    uint8_t            RGB_Buffer[WS2812_MAX_LEDS][3]; // G-R-B format stored as R-G-B logical? 
                                                       // Protocol sends G,R,B. logic stores standard RGB?
                                                       
    // DMA Buffer: +1 for Reset pulse (low) at end if needed, 
    // or we just send zeros. 
    // We allocate this buffer effectively.
    // Making it part of handle consumes Stack if local or Heap/BSS.
    // We will allocate it inside the source file static to save handle size? No, keep logic portable.
    // 16-bit Timer CCR is common.
    uint16_t           DMA_Buffer[WS2812_MAX_LEDS * 24 + 1]; 
    
    uint8_t            Busy;       // Transfer in progress
} WS2812_HandleTypeDef;

/* Function Prototypes */

/**
 * @brief Initialize WS2812 Driver
 * @param hws Handle
 * @param htim Timer Handle (Must be configured for PWM + DMA)
 *             ARR should be set for 800kHz (e.g., 90-1 at 72MHz).
 * @param channel Timer Channel
 * @param num_leds Number of LEDs (<= WS2812_MAX_LEDS)
 */
void WS2812_Init(WS2812_HandleTypeDef *hws, TIM_HandleTypeDef *htim, uint32_t channel, uint16_t num_leds);

/**
 * @brief Set color of a specific LED
 * @param index LED Index
 * @param r Red (0-255)
 * @param g Green (0-255)
 * @param b Blue (0-255)
 */
void WS2812_SetPixelColor(WS2812_HandleTypeDef *hws, uint16_t index, uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief Set all LEDs to a color
 */
void WS2812_Fill(WS2812_HandleTypeDef *hws, uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief Update the LEDs (Transfer Data)
 *        Non-blocking if DMA is working.
 */
void WS2812_Show(WS2812_HandleTypeDef *hws);

/**
 * @brief DMA Callback - Must be called from HAL_TIM_PWM_PulseFinishedCallback
 */
void WS2812_DmaCallback(WS2812_HandleTypeDef *hws);

#endif // __WS2812_H
