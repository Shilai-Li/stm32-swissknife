/**
 * @file adc_tests.c
 * @brief ADC Driver Test Code
 */

#include "adc_filter.h"
#include "uart.h"

// --- Configuration ---
// Requires ADC1 initialized in main.c
// Assume Channel 0 (PA0) is connected to something (potentiometer or sensor)
 extern ADC_HandleTypeDef hadc1; 

ADC_Filter_HandleTypeDef hadc_filter;

void User_Entry(void)
{
    UART_Init();
    UART_Debug_Printf("\r\n--- ADC Filter Test Start ---\r\n");

    // 1. Initialize
    // Window size 10 means average last 10 samples
    UART_Debug_Printf("Initializing ADC Filter (Window 10)...\r\n");
    ADC_Filter_Init(&hadc_filter, &hadc1, ADC_CHANNEL_0, 10);
    
    // 2. Poll Loop
    UART_Debug_Printf("Reading Sensor...\r\n");
    
    while (1) {
        // Method 1: Blocking Read (easiest for single channel)
        uint16_t filtered_val = ADC_Filter_Read(&hadc_filter);
        
        // Voltage calculation (Assuming 3.3V ref, 12-bit)
        float voltage = (filtered_val * 3.3f) / 4095.0f;
        
        // Print
        // Note: %f might require compiler flags for floats. Using integer parts for safety.
        // Or using formatted string logic.
        int v_int = (int)voltage;
        int v_frac = (int)((voltage - v_int) * 1000);
        
        // UART_Debug_Printf("Raw: %d -> Volts: %d.%03dV\r\n", filtered_val, v_int, v_frac);
        // Only print every 500ms to avoid spam
        static uint32_t last_print = 0;
        if (HAL_GetTick() - last_print > 500) {
            UART_Debug_Printf("ADC: %4d | %d.%03dV\r\n", filtered_val, v_int, v_frac);
            last_print = HAL_GetTick();
        }
        
        HAL_Delay(10); // 100Hz Sampling
    }
}
