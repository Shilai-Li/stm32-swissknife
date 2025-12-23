/**
 * @file potentiometer.c
 * @brief Potentiometer Driver Implementation
 */

#include "potentiometer.h"

void Pot_Init(Pot_Handle_t *h, const Pot_Config_t *config) {
    if (!h || !config) return;
    
    h->config = *config;
    
    // Init Filter (Window 8)
    MovingAverage_Init(&h->filter, h->filter_buffer, 8);
    
    h->last_raw = 0;
    h->last_filtered = 0;
}

uint16_t Pot_Update(Pot_Handle_t *h) {
    if (!h || !h->config.hadc) return 0;
    
    // 1. Hardware Read (Simple Poll)
    HAL_ADC_Start(h->config.hadc);
    HAL_ADC_PollForConversion(h->config.hadc, 10);
    uint16_t raw = (uint16_t)HAL_ADC_GetValue(h->config.hadc);
    
    h->last_raw = raw;
    
    // 2. Filter
    uint16_t val = MovingAverage_Update(&h->filter, raw);
    
    // 3. Inverse
    if (h->config.inverse) {
        val = 4095 - val;
    }
    
    // 4. Deadzone
    if (val < h->config.deadzone_low) val = 0;
    if (val > h->config.deadzone_high) val = 4095;
    
    h->last_filtered = val;
    return val;
}

uint8_t Pot_GetPercent(Pot_Handle_t *h) {
    return (uint8_t)((uint32_t)h->last_filtered * 100 / 4095);
}

float Pot_GetRatio(Pot_Handle_t *h) {
    return (float)h->last_filtered / 4095.0f;
}

int32_t Pot_Map(Pot_Handle_t *h, int32_t min_out, int32_t max_out) {
    // Standard Linear Map
    // Y = (X - InMin) * (OutRange) / InRange + OutMin
    // InMin = 0, InMax = 4095
    
    int64_t val = h->last_filtered; // Use 64-bit to prevent overflow during multiply
    int64_t range = max_out - min_out;
    
    int64_t res = (val * range) / 4095 + min_out;
    return (int32_t)res;
}
