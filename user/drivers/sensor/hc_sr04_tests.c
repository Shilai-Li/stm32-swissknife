#include "hc_sr04.h"
#include "uart.h"
#include <stdio.h>

#define UART_CH 0

static HCSR04_HandleTypeDef ultrasonic;
static char msg[128];
static uint32_t measurement_count = 0;

void hc_sr04_error_callback(HCSR04_HandleTypeDef *dev) {
    snprintf(msg, sizeof(msg), "[HC-SR04] Error! Timeouts: %lu\r\n", dev->timeout_cnt);
    UART_SendString(UART_CH, msg);
}

void app_main(void) {
    UART_SendString(UART_CH, "\r\n===== HC-SR04 Ultrasonic Test =====\r\n");
    
    // Timer no longer required, uses DWT
    
    HCSR04_Init(&ultrasonic,
                GPIOA, GPIO_PIN_0,  // Trig
                GPIOA, GPIO_PIN_1); // Echo
    
    HCSR04_SetErrorCallback(&ultrasonic, hc_sr04_error_callback);
    
    UART_SendString(UART_CH, "HC-SR04: Trig=PA0, Echo=PA1\r\n");
    UART_SendString(UART_CH, "Measuring every 100ms...\r\n");
    UART_SendString(UART_CH, "Send 's' for statistics\r\n\r\n");
    
    uint8_t cmd;
    
    while (1) {
        float distance = HCSR04_Read(&ultrasonic);
        measurement_count++;
        
        if (distance > 0) {
            snprintf(msg, sizeof(msg), "[%lu] Distance: %.1f cm\r\n", 
                    measurement_count, distance);
            UART_SendString(UART_CH, msg);
        } else {
            snprintf(msg, sizeof(msg), "[%lu] Out of range / Error\r\n", 
                    measurement_count);
            UART_SendString(UART_CH, msg);
        }
        
        if (UART_Read(UART_CH, &cmd) && (cmd == 's' || cmd == 'S')) {
            snprintf(msg, sizeof(msg), 
                "\r\nStatistics:\r\n"
                "  Total Measurements: %lu\r\n"
                "  Success: %lu\r\n"
                "  Errors: %lu\r\n"
                "  Timeouts: %lu\r\n"
                "  Success Rate: %.1f%%\r\n\r\n",
                measurement_count,
                ultrasonic.success_cnt,
                ultrasonic.error_cnt,
                ultrasonic.timeout_cnt,
                (float)ultrasonic.success_cnt * 100.0f / measurement_count);
            UART_SendString(UART_CH, msg);
        }
        
        HAL_Delay(100);
    }
}
