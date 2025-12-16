/**
 * @file ws2812.c
 * @brief WS2812B Driver Implementation
 * @author Standard Implementation
 * @date 2024
 */

#include "drivers/ws2812.h"
#include <string.h>

void WS2812_Init(WS2812_HandleTypeDef *hws, TIM_HandleTypeDef *htim, uint32_t channel, uint16_t num_leds) {
    hws->htim = htim;
    hws->Channel = channel;
    hws->NumLEDs = (num_leds > WS2812_MAX_LEDS) ? WS2812_MAX_LEDS : num_leds;
    hws->Busy = 0;
    
    // Clear Buffers
    WS2812_Fill(hws, 0, 0, 0);
    
    // Stop PWM initially
    HAL_TIM_PWM_Stop(htim, channel);
}

void WS2812_SetPixelColor(WS2812_HandleTypeDef *hws, uint16_t index, uint8_t r, uint8_t g, uint8_t b) {
    if (index >= hws->NumLEDs) return;
    hws->RGB_Buffer[index][0] = r;
    hws->RGB_Buffer[index][1] = g;
    hws->RGB_Buffer[index][2] = b;
}

void WS2812_Fill(WS2812_HandleTypeDef *hws, uint8_t r, uint8_t g, uint8_t b) {
    for (int i=0; i<hws->NumLEDs; i++) {
        hws->RGB_Buffer[i][0] = r;
        hws->RGB_Buffer[i][1] = g;
        hws->RGB_Buffer[i][2] = b;
    }
}

void WS2812_Show(WS2812_HandleTypeDef *hws) {
    if (hws->Busy) return; // Skip if previous transfer not done
    
    hws->Busy = 1;
    
    uint16_t period = hws->htim->Init.Period; 
    // Usually Configured ARR is Period-1 logic in registers, but Init.Period holds the set value.
    if (period == 0) period = 90; // Fallback default (~800kHz at 72MHz)
    
    uint16_t pwm_0 = WS2812_DUTY_0(period);
    uint16_t pwm_1 = WS2812_DUTY_1(period);
    
    uint32_t ptr = 0;
    
    for (int i=0; i<hws->NumLEDs; i++) {
        // WS2812 Protocol: GRB Order. MSB First.
        uint8_t g = hws->RGB_Buffer[i][1];
        uint8_t r = hws->RGB_Buffer[i][0];
        uint8_t b = hws->RGB_Buffer[i][2];
        
        // Green
        for (int j=7; j>=0; j--) {
            hws->DMA_Buffer[ptr++] = (g & (1 << j)) ? pwm_1 : pwm_0;
        }
        // Red
        for (int j=7; j>=0; j--) {
            hws->DMA_Buffer[ptr++] = (r & (1 << j)) ? pwm_1 : pwm_0;
        }
        // Blue
        for (int j=7; j>=0; j--) {
            hws->DMA_Buffer[ptr++] = (b & (1 << j)) ? pwm_1 : pwm_0;
        }
    }
    
    // Set End pulse (0)
    hws->DMA_Buffer[ptr] = 0; 
    
    // Start DMA
    // Note: DMA Buffer length = NumLEDs * 24 + 1
    // Need to cast buffer for HAL API
    HAL_TIM_PWM_Start_DMA(hws->htim, hws->Channel, (uint32_t*)hws->DMA_Buffer, ptr + 1);
}

void WS2812_DmaCallback(WS2812_HandleTypeDef *hws) {
    // Stop PWM to generate Reset code (>50us low) automatically since last cycle was 0 or just stop
    HAL_TIM_PWM_Stop_DMA(hws->htim, hws->Channel);
    // Be careful: Stop_DMA usually stops output.
    // Ensure the output is Low when Idle.
    // The last element of buffer was 0, so duty dropped to 0.
    
    hws->Busy = 0;
}
