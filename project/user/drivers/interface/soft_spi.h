/**
 * @file soft_spi.h
 * @brief Software SPI (Bit-Banging) Driver Header File
 * @author Standard Implementation
 * @date 2024
 */

#ifndef __SOFT_SPI_H
#define __SOFT_SPI_H

#include "main.h"

// SPI Modes
// Mode 0: CPOL=0, CPHA=0 (Idle Low, Capture on 1st Edge/Rising)
// Mode 1: CPOL=0, CPHA=1 (Idle Low, Capture on 2nd Edge/Falling)
// Mode 2: CPOL=1, CPHA=0 (Idle High, Capture on 1st Edge/Falling)
// Mode 3: CPOL=1, CPHA=1 (Idle High, Capture on 2nd Edge/Rising)
#define SOFT_SPI_MODE_0  0
#define SOFT_SPI_MODE_1  1
#define SOFT_SPI_MODE_2  2
#define SOFT_SPI_MODE_3  3

typedef struct {
    GPIO_TypeDef *SckPort;
    uint16_t      SckPin;
    GPIO_TypeDef *MosiPort;
    uint16_t      MosiPin;
    GPIO_TypeDef *MisoPort; // Optional, can be NULL
    uint16_t      MisoPin;
    uint8_t       Mode;     // SOFT_SPI_MODE_x
    uint32_t      DelayTicks;
} Soft_SPI_HandleTypeDef;

/* Function Prototypes */

void Soft_SPI_Init(Soft_SPI_HandleTypeDef *hspi, 
                  GPIO_TypeDef *sck_port, uint16_t sck_pin,
                  GPIO_TypeDef *mosi_port, uint16_t mosi_pin,
                  GPIO_TypeDef *miso_port, uint16_t miso_pin,
                  uint8_t mode);

uint8_t Soft_SPI_Transmit(Soft_SPI_HandleTypeDef *hspi, uint8_t *pData, uint16_t Size, uint32_t Timeout);
uint8_t Soft_SPI_Receive(Soft_SPI_HandleTypeDef *hspi, uint8_t *pData, uint16_t Size, uint32_t Timeout);
uint8_t Soft_SPI_TransmitReceive(Soft_SPI_HandleTypeDef *hspi, uint8_t *pTxData, uint8_t *pRxData, uint16_t Size, uint32_t Timeout);

#endif // __SOFT_SPI_H
