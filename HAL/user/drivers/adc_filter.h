/**
 * @file adc_filter.h
 * @brief ADC Driver with Moving Average Filter Wrapper
 * @author Standard Implementation
 * @date 2024
 */

#ifndef __ADC_FILTER_H
#define __ADC_FILTER_H

#include "main.h"

#ifndef __STM32F1xx_HAL_ADC_H
#include "stm32f1xx_hal.h"
#endif

// --- Configuration ---
// Maximum window size for moving average
// Higher = Smoother but lagged response
// Lower = Faster but noisier
#define ADC_FILTER_MAX_WINDOW 32

typedef struct {
    ADC_HandleTypeDef *hadc;
    uint32_t           Channel; // ADC Channel config (ADC_CHANNEL_x)
    
    // Filtering State
    uint16_t           Buffer[ADC_FILTER_MAX_WINDOW];
    uint8_t            WindowSize;
    uint8_t            Index;
    uint32_t           Sum;
    uint8_t            Filled; // Flag to indicate if buffer is full
} ADC_Filter_HandleTypeDef;

/* Function Prototypes */

/**
 * @brief Initialize the ADC Filter Driver
 * @param hfilter Handle
 * @param hadc HAL ADC Handle
 * @param channel ADC Channel (e.g. ADC_CHANNEL_0). Warning: In polling mode, we often config rank manually or assume single conversion.
 *                This driver assumes the ADC is configured to read the desired channel, 
 *                OR this channel param is used to SwitchChannelConfig if using multi-channel polling.
 * @param window_size Moving average window size (1 to ADC_FILTER_MAX_WINDOW)
 */
void ADC_Filter_Init(ADC_Filter_HandleTypeDef *hfilter, ADC_HandleTypeDef *hadc, uint32_t channel, uint8_t window_size);

/**
 * @brief Update the filter with a new raw reading.
 *        Usually called after HAL_ADC_GetValue().
 * @param raw_value The raw 12-bit ADC value
 * @return The current filtered average
 */
uint16_t ADC_Filter_Update(ADC_Filter_HandleTypeDef *hfilter, uint16_t raw_value);

/**
 * @brief Perform a blocking read and update filter (Simple Polling)
 *        NOTE: This function assumes the ADC is set up for Single Conversion or Software Trigger.
 *        If using DMA, do not use this, use ADC_Filter_Update in the DMA callback.
 * @return The current filtered average
 */
uint16_t ADC_Filter_Read(ADC_Filter_HandleTypeDef *hfilter);

/**
 * @brief Reset the filter history
 */
void ADC_Filter_Reset(ADC_Filter_HandleTypeDef *hfilter);

/**
 * @brief Get current average without updating
 */
uint16_t ADC_Filter_GetAverage(ADC_Filter_HandleTypeDef *hfilter);

#endif // __ADC_FILTER_H
