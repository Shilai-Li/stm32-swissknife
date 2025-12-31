#include "light_sensor.h"
#include "stm32f4xx_hal.h"

static void LightSensor_HandleError(LightSensor_Handle_t *h) {
    if (h) {
        h->error_cnt++;
        if (h->error_cb) {
            h->error_cb(h);
        }
    }
}

void LightSensor_SetErrorCallback(LightSensor_Handle_t *handle, LightSensor_ErrorCallback cb) {
    if (handle) {
        handle->error_cb = cb;
    }
}

void LightSensor_Init(LightSensor_Handle_t *h, const LightSensor_Config_t *config) {
    if (!h || !config) return;
    
    h->config = *config;
    
    // Init Filter
    MovingAverage_Init(&h->filter, h->filter_buffer, 16);
    
    h->is_dark = false;
    h->last_raw = 0;
    h->last_filtered = 0;
    
    h->error_cnt = 0;
    h->success_cnt = 0;
    h->error_cb = NULL;
}

uint16_t LightSensor_Update(LightSensor_Handle_t *h) {
    if (!h || !h->config.hadc) return 0;
    
    ADC_HandleTypeDef *hadc = (ADC_HandleTypeDef*)h->config.hadc;
    
    // 1. Read Hardware
    HAL_ADC_Start(hadc);
    
    HAL_StatusTypeDef status = HAL_ADC_PollForConversion(hadc, 10);
    if (status != HAL_OK) {
        LightSensor_HandleError(h);
        return h->last_filtered;
    }
    
    uint16_t raw = (uint16_t)HAL_ADC_GetValue(hadc);
    h->last_raw = raw;
    h->success_cnt++;
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
