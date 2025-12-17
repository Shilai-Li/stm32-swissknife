/**
 * @file soft_spi.c
 * @brief Software SPI (Bit-Banging) Driver Implementation
 * @author Standard Implementation
 * @date 2024
 */

#include "soft_spi.h"

// --- Private Functions ---

static void Soft_SPI_Delay(Soft_SPI_HandleTypeDef *hspi) {
    volatile uint32_t i = hspi->DelayTicks;
    while(i--);
}

// Low level pin ops
#define SCK_H()   HAL_GPIO_WritePin(hspi->SckPort, hspi->SckPin, GPIO_PIN_SET)
#define SCK_L()   HAL_GPIO_WritePin(hspi->SckPort, hspi->SckPin, GPIO_PIN_RESET)
#define MOSI_H()  HAL_GPIO_WritePin(hspi->MosiPort, hspi->MosiPin, GPIO_PIN_SET)
#define MOSI_L()  HAL_GPIO_WritePin(hspi->MosiPort, hspi->MosiPin, GPIO_PIN_RESET)
#define MISO_RD() (hspi->MisoPort ? HAL_GPIO_ReadPin(hspi->MisoPort, hspi->MisoPin) : 0)

// Single Byte Transmit/Receive
static uint8_t Soft_SPI_TxRx_Byte(Soft_SPI_HandleTypeDef *hspi, uint8_t data) {
    uint8_t rx_data = 0;
    
    // Setup for CPOL/CPHA
    // Mode 0: CPOL=0 (Idle L), CPHA=0 (Sample Rising, Shift Falling - effectively data valid on Rising)
    // Mode 3: CPOL=1 (Idle H), CPHA=1 (Sample Rising, Shift Falling - similar data validity)
    
    // Simple implementation loop
    // MSB First
    
    for (uint8_t i = 0; i < 8; i++) {
        // 1. Setup Data (MOSI)
        if (hspi->Mode == SOFT_SPI_MODE_0 || hspi->Mode == SOFT_SPI_MODE_2) {
            // CPHA=0: Data must be valid BEFORE leading edge
            if (data & 0x80) MOSI_H(); else MOSI_L();
            Soft_SPI_Delay(hspi); 
            
            // Leading Edge (Sample)
            if (hspi->Mode == SOFT_SPI_MODE_0) SCK_H(); else SCK_L();
            
            if (MISO_RD()) rx_data |= (0x80 >> i); // Sample MISO
            Soft_SPI_Delay(hspi);
            
            // Trailing Edge (Shift)
            if (hspi->Mode == SOFT_SPI_MODE_0) SCK_L(); else SCK_H();
            
        } else {
            // CPHA=1: Data change on leading edge, Sample on trailing edge
            // Leading Edge (Change)
            if (hspi->Mode == SOFT_SPI_MODE_1) SCK_H(); else SCK_L();
            
            if (data & 0x80) MOSI_H(); else MOSI_L(); // Change Data
            Soft_SPI_Delay(hspi);
            
            // Trailing Edge (Sample)
            if (hspi->Mode == SOFT_SPI_MODE_1) SCK_L(); else SCK_H();
            
            if (MISO_RD()) rx_data |= (0x80 >> i); // Sample MISO
            Soft_SPI_Delay(hspi);
        }
        
        data <<= 1;
    }
    
    return rx_data;
}

// --- Public Functions ---

void Soft_SPI_Init(Soft_SPI_HandleTypeDef *hspi, 
                  GPIO_TypeDef *sck_port, uint16_t sck_pin,
                  GPIO_TypeDef *mosi_port, uint16_t mosi_pin,
                  GPIO_TypeDef *miso_port, uint16_t miso_pin,
                  uint8_t mode)
{
    hspi->SckPort = sck_port; hspi->SckPin = sck_pin;
    hspi->MosiPort = mosi_port; hspi->MosiPin = mosi_pin;
    hspi->MisoPort = miso_port; hspi->MisoPin = miso_pin;
    hspi->Mode = mode;
    hspi->DelayTicks = 5; // ~1-2MHz depending on CPU
    
    // Idle State
    if (mode == SOFT_SPI_MODE_0 || mode == SOFT_SPI_MODE_1) {
        SCK_L();
    } else {
        SCK_H();
    }
}

uint8_t Soft_SPI_Transmit(Soft_SPI_HandleTypeDef *hspi, uint8_t *pData, uint16_t Size, uint32_t Timeout) {
    for (uint16_t i = 0; i < Size; i++) {
        Soft_SPI_TxRx_Byte(hspi, pData[i]);
    }
    return 0; // HAL_OK
}

uint8_t Soft_SPI_Receive(Soft_SPI_HandleTypeDef *hspi, uint8_t *pData, uint16_t Size, uint32_t Timeout) {
    for (uint16_t i = 0; i < Size; i++) {
        pData[i] = Soft_SPI_TxRx_Byte(hspi, 0xFF); // Send dummy
    }
    return 0;
}

uint8_t Soft_SPI_TransmitReceive(Soft_SPI_HandleTypeDef *hspi, uint8_t *pTxData, uint8_t *pRxData, uint16_t Size, uint32_t Timeout) {
    for (uint16_t i = 0; i < Size; i++) {
        pRxData[i] = Soft_SPI_TxRx_Byte(hspi, pTxData[i]);
    }
    return 0;
}
