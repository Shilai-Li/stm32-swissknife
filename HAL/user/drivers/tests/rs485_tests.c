/**
 * @file rs485_tests.c
 * @brief RS485 Driver Test Code
 */

#include "drivers/rs485.h"
#include "drivers/uart.h"

// --- Configuration ---
// Requires a UART and a GPIO for DE
 extern UART_HandleTypeDef huart2; // Use UART2 for RS485 for example, UART1 for DebugConsole

#ifndef RS485_DE_PORT
#define RS485_DE_PORT   GPIOA
#define RS485_DE_PIN    GPIO_PIN_1
#endif

RS485_HandleTypeDef hrs485_dev;

void User_Entry(void)
{
    // Initialize Debug UART (UART1 usually)
    UART_Init();
    UART_Debug_Printf("\r\n--- RS485 Test Start ---\r\n");

    // Initialize RS485 (UART2 + GPIO)
    // Note: User must have initialized huart2 in main.c via CubeMX
    RS485_Init(&hrs485_dev, &huart2, RS485_DE_PORT, RS485_DE_PIN);
    
    UART_Debug_Printf("RS485 Initialized. Sending Ping...\r\n");
    
    // 1. Send Test
    RS485_Printf(&hrs485_dev, "Hello RS485 World!\r\n");
    HAL_Delay(100);
    
    // 2. Loopback Echo Test (Need external loopback or another device)
    UART_Debug_Printf("Entering Echo Mode. Send data to RS485 to see it echoed.\r\n");
    
    uint8_t rx_byte;
    while (1) {
        // Simple blocking receive for 1 byte with short timeout to allow other tasks
        if (RS485_Receive(&hrs485_dev, &rx_byte, 1, 100) == HAL_OK) {
            // Echo back via RS485
            RS485_Send(&hrs485_dev, &rx_byte, 1, 100);
            
            // Also print to debug console
            // UART_Debug_Printf("RS485 Rx: %c\r\n", rx_byte);
        }
        
        // Blink optional status LED
        // HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
    }
}
