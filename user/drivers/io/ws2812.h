#ifndef __WS2812_H
#define __WS2812_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
// --- Configuration ---
#define WS2812_MAX_LEDS     64

// Timings (Dynamically calculated based on ARR)
#define WS2812_DUTY_0(period)  ((period * 32) / 100)
#define WS2812_DUTY_1(period)  ((period * 64) / 100)

typedef struct {
    TIM_HandleTypeDef *htim;
    uint32_t           Channel;
    uint16_t           NumLEDs;
    
    // Pixel Buffer (RGB)
    uint8_t            RGB_Buffer[WS2812_MAX_LEDS][3]; 
                                                       
    // DMA Buffer (Bit-timing buffer) - Warning: Large Size!
    // Recommend defining this struct as Global/Static, NOT on Stack.
    uint16_t           DMA_Buffer[WS2812_MAX_LEDS * 24 + 1]; 
    
    uint8_t            Busy;
} WS2812_HandleTypeDef;

// Init (htim must be PWM+DMA configured)
void WS2812_Init(WS2812_HandleTypeDef *hws, TIM_HandleTypeDef *htim, uint32_t channel, uint16_t num_leds);

// Set Single Pixel
void WS2812_SetPixelColor(WS2812_HandleTypeDef *hws, uint16_t index, uint8_t r, uint8_t g, uint8_t b);

// Fill All
void WS2812_Fill(WS2812_HandleTypeDef *hws, uint8_t r, uint8_t g, uint8_t b);

// Update/Refresh (Start DMA Transfer)
void WS2812_Show(WS2812_HandleTypeDef *hws);

// DMA Callback (Call from HAL_TIM_PWM_PulseFinishedCallback)
void WS2812_DmaCallback(WS2812_HandleTypeDef *hws);

#ifdef __cplusplus
}
#endif

#endif // __WS2812_H
