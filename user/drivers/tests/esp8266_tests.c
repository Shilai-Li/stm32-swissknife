/**
 * @file esp8266_tests.c
 * @brief Test Suite for ESP8266 Driver
 */

#include "esp8266.h"
#include <stdio.h>
#include "uart.h"
#include "delay.h"

// Adjust these to match your hardware wiring
// Example assumes ESP8266 is on UART2, Debug is on UART1/UART3 or we just print to same.
// In this project configuration, the User likely needs to check `main.c` or schematic.
// For the test, we assume UART_CHANNEL_3 for ESP and UART_CHANNEL_2 (Default Debug) for Logs.
// NOTE: Verify your board setup!

#define ESP_UART_CH     UART_CHANNEL_3
#define DEBUG_UART_CH   UART_CHANNEL_2

ESP8266_Handle_t esp;

void app_main(void) {
    UART_Init();
    
    UART_Debug_Printf("=== ESP8266 Test Start ===\r\n");

    ESP8266_Config_t config = {
        .cmd_uart = ESP_UART_CH,
        .debug_uart = DEBUG_UART_CH,
        .timeout_ms = 2000,
        .echo_off = true
    };
    
    UART_Debug_Printf("Initializing ESP8266 on Channel %d...\r\n", ESP_UART_CH);
    
    if (ESP8266_Init(&esp, &config) == ESP8266_OK) {
        UART_Debug_Printf("INIT CHECK: OK\r\n");
    } else {
        UART_Debug_Printf("INIT CHECK: FAIL\r\n");
        return;
    }
    
    // Scan/Join Test (Modify Credentials if testing real connection)
    // ESP8266_JoinAP(&esp, "MyWiFi", "MyPassword");

    while (1) {
        // Echo Loop for manual diagnosis
        uint8_t c;
        // From PC to ESP
        if (UART_Read(DEBUG_UART_CH, &c)) {
            UART_Send(ESP_UART_CH, &c, 1);
        }
        // From ESP to PC
        if (UART_Read(ESP_UART_CH, &c)) {
            UART_Send(DEBUG_UART_CH, &c, 1);
        }
    }
}
