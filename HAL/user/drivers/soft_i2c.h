/**
 * @file soft_i2c.h
 * @brief Software I2C (Bit-Banging) Driver Header File
 * @author Standard Implementation
 * @date 2024
 */

#ifndef __SOFT_I2C_H
#define __SOFT_I2C_H

#include "main.h"

typedef struct {
    GPIO_TypeDef *SclPort;
    uint16_t      SclPin;
    GPIO_TypeDef *SdaPort;
    uint16_t      SdaPin;
    uint32_t      DelayTicks; // Approximate delay for speed control
} Soft_I2C_HandleTypeDef;

/* Function Prototypes */

/**
 * @brief Initialize Software I2C
 * @param hi2c Handle
 * @param scl_port SCL GPIO Port
 * @param scl_pin  SCL GPIO Pin
 * @param sda_port SDA GPIO Port
 * @param sda_pin  SDA GPIO Pin
 */
void Soft_I2C_Init(Soft_I2C_HandleTypeDef *hi2c, 
                  GPIO_TypeDef *scl_port, uint16_t scl_pin,
                  GPIO_TypeDef *sda_port, uint16_t sda_pin);

/**
 * @brief Check if device is ready
 * @param DevAddress 7-bit Device Address (shifted left by 1)
 */
uint8_t Soft_I2C_IsDeviceReady(Soft_I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint32_t Trials, uint32_t Timeout);

/**
 * @brief Master Transmit
 */
uint8_t Soft_I2C_Master_Transmit(Soft_I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout);

/**
 * @brief Master Receive
 */
uint8_t Soft_I2C_Master_Receive(Soft_I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout);

/**
 * @brief Write Memory (Register)
 */
uint8_t Soft_I2C_Mem_Write(Soft_I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout);

/**
 * @brief Read Memory (Register)
 */
uint8_t Soft_I2C_Mem_Read(Soft_I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout);

#endif // __SOFT_I2C_H
