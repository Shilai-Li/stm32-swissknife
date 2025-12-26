/**
 * @file light_sensor_tests.c
 * @brief Test for Light Sensor & Filter
 */

#include "light_sensor.h"
#include "uart.h"
#include "delay.h"
#include <stdio.h>

extern ADC_HandleTypeDef hadc1; // Assume ADC1 used

void app_main(void) {
    UART_Init();
    UART_Debug_Printf("\r\n=== Light Sensor Test Start ===\r\n");

    LightSensor_Handle_t sensor;
    LightSensor_Config_t config = {
        .hadc = &hadc1,
        .inverse_logic = false, // Assume LDR to VCC, PullDown Resistor (High=Light)
        .dark_threshold = 1000,
        .light_threshold = 2000 // Hysteresis Window 1000-2000
    };
    
    LightSensor_Init(&sensor, &config);
    
    while(1) {
        uint16_t val = LightSensor_Update(&sensor);
        bool dark = LightSensor_IsDark(&sensor);
        uint8_t pct = LightSensor_GetIntensityPercentage(&sensor);
        
        UART_Debug_Printf("ADC: %4d | Intensity: %3d%% | Status: %s\r\n", 
            val, pct, dark ? "DARK" : "LIGHT");
        
        Delay_ms(200);
    }
}
