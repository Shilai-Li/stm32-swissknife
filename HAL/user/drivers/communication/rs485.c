/**
 * @file rs485.c
 * @brief RS485 Transceiver Driver Implementation
 * @author Standard Implementation
 * @date 2024
 */

#include "drivers/rs485.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

// Helper Macros
#define RS485_TX_MODE(h)  HAL_GPIO_WritePin(h->DePort, h->DePin, GPIO_PIN_SET)
#define RS485_RX_MODE(h)  HAL_GPIO_WritePin(h->DePort, h->DePin, GPIO_PIN_RESET)

void RS485_Init(RS485_HandleTypeDef *hrs485, UART_HandleTypeDef *huart, GPIO_TypeDef *de_port, uint16_t de_pin) {
    hrs485->huart = huart;
    hrs485->DePort = de_port;
    hrs485->DePin = de_pin;
    
    // Default state: Receive Mode (Bus Idle)
    RS485_RX_MODE(hrs485);
}

void RS485_Send(RS485_HandleTypeDef *hrs485, uint8_t *pData, uint16_t Size, uint32_t Timeout) {
    // 1. Enable Driver (High)
    RS485_TX_MODE(hrs485);
    
    // Optional: Small delay if hardware transceiver switch time is slow (usually not needed for modern chips like MAX485, but safe for older ones)
    // for(volatile int i=0; i<100; i++); 

    // 2. Transmit
    HAL_UART_Transmit(hrs485->huart, pData, Size, Timeout);
    
    // 3. Disable Driver (Low) -> Back to RX
    // Ideally we should wait for TC (Transmission Complete) flag, NOT just TXE.
    // HAL_UART_Transmit usually waits for TC, but doubled check ensures we don't cut off the last stop bit.
    
    // Wait for TC flag ensuring last bit has shifted out entirely
    // Note: HAL_UART_Transmit handles Timeout, but explicit wait is RS485 best practice before flipping the pin.
    while(__HAL_UART_GET_FLAG(hrs485->huart, UART_FLAG_TC) == RESET);

    RS485_RX_MODE(hrs485);
}

uint8_t RS485_Receive(RS485_HandleTypeDef *hrs485, uint8_t *pData, uint16_t Size, uint32_t Timeout) {
    // Ensure we are in RX mode (should be default, but enforce)
    RS485_RX_MODE(hrs485);
    
    return HAL_UART_Receive(hrs485->huart, pData, Size, Timeout);
}

void RS485_Printf(RS485_HandleTypeDef *hrs485, const char *format, ...) {
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    RS485_Send(hrs485, (uint8_t*)buffer, strlen(buffer), 1000);
}
