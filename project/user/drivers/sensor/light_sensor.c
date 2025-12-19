/**
 * @file light_sensor.c
 * @brief Light Sensor Driver Implementation
 */

#include "light_sensor.h"

void LightSensor_Init(LightSensor_Handle_t *h, const LightSensor_Config_t *config) {
    if (!h || !config) return;
    
    h->config = *config;
    
    // Init Filter
    // We used a fixed internal buffer in struct for convenience
    MovingAverage_Init(&h->filter, h->filter_buffer, 16);
    
    h->is_dark = false;
    h->last_raw = 0;
    h->last_filtered = 0;
}

uint16_t LightSensor_Update(LightSensor_Handle_t *h) {
    if (!h || !h->config.hadc) return 0;
    
    // 1. Read Hardware
    // Assuming Polling for simplicity, or user calls this after DMA ISR
    // If strict polling:
    HAL_ADC_Start(h->config.hadc);
    HAL_ADC_PollForConversion(h->config.hadc, 10);
    uint16_t raw = (uint16_t)HAL_ADC_GetValue(h->config.hadc);
    
    h->last_raw = raw;
    
    // 2. Apply Algorithm
    uint16_t filtered = MovingAverage_Update(&h->filter, raw);
    h->last_filtered = filtered;
    
    // 3. Update Logic State (Hysteresis)
    /*
       Logic:
       If Inverse (Low = Light):
          val < Light_Thresh -> Light
          val > Dark_Thresh -> Dark
       
       Standard LDR + PullDown (High = Low Res = Light):
          val > Light_Thresh -> Light
          val < Dark_Thresh -> Dark
    */
    
    if (h->config.inverse_logic) {
        // Low Value = Bright Light
        // High Value = Dark
        if (filtered > h->config.dark_threshold) {
             h->is_dark = true;
        } else if (filtered < h->config.light_threshold) {
             h->is_dark = false;
        }
        // In between: keep previous state
    } else {
        // High Value = Bright Light
        // Low Value = Dark
        if (filtered < h->config.dark_threshold) {
            h->is_dark = true; 
        } else if (filtered > h->config.light_threshold) {
            h->is_dark = false;
        }
    }
    
    return filtered;
}

bool LightSensor_IsDark(LightSensor_Handle_t *h) {
    return h->is_dark;
}

uint8_t LightSensor_GetIntensityPercentage(LightSensor_Handle_t *h) {
    uint32_t val = h->last_filtered;
    if (h->config.inverse_logic) {
        // 4095 is Dark (0%), 0 is Bright (100%)
        return (uint8_t)(100 - (val * 100 / 4095));
    } else {
        // 0 is Dark, 4095 is Bright
        return (uint8_t)(val * 100 / 4095);
    }
}
