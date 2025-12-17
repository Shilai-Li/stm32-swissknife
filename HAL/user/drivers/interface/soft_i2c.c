/**
 * @file soft_i2c.c
 * @brief Software I2C (Bit-Banging) Driver Implementation
 * @author Standard Implementation
 * @date 2024
 */

#include "drivers/soft_i2c.h"

// Constants
#define I2C_READ    1
#define I2C_WRITE   0
#define I2C_ACK     0
#define I2C_NACK    1

#define I2C_MEMADD_SIZE_8BIT    0x00000001U
#define I2C_MEMADD_SIZE_16BIT   0x00000010U

// --- Private Low Level Functions ---

// Simple microsecond delay approximation
// For 72MHz, loop logic overhead + instruction takes cycles.
// Tuned for ~100-400kHz. 
// volatile to prevent optimization
static void Soft_I2C_Delay(Soft_I2C_HandleTypeDef *hi2c) {
    volatile uint32_t i = hi2c->DelayTicks;
    while(i--);
}

static void Soft_I2C_SDA_High(Soft_I2C_HandleTypeDef *hi2c) {
    HAL_GPIO_WritePin(hi2c->SdaPort, hi2c->SdaPin, GPIO_PIN_SET);
}

static void Soft_I2C_SDA_Low(Soft_I2C_HandleTypeDef *hi2c) {
    HAL_GPIO_WritePin(hi2c->SdaPort, hi2c->SdaPin, GPIO_PIN_RESET);
}

static void Soft_I2C_SCL_High(Soft_I2C_HandleTypeDef *hi2c) {
    HAL_GPIO_WritePin(hi2c->SclPort, hi2c->SclPin, GPIO_PIN_SET);
}

static void Soft_I2C_SCL_Low(Soft_I2C_HandleTypeDef *hi2c) {
    HAL_GPIO_WritePin(hi2c->SclPort, hi2c->SclPin, GPIO_PIN_RESET);
}

static uint8_t Soft_I2C_SDA_Read(Soft_I2C_HandleTypeDef *hi2c) {
    return (uint8_t)HAL_GPIO_ReadPin(hi2c->SdaPort, hi2c->SdaPin);
}

static void Soft_I2C_Start(Soft_I2C_HandleTypeDef *hi2c) {
    Soft_I2C_SDA_High(hi2c);
    Soft_I2C_SCL_High(hi2c);
    Soft_I2C_Delay(hi2c);
    Soft_I2C_SDA_Low(hi2c);
    Soft_I2C_Delay(hi2c);
    Soft_I2C_SCL_Low(hi2c);
}

static void Soft_I2C_Stop(Soft_I2C_HandleTypeDef *hi2c) {
    Soft_I2C_SDA_Low(hi2c);
    Soft_I2C_Delay(hi2c);
    Soft_I2C_SCL_High(hi2c);
    Soft_I2C_Delay(hi2c);
    Soft_I2C_SDA_High(hi2c);
    Soft_I2C_Delay(hi2c);
}

static void Soft_I2C_SendByte(Soft_I2C_HandleTypeDef *hi2c, uint8_t byte) {
    for(uint8_t i=0; i<8; i++) {
        if(byte & 0x80) Soft_I2C_SDA_High(hi2c);
        else            Soft_I2C_SDA_Low(hi2c);
        byte <<= 1;
        Soft_I2C_Delay(hi2c);
        
        Soft_I2C_SCL_High(hi2c);
        Soft_I2C_Delay(hi2c);
        Soft_I2C_SCL_Low(hi2c);
        Soft_I2C_Delay(hi2c);
    }
}

static uint8_t Soft_I2C_ReadByte(Soft_I2C_HandleTypeDef *hi2c, uint8_t ack) {
    uint8_t byte = 0;
    
    // SDA Input Mode (Assuming Open-Drain config, simply writing 1 lets external pull-up work, 
    // BUT HAL_GPIO_WritePin only sets ODR. 
    // Standard STM32 I2C GPIO config is: Output Open-Drain. 
    // Writing 1 to Output sets pin to Hi-Z (High), allowing reading from IDR.
    // So we just set High before reading.
    Soft_I2C_SDA_High(hi2c); 
    
    for(uint8_t i=0; i<8; i++) {
        byte <<= 1;
        Soft_I2C_SCL_High(hi2c);
        Soft_I2C_Delay(hi2c);
        if(Soft_I2C_SDA_Read(hi2c)) byte |= 0x01;
        Soft_I2C_SCL_Low(hi2c);
        Soft_I2C_Delay(hi2c);
    }
    
    // Send ACK/NACK
    if(ack == I2C_ACK) Soft_I2C_SDA_Low(hi2c);
    else               Soft_I2C_SDA_High(hi2c);
    
    Soft_I2C_Delay(hi2c);
    Soft_I2C_SCL_High(hi2c);
    Soft_I2C_Delay(hi2c);
    Soft_I2C_SCL_Low(hi2c);
    Soft_I2C_Delay(hi2c);
    
    return byte;
}

static uint8_t Soft_I2C_WaitAck(Soft_I2C_HandleTypeDef *hi2c) {
    Soft_I2C_SDA_High(hi2c); // Release SDA
    Soft_I2C_Delay(hi2c);
    Soft_I2C_SCL_High(hi2c);
    Soft_I2C_Delay(hi2c);
    
    uint32_t to = 1000;
    while(Soft_I2C_SDA_Read(hi2c)) {
        to--;
        if(to == 0) {
            Soft_I2C_Stop(hi2c);
            return 1; // Timed out / NACK
        }
    }
    
    Soft_I2C_SCL_Low(hi2c);
    return 0; // ACK received
}

// --- Public Functions ---

void Soft_I2C_Init(Soft_I2C_HandleTypeDef *hi2c, 
                  GPIO_TypeDef *scl_port, uint16_t scl_pin,
                  GPIO_TypeDef *sda_port, uint16_t sda_pin)
{
    hi2c->SclPort = scl_port; hi2c->SclPin = scl_pin;
    hi2c->SdaPort = sda_port; hi2c->SdaPin = sda_pin;
    hi2c->DelayTicks = 10; // Tune this for speed (e.g. 5-50)
    
    // We assume GPIOs are already initialized as Output Open-Drain by CubeMX/User
    // Default state: High
    Soft_I2C_SDA_High(hi2c);
    Soft_I2C_SCL_High(hi2c);
}

uint8_t Soft_I2C_IsDeviceReady(Soft_I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint32_t Trials, uint32_t Timeout) {
    // Simple check: Send Start + Addr + Wait Ack
    for(uint32_t i=0; i<Trials; i++) {
        Soft_I2C_Start(hi2c);
        Soft_I2C_SendByte(hi2c, DevAddress & 0xFE); // Write mode
        if(Soft_I2C_WaitAck(hi2c) == 0) {
            Soft_I2C_Stop(hi2c);
            return 0; // Ready
        }
        Soft_I2C_Stop(hi2c); // Redundant stop if NACKed inside WaitAck but safe
    }
    return 1; // Not Ready
}

uint8_t Soft_I2C_Master_Transmit(Soft_I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout) {
    Soft_I2C_Start(hi2c);
    
    Soft_I2C_SendByte(hi2c, DevAddress & 0xFE); // Write
    if(Soft_I2C_WaitAck(hi2c)) return 1;
    
    for(uint16_t i=0; i<Size; i++) {
        Soft_I2C_SendByte(hi2c, pData[i]);
        if(Soft_I2C_WaitAck(hi2c)) return 1;
    }
    
    Soft_I2C_Stop(hi2c);
    return 0;
}

uint8_t Soft_I2C_Master_Receive(Soft_I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout) {
    Soft_I2C_Start(hi2c);
    
    Soft_I2C_SendByte(hi2c, (DevAddress & 0xFE) | 0x01); // Read
    if(Soft_I2C_WaitAck(hi2c)) return 1;
    
    for(uint16_t i=0; i<Size; i++) {
        // Send ACK for all bytes except last, last gets NACK
        pData[i] = Soft_I2C_ReadByte(hi2c, (i == Size - 1) ? I2C_NACK : I2C_ACK);
    }
    
    Soft_I2C_Stop(hi2c);
    return 0;
}

uint8_t Soft_I2C_Mem_Write(Soft_I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout) {
    Soft_I2C_Start(hi2c);
    
    Soft_I2C_SendByte(hi2c, DevAddress & 0xFE);
    if(Soft_I2C_WaitAck(hi2c)) return 1;
    
    // Send Memory Address
    if(MemAddSize == I2C_MEMADD_SIZE_16BIT) {
        Soft_I2C_SendByte(hi2c, (MemAddress >> 8) & 0xFF);
        if(Soft_I2C_WaitAck(hi2c)) return 1;
        Soft_I2C_SendByte(hi2c, MemAddress & 0xFF);
        if(Soft_I2C_WaitAck(hi2c)) return 1;
    } else {
        Soft_I2C_SendByte(hi2c, MemAddress & 0xFF);
        if(Soft_I2C_WaitAck(hi2c)) return 1;
    }
    
    // Send Data
    for(uint16_t i=0; i<Size; i++) {
        Soft_I2C_SendByte(hi2c, pData[i]);
        if(Soft_I2C_WaitAck(hi2c)) return 1;
    }
    
    Soft_I2C_Stop(hi2c);
    return 0;
}

uint8_t Soft_I2C_Mem_Read(Soft_I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout) {
    // 1. Dummy Write for Address
    Soft_I2C_Start(hi2c);
    
    Soft_I2C_SendByte(hi2c, DevAddress & 0xFE);
    if(Soft_I2C_WaitAck(hi2c)) return 1;
    
    if(MemAddSize == I2C_MEMADD_SIZE_16BIT) {
        Soft_I2C_SendByte(hi2c, (MemAddress >> 8) & 0xFF);
        if(Soft_I2C_WaitAck(hi2c)) return 1;
        Soft_I2C_SendByte(hi2c, MemAddress & 0xFF);
        if(Soft_I2C_WaitAck(hi2c)) return 1;
    } else {
        Soft_I2C_SendByte(hi2c, MemAddress & 0xFF);
        if(Soft_I2C_WaitAck(hi2c)) return 1;
    }
    
    // 2. Restart and Read
    Soft_I2C_Start(hi2c); // Repeated Start
    
    Soft_I2C_SendByte(hi2c, (DevAddress & 0xFE) | 0x01); // Read
    if(Soft_I2C_WaitAck(hi2c)) return 1;
    
    for(uint16_t i=0; i<Size; i++) {
        pData[i] = Soft_I2C_ReadByte(hi2c, (i == Size - 1) ? I2C_NACK : I2C_ACK);
    }
    
    Soft_I2C_Stop(hi2c);
    return 0;
}
