/**
 * @file nrf24l01.c
 * @brief NRF24L01+ Driver Implementation
 */

#include "nrf24l01.h"
#include "delay.h" // Requires delay_us

#ifdef HAL_SPI_MODULE_ENABLED

// Address width 5 bytes
#define TX_ADR_WIDTH 5
#define RX_ADR_WIDTH 5

// Defaults
static const uint8_t TX_ADDRESS[TX_ADR_WIDTH] = {0x34,0x43,0x10,0x10,0x01}; // Local Address
static const uint8_t RX_ADDRESS[RX_ADR_WIDTH] = {0x34,0x43,0x10,0x10,0x01}; // Destination Address

#define CE_Low(h)   HAL_GPIO_WritePin(h->CE_Port, h->CE_Pin, GPIO_PIN_RESET)
#define CE_High(h)  HAL_GPIO_WritePin(h->CE_Port, h->CE_Pin, GPIO_PIN_SET)
#define CSN_Low(h)  HAL_GPIO_WritePin(h->CSN_Port, h->CSN_Pin, GPIO_PIN_RESET)
#define CSN_High(h) HAL_GPIO_WritePin(h->CSN_Port, h->CSN_Pin, GPIO_PIN_SET)

static uint8_t SPI_RW(NRF24_Handle_t *h, uint8_t byte) {
    uint8_t rx_byte = 0;
    HAL_SPI_TransmitReceive(h->hspi, &byte, &rx_byte, 1, 100);
    return rx_byte;
}

static uint8_t Write_Reg(NRF24_Handle_t *h, uint8_t reg, uint8_t value) {
    uint8_t status;
    CSN_Low(h);
    status = SPI_RW(h, NRF24_CMD_W_REGISTER | (reg & 0x1F));
    SPI_RW(h, value);
    CSN_High(h);
    return status;
}

static uint8_t Read_Reg(NRF24_Handle_t *h, uint8_t reg) {
    uint8_t value;
    CSN_Low(h);
    SPI_RW(h, NRF24_CMD_R_REGISTER | (reg & 0x1F));
    value = SPI_RW(h, 0xFF); // Dummy
    CSN_High(h);
    return value;
}

static void Write_Buf(NRF24_Handle_t *h, uint8_t reg, const uint8_t *pBuf, uint8_t len) {
    CSN_Low(h);
    SPI_RW(h, NRF24_CMD_W_REGISTER | (reg & 0x1F));
    // HAL_SPI_Transmit(h->hspi, (uint8_t*)pBuf, len, 100); 
    // Manual loop usually safer for short lengths to avoid overhead
    for(uint8_t i=0; i<len; i++) SPI_RW(h, pBuf[i]);
    CSN_High(h);
}

static void Read_Buf(NRF24_Handle_t *h, uint8_t reg, uint8_t *pBuf, uint8_t len) {
    CSN_Low(h);
    SPI_RW(h, NRF24_CMD_R_REGISTER | (reg & 0x1F));
    for(uint8_t i=0; i<len; i++) pBuf[i] = SPI_RW(h, 0xFF);
    CSN_High(h);
}

bool NRF24_Init(NRF24_Handle_t *h, SPI_HandleTypeDef *hspi, 
                GPIO_TypeDef *csn_port, uint16_t csn_pin,
                GPIO_TypeDef *ce_port, uint16_t ce_pin) 
{
    h->hspi = hspi;
    h->CSN_Port = csn_port;
    h->CSN_Pin = csn_pin;
    h->CE_Port = ce_port;
    h->CE_Pin = ce_pin;
    h->payload_size = 32;
    h->channel = 40;

    // CSN High normally
    CSN_High(h);
    // CE Low to Standby
    CE_Low(h);
    
    // Check presence
    Write_Reg(h, NRF24_REG_TX_ADDR, 0xAA);
    uint8_t check = Read_Reg(h, NRF24_REG_TX_ADDR);
    if (check != 0xAA) return false; // Chip not responding?

    // Common Config
    Write_Reg(h, NRF24_REG_EN_AA, 0x01);      // Enable Auto Ack on Pipe0
    Write_Reg(h, NRF24_REG_EN_RXADDR, 0x01);  // Enable RX Pipe0
    Write_Reg(h, NRF24_REG_SETUP_RETR, 0x1A); // 500us retry, 10 retries
    Write_Reg(h, NRF24_REG_RF_CH, h->channel);
    Write_Reg(h, NRF24_REG_RF_SETUP, 0x0F);   // 0dbm, 2Mbps
    Write_Reg(h, NRF24_REG_RX_PW_P0, h->payload_size);
    
    return true;
}

void NRF24_SetRxMode(NRF24_Handle_t *h) {
    CE_Low(h);
    Write_Buf(h, NRF24_REG_RX_ADDR_P0, RX_ADDRESS, RX_ADR_WIDTH);
    
    // PRIM_RX = 1, PWR_UP = 1, CRC EN
    Write_Reg(h, NRF24_REG_CONFIG, 0x0F); 
    
    CE_High(h); // Enable RX
}

void NRF24_SetTxMode(NRF24_Handle_t *h) {
    CE_Low(h);
    Write_Buf(h, NRF24_REG_TX_ADDR, TX_ADDRESS, TX_ADR_WIDTH);
    Write_Buf(h, NRF24_REG_RX_ADDR_P0, TX_ADDRESS, TX_ADR_WIDTH); // For Auto-Ack
    
    // PRIM_RX = 0, PWR_UP = 1, CRC EN
    Write_Reg(h, NRF24_REG_CONFIG, 0x0E);
    
    // CE pulsing handled in Tx
}

bool NRF24_Tx(NRF24_Handle_t *h, const uint8_t *data, uint8_t len) {
    NRF24_SetTxMode(h);
    
    CSN_Low(h);
    SPI_RW(h, NRF24_CMD_W_TX_PAYLOAD);
    for(uint8_t i=0; i<len; i++) SPI_RW(h, data[i]);
    CSN_High(h);
    
    CE_High(h);
    Delay_us(15); // Pulse min 10us
    CE_Low(h);
    
    // Wait for Send
    uint32_t start = HAL_GetTick();
    uint8_t status;
    while( (HAL_GetTick() - start) < 100 ) { // 100ms timeout
        status = Read_Reg(h, NRF24_REG_STATUS);
        if (status & 0x20) { // TX_DS (Success)
            Write_Reg(h, NRF24_REG_STATUS, 0x20); // Clear
            return true;
        }
        if (status & 0x10) { // MAX_RT (Max Retries)
            Write_Reg(h, NRF24_REG_STATUS, 0x10); // Clear
            Write_Reg(h, NRF24_CMD_FLUSH_TX, 0);
            return false;
        }
    }
    return false;
}

bool NRF24_DataReady(NRF24_Handle_t *h) {
    uint8_t status = Read_Reg(h, NRF24_REG_STATUS);
    if (status & 0x40) { // RX_DR
        return true;
    }
    return false;
}

void NRF24_Rx(NRF24_Handle_t *h, uint8_t *data) {
    CSN_Low(h);
    SPI_RW(h, NRF24_CMD_R_RX_PAYLOAD);
    for(uint8_t i=0; i<h->payload_size; i++) data[i] = SPI_RW(h, 0xFF);
    CSN_High(h);
    
    Write_Reg(h, NRF24_REG_STATUS, 0x40); // Clear RX_DR
}

#else
// Stubs
bool NRF24_Init(NRF24_Handle_t *h, SPI_HandleTypeDef *hspi, GPIO_TypeDef *p1, uint16_t x1, GPIO_TypeDef *p2, uint16_t x2) { return false; }
bool NRF24_Tx(NRF24_Handle_t *h, const uint8_t *data, uint8_t len) { return false; }
bool NRF24_DataReady(NRF24_Handle_t *h) { return false; }
void NRF24_Rx(NRF24_Handle_t *h, uint8_t *data) {}
void NRF24_SetRxMode(NRF24_Handle_t *h) {}
void NRF24_SetTxMode(NRF24_Handle_t *h) {}
#endif
