/**
 * @file rs485.h
 * @brief RS485 Transceiver Driver Header File
 * @author Standard Implementation
 * @date 2024
 */

#ifndef __RS485_H
#define __RS485_H

#include "main.h"

#ifndef __STM32F1xx_HAL_UART_H
#include "main.h"
#endif

typedef struct {
    UART_HandleTypeDef *huart;       // Pointer to UART Handle
    GPIO_TypeDef       *DePort;      // Driver Enable / Receive Enable Port
    uint16_t            DePin;       // Driver Enable Pin
    
    // Optional: Buffer management for Async (Interrupt/DMA) mode
    uint8_t            *RxBuffer;
    uint16_t            RxBufferSize;
    volatile uint16_t   RxIndex;
} RS485_HandleTypeDef;

/* Function Prototypes */

/**
 * @brief Initialize RS485 struct
 */
void RS485_Init(RS485_HandleTypeDef *hrs485, UART_HandleTypeDef *huart, GPIO_TypeDef *de_port, uint16_t de_pin);

/**
 * @brief Blocking Transmit (Safest for RS485 timing)
 */
void RS485_Send(RS485_HandleTypeDef *hrs485, uint8_t *pData, uint16_t Size, uint32_t Timeout);

/**
 * @brief Blocking Receive
 */
uint8_t RS485_Receive(RS485_HandleTypeDef *hrs485, uint8_t *pData, uint16_t Size, uint32_t Timeout);

/**
 * @brief Send String (Blocking)
 */
void RS485_Printf(RS485_HandleTypeDef *hrs485, const char *format, ...);

#endif // __RS485_H
