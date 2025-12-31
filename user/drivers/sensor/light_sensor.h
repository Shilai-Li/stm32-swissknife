#ifndef LIGHT_SENSOR_H
#define LIGHT_SENSOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdbool.h>
#include "moving_average.h"

/**
 * @brief Light Sensor Configuration
 */
typedef struct {
    void *hadc; // Generic pointer to Hardware ADC Handle (cast to ADC_HandleTypeDef* in implementation)
    /* 
       Note: The user code is responsible for configuring ADC Rank/Channel 
       or using a wrapper that handles channel switching if polling.
       This driver assumes reading hADC gets the LDR value.
    */
    
    uint16_t dark_threshold;   // ADC value below which is considered 'Dark'
    uint16_t light_threshold;  // ADC value above which is considered 'Light' 
                               // (Use gap for hysteresis)
    
    bool inverse_logic;        // If true, Low ADC = Light (depends on circuit: Pull-up vs Pull-down)
} LightSensor_Config_t;

/* Forward declaration */
typedef struct LightSensor_Handle_s LightSensor_Handle_t;
typedef void (*LightSensor_ErrorCallback)(LightSensor_Handle_t *dev);

struct LightSensor_Handle_s {
    LightSensor_Config_t config;
    
    /* Composition: Algorithm */
    MovingAverage_Handle_t filter;
    uint16_t               filter_buffer[16]; // Fixed window size 16
    
    /* State */
    bool is_dark;
    uint16_t last_raw;
    uint16_t last_filtered;
    
    /* Stats */
    volatile uint32_t error_cnt;
    volatile uint32_t success_cnt;
    
    /* Callback */
    LightSensor_ErrorCallback error_cb;
};

void LightSensor_SetErrorCallback(LightSensor_Handle_t *handle, LightSensor_ErrorCallback cb);

/**
 * @brief Initialize Light Sensor
 */
void LightSensor_Init(LightSensor_Handle_t *handle, const LightSensor_Config_t *config);

/**
 * @brief Update Sensor State (Fast)
 * @details Reads ADC, updates filter, updates logic state
 * @return Filtered ADC Value
 */
uint16_t LightSensor_Update(LightSensor_Handle_t *handle);

/**
 * @brief Check if currently 'Dark' (with Hysteresis)
 */
bool LightSensor_IsDark(LightSensor_Handle_t *handle);

/**
 * @brief Get Raw Lux (Approximation purely based on ADC ratio)
 * @return 0-100% intensity
 */
uint8_t LightSensor_GetIntensityPercentage(LightSensor_Handle_t *handle);

#ifdef __cplusplus
}
#endif

#endif // LIGHT_SENSOR_H
