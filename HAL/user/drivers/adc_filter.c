/**
 * @file adc_filter.c
 * @brief ADC Driver with Moving Average Filter Implementation
 * @author Standard Implementation
 * @date 2024
 */

#include "drivers/adc_filter.h"
#include <string.h>

void ADC_Filter_Init(ADC_Filter_HandleTypeDef *hfilter, ADC_HandleTypeDef *hadc, uint32_t channel, uint8_t window_size) {
    hfilter->hadc = hadc;
    hfilter->Channel = channel;
    
    if (window_size > ADC_FILTER_MAX_WINDOW) window_size = ADC_FILTER_MAX_WINDOW;
    if (window_size < 1) window_size = 1;
    
    hfilter->WindowSize = window_size;
    
    ADC_Filter_Reset(hfilter);
}

void ADC_Filter_Reset(ADC_Filter_HandleTypeDef *hfilter) {
    memset(hfilter->Buffer, 0, sizeof(hfilter->Buffer));
    hfilter->Index = 0;
    hfilter->Sum = 0;
    hfilter->Filled = 0;
}

uint16_t ADC_Filter_Update(ADC_Filter_HandleTypeDef *hfilter, uint16_t raw_value) {
    // If we haven't filled the buffer yet, just fill and return partial average or raw
    // To keep logic simple: Moving average 
    // NewSum = OldSum - OldestValue + NewValue
    
    if (hfilter->Filled < hfilter->WindowSize) {
        hfilter->Buffer[hfilter->Index] = raw_value;
        hfilter->Sum += raw_value;
        hfilter->Index++;
        
        if (hfilter->Index >= hfilter->WindowSize) {
            hfilter->Filled = hfilter->WindowSize;
            hfilter->Index = 0; // Wrap around
        }
        
        // Return average of current filled amount
        return (uint16_t)(hfilter->Sum / hfilter->Index); // Warning: Index is count here
    }
    
    // Normal operation (Buffer full)
    hfilter->Sum -= hfilter->Buffer[hfilter->Index];
    hfilter->Buffer[hfilter->Index] = raw_value;
    hfilter->Sum += raw_value;
    
    hfilter->Index++;
    if (hfilter->Index >= hfilter->WindowSize) {
        hfilter->Index = 0;
    }
    
    return (uint16_t)(hfilter->Sum / hfilter->WindowSize);
}

uint16_t ADC_Filter_Read(ADC_Filter_HandleTypeDef *hfilter) {
    // 1. Config Channel (Optional, if multiple channels are used sequentially)
    // Warning: Frequent reconfiguration adds overhead.
    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel = hfilter->Channel;
    sConfig.Rank = ADC_REGULAR_RANK_1; // For F1 series using standard sequence
    sConfig.SamplingTime = ADC_SAMPLETIME_55CYCLES_5; // or whatever standard
    
    // Note: If you have configured this in CubeMX as Rank 1, this config might be redundant 
    // or conflict if you are using Scan Mode.
    // For simple single channel polling driver, we assume User might want to switch channels.
    if (HAL_ADC_ConfigChannel(hfilter->hadc, &sConfig) != HAL_OK) {
        // Error handling
    }

    // 2. Start Conversion
    HAL_ADC_Start(hfilter->hadc);
    
    // 3. Poll for conversion
    if (HAL_ADC_PollForConversion(hfilter->hadc, 100) == HAL_OK) {
        uint16_t raw = (uint16_t)HAL_ADC_GetValue(hfilter->hadc);
        // 4. Update Filter
        return ADC_Filter_Update(hfilter, raw);
    }
    
    return 0; // Error
}

uint16_t ADC_Filter_GetAverage(ADC_Filter_HandleTypeDef *hfilter) {
    if (hfilter->Filled == 0 && hfilter->Index == 0) return 0;
    
    if (hfilter->Filled < hfilter->WindowSize) {
        if (hfilter->Index == 0) return 0;
        return (uint16_t)(hfilter->Sum / hfilter->Index);
    }
    
    return (uint16_t)(hfilter->Sum / hfilter->WindowSize);
}
