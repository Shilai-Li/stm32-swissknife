#include "dht11.h"
#include "uart.h"
#include <stdio.h>

#define UART_CH 0

static DHT11_Handle_t dht11;
static char msg[128];

void dht11_error_callback(DHT11_Handle_t *dev) {
    snprintf(msg, sizeof(msg), "[DHT11] Error occurred (Total: %u)\r\n", dev->error_cnt);
    UART_SendString(UART_CH, msg);
}

void app_main(void) {
    UART_SendString(UART_CH, "\r\n===== DHT11 Driver Test =====\r\n");
    
    // DWT Delay needs no explicit timer start (it auto-inits on use)
    
    DHT11_Init(&dht11, GPIOA, GPIO_PIN_1);
    DHT11_SetErrorCallback(&dht11, dht11_error_callback);
    
    UART_SendString(UART_CH, "DHT11 initialized on PA1\r\n");
    UART_SendString(UART_CH, "Reading every 3 seconds...\r\n");
    
    while (1) {
        DHT11_Status status = DHT11_Read(&dht11);
        
        switch (status) {
            case DHT11_OK:
                snprintf(msg, sizeof(msg), 
                    "Temp: %d.%d°C | Humidity: %d.%d%% | Success: %u\r\n",
                    dht11.temp_int, dht11.temp_dec,
                    dht11.humidity_int, dht11.humidity_dec,
                    dht11.successful_read_cnt);
                UART_SendString(UART_CH, msg);
                break;
                
            case DHT11_ERROR_CHECKSUM:
                snprintf(msg, sizeof(msg), 
                    "Checksum Error (Total: %u)\r\n",
                    dht11.checksum_error_cnt);
                UART_SendString(UART_CH, msg);
                break;
                
            case DHT11_ERROR_TIMEOUT:
                snprintf(msg, sizeof(msg), 
                    "Timeout (Total: %u) - Check wiring!\r\n",
                    dht11.timeout_cnt);
                UART_SendString(UART_CH, msg);
                break;
                
            case DHT11_ERROR_GPIO:
                UART_SendString(UART_CH, "GPIO Error - Check init!\r\n");
                break;
        }
        
        snprintf(msg, sizeof(msg), "Stats: Errors=%u, Success=%u\r\n\r\n",
                 dht11.error_cnt, dht11.successful_read_cnt);
        UART_SendString(UART_CH, msg);
        
        HAL_Delay(3000);
    }
}
