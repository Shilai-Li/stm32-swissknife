/**
 * @file nrf24l01.h
 * @brief NRF24L01+ 2.4GHz Wireless Module Driver
 * @details Supports SPI hardware interface.
 */

#ifndef NRF24L01_H
#define NRF24L01_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"
#include <stdbool.h>

// NRF24L01 Register Definition
#define NRF24_REG_CONFIG      0x00
#define NRF24_REG_EN_AA       0x01
#define NRF24_REG_EN_RXADDR   0x02
#define NRF24_REG_SETUP_AW    0x03
#define NRF24_REG_SETUP_RETR  0x04
#define NRF24_REG_RF_CH       0x05
#define NRF24_REG_RF_SETUP    0x06
#define NRF24_REG_STATUS      0x07
#define NRF24_REG_OBSERVE_TX  0x08
#define NRF24_REG_RPD         0x09
#define NRF24_REG_RX_ADDR_P0  0x0A
#define NRF24_REG_RX_ADDR_P1  0x0B
#define NRF24_REG_RX_ADDR_P2  0x0C
#define NRF24_REG_RX_ADDR_P3  0x0D
#define NRF24_REG_RX_ADDR_P4  0x0E
#define NRF24_REG_RX_ADDR_P5  0x0F
#define NRF24_REG_TX_ADDR     0x10
#define NRF24_REG_RX_PW_P0    0x11
// ... (simplified, only essential)
#define NRF24_REG_FIFO_STATUS 0x17
#define NRF24_REG_DYNPD       0x1C
#define NRF24_REG_FEATURE     0x1D

// Commands
#define NRF24_CMD_R_REGISTER    0x00
#define NRF24_CMD_W_REGISTER    0x20
#define NRF24_CMD_R_RX_PAYLOAD  0x61
#define NRF24_CMD_W_TX_PAYLOAD  0xA0
#define NRF24_CMD_FLUSH_TX      0xE1
#define NRF24_CMD_FLUSH_RX      0xE2
#define NRF24_CMD_NOP           0xFF

typedef struct {
    SPI_HandleTypeDef *hspi;
    GPIO_TypeDef      *CSN_Port;
    uint16_t           CSN_Pin;
    GPIO_TypeDef      *CE_Port;
    uint16_t           CE_Pin;
    
    // Config
    uint8_t payload_size; // Default 32
    uint8_t channel;      // Default 40
} NRF24_Handle_t;

/**
 * @brief Initialize NRF24L01 Module
 * @return true if communication successful
 */
bool NRF24_Init(NRF24_Handle_t *handle, SPI_HandleTypeDef *hspi, 
                GPIO_TypeDef *csn_port, uint16_t csn_pin,
                GPIO_TypeDef *ce_port, uint16_t ce_pin);

/**
 * @brief Send Data (Blocking)
 * @return true if ACK received, false if Max Retries hit
 */
bool NRF24_Tx(NRF24_Handle_t *handle, const uint8_t *data, uint8_t len);

/**
 * @brief Check if data is available in RX FIFO
 */
bool NRF24_DataReady(NRF24_Handle_t *handle);

/**
 * @brief Read Data from RX FIFO
 */
void NRF24_Rx(NRF24_Handle_t *handle, uint8_t *data);

void NRF24_SetRxMode(NRF24_Handle_t *handle);
void NRF24_SetTxMode(NRF24_Handle_t *handle);

#ifdef __cplusplus
}
#endif

#endif // NRF24L01_H
