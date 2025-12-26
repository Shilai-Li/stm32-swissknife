#include "ds18b20.h"
#include "uart.h"
#include <stdio.h>

#define UART_CH 0

static DS18B20_Handle_t sensor;
static char msg[128];

void ds18b20_error_callback(DS18B20_Handle_t *dev) {
    snprintf(msg, sizeof(msg), "[DS18B20] Error (Total: %lu)\r\n", dev->error_cnt);
    UART_SendString(UART_CH, msg);
}

void app_main(void) {
    UART_SendString(UART_CH, "\r\n===== DS18B20 OneWire Test =====\r\n");
    
    // Timer no longer needed
    
    DS18B20_Init(&sensor, GPIOB, GPIO_PIN_1);
    DS18B20_SetErrorCallback(&sensor, ds18b20_error_callback);
    
    UART_SendString(UART_CH, "DS18B20 initialized on PB1\r\n");
    UART_SendString(UART_CH, "Commands: [b]Blocking [n]Non-blocking\r\n");
    
    uint8_t cmd;
    
    while (1) {
        if (UART_Read(UART_CH, &cmd)) {
            switch (cmd) {
                case 'b':
                case 'B': {
                    UART_SendString(UART_CH, "Blocking read (750ms)...\r\n");
                    float temp = DS18B20_ReadTempBlocked(&sensor);
                    
                    if (temp != -999.0f) {
                        snprintf(msg, sizeof(msg), "Temperature: %.2f°C\r\n", temp);
                    } else {
                        snprintf(msg, sizeof(msg), "Read failed! Errors: %lu\r\n", 
                                sensor.error_cnt);
                    }
                    UART_SendString(UART_CH, msg);
                    break;
                }
                
                case 'n':
                case 'N': {
                    UART_SendString(UART_CH, "Starting conversion...\r\n");
                    DS18B20_StartConversion(&sensor);
                    UART_SendString(UART_CH, "Wait 750ms then read\r\n");
                    HAL_Delay(750);
                    
                    float temp = DS18B20_ReadTemp(&sensor);
                    if (temp != -999.0f) {
                        snprintf(msg, sizeof(msg), "Temperature: %.2f°C\r\n", temp);
                    } else {
                        snprintf(msg, sizeof(msg), "Read failed!\r\n");
                    }
                    UART_SendString(UART_CH, msg);
                    break;
                }
                
                case 's':
                case 'S':
                    snprintf(msg, sizeof(msg), 
                        "Statistics:\r\n  Success: %lu\r\n  Errors: %lu\r\n  Last: %.2f°C\r\n",
                        sensor.success_cnt, sensor.error_cnt, sensor.last_temp);
                    UART_SendString(UART_CH, msg);
                    break;
                    
                default:
                    UART_SendString(UART_CH, "Unknown cmd. Use: b/n/s\r\n");
                    break;
            }
        }
        
        HAL_Delay(10);
    }
}
