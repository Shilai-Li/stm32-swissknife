/**
 * @file ds18b20_tests.c
 * @brief Test for DS18B20
 */

#include "ds18b20.h"
#include "uart.h"
#include "delay.h"
#include <stdio.h>

// Adjust Pin Config
#define DS_PORT GPIOA
#define DS_PIN  GPIO_PIN_1

void app_main(void) {
    UART_Init();
    // Delay Init is implicit usually via global init, but ensure it works.
    
    UART_Debug_Printf("\r\n=== DS18B20 Test Start ===\r\n");

    DS18B20_Handle_t sensor;
    DS18B20_Init(&sensor, DS_PORT, DS_PIN);
    
    UART_Debug_Printf("Reading Temperature (Loop)...\r\n");

    while(1) {
        float t = DS18B20_ReadTempBlocked(&sensor);
        
        if (t < -900.0f) {
            UART_Debug_Printf("Error: Sensor Not Found\r\n");
        } else {
            // Print float via integer hack if %f not supported
            int t_int = (int)t;
            int t_frac = (int)((t - t_int) * 100);
            if(t_frac < 0) t_frac = -t_frac;
            
            UART_Debug_Printf("Temp: %d.%02d C\r\n", t_int, t_frac);
        }
        
        Delay_ms(1000);
    }
}
