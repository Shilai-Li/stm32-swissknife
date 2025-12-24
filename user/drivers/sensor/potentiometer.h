/**
 * @file potentiometer.h
 * @brief Potentiometer/Knob Driver
 * @details Uses ADC + Moving Average + Helper Mapping
 */

#ifndef POTENTIOMETER_H
#define POTENTIOMETER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdbool.h>
#include "middlewares/algorithms/moving_average.h"

typedef struct {
    ADC_HandleTypeDef *hadc;
    /* User responsible for channel selection logic if polling */
    
    uint16_t deadzone_low;  // e.g. 50 (Values < 50 become 0)
    uint16_t deadzone_high; // e.g. 4050 (Values > 4050 become 4095)
    bool     inverse;       // Reverse direction
} Pot_Config_t;

typedef struct {
    Pot_Config_t config;
    
    /* Composition: Algorithm */
    MovingAverage_Handle_t filter;
    uint16_t               filter_buffer[8]; // Smaller window (8) for faster response than light sensor
    
    /* State */
    uint16_t last_raw;
    uint16_t last_filtered;
} Pot_Handle_t;

/**
 * @brief Initialize Potentiometer
 */
void Pot_Init(Pot_Handle_t *handle, const Pot_Config_t *config);

/**
 * @brief Update and Read
 * @return Filtered ADC Value (0-4095)
 */
uint16_t Pot_Update(Pot_Handle_t *handle);

/**
 * @brief Get Percentage (0-100)
 */
uint8_t Pot_GetPercent(Pot_Handle_t *handle);

/**
 * @brief Get Float Ratio (0.0 - 1.0)
 */
float Pot_GetRatio(Pot_Handle_t *handle);

/**
 * @brief Map value to a custom range (e.g., -100 to +100 speed)
 */
int32_t Pot_Map(Pot_Handle_t *handle, int32_t min_out, int32_t max_out);

#ifdef __cplusplus
}
#endif

#endif // POTENTIOMETER_H
