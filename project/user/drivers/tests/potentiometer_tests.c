/**
 * @file potentiometer_tests.c
 * @brief Test for Potentiometer
 */

#include "potentiometer.h"
#include "uart.h"
#include "delay.h"
#include <stdio.h>

extern ADC_HandleTypeDef hadc1; 

void Test_Pot_Entry(void) {
    UART_Init();
    UART_Debug_Printf("\r\n=== Potentiometer Test Start ===\r\n");

    Pot_Handle_t pot;
    Pot_Config_t config = {
        .hadc = &hadc1,
        .deadzone_low = 50,    // Ignore bottom jitter
        .deadzone_high = 4050, // Ignore top jitter
        .inverse = false
    };
    
    Pot_Init(&pot, &config);
    
    while(1) {
        uint16_t ra = Pot_Update(&pot);
        uint8_t pct = Pot_GetPercent(&pot);
        int32_t speed = Pot_Map(&pot, -100, 100); // Test mapping to motor speed
        
        // Use int hack for printing float
        float ratio = Pot_GetRatio(&pot);
        int r_int = (int)ratio;
        int r_frac = (int)((ratio - r_int) * 100);
        
        UART_Debug_Printf("Raw: %4d | %3d%% | Ratio: %d.%02d | MapSpeed: %4ld\r\n", 
            ra, pct, r_int, r_frac, speed);
        
        Delay_ms(100);
    }
}
