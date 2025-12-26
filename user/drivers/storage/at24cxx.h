/**
 * @file at24cxx.h
 * @brief AT24Cxx I2C EEPROM Driver Header File
 * @author Standard Implementation
 * @date 2024
 */

#ifndef __AT24CXX_H
#define __AT24CXX_H

#include "main.h"

#ifndef __STM32F1xx_HAL_I2C_H
#include "main.h"
#endif

/* AT24Cxx Model Definitions */
#define AT24C01		127
#define AT24C02		255
#define AT24C04		511
#define AT24C08		1023
#define AT24C16		2047
#define AT24C32		4095
#define AT24C64		8191
#define AT24C128	16383
#define AT24C256	32767
#define AT24C512	65535

/* Default I2C Address (A0-A2 grounded) */
#define AT24CXX_I2C_ADDR  0xA0

typedef struct {
    I2C_HandleTypeDef *hi2c;
    uint16_t          PageSize;
    uint32_t          Capacity; // In bytes
    uint8_t           I2C_Address;
    uint8_t           AddressByteWidth; // 1 (8-bit) or 2 (16-bit)
} AT24CXX_HandleTypeDef;

/* Function Prototypes */
/**
 * @brief Initialize the AT24Cxx EEPROM driver
 * @param hat24 Handle to the AT24Cxx structure
 * @param hi2c Pointer to the I2C handle
 * @param type AT24Cxx model (e.g., AT24C02, AT24C256)
 * @param address I2C address (default 0xA0)
 * @return 0 on success, 1 on error
 */
uint8_t AT24CXX_Init(AT24CXX_HandleTypeDef *hat24, I2C_HandleTypeDef *hi2c, uint32_t type, uint8_t address);

/**
 * @brief Check if device is connected
 * @param hat24 Handle to the AT24Cxx structure
 * @return 0 on success (connected), 1 on error
 */
uint8_t AT24CXX_Check(AT24CXX_HandleTypeDef *hat24);

/**
 * @brief Read data from EEPROM
 * @param hat24 Handle to the AT24Cxx structure
 * @param ReadAddr Address to read from
 * @param pBuffer Pointer to buffer to store data
 * @param NumByteToRead Number of bytes to read
 */
void AT24CXX_Read(AT24CXX_HandleTypeDef *hat24, uint32_t ReadAddr, uint8_t *pBuffer, uint16_t NumByteToRead);

/**
 * @brief Write data to EEPROM (Handles page write automatically)
 * @param hat24 Handle to the AT24Cxx structure
 * @param WriteAddr Address to write to
 * @param pBuffer Pointer to data buffer
 * @param NumByteToWrite Number of bytes to write
 */
void AT24CXX_Write(AT24CXX_HandleTypeDef *hat24, uint32_t WriteAddr, uint8_t *pBuffer, uint16_t NumByteToWrite);

/**
 * @brief Erase the entire EEPROM (Fill with 0xFF)
 * @param hat24 Handle to the AT24Cxx structure
 */
void AT24CXX_Erase_Chip(AT24CXX_HandleTypeDef *hat24);

#endif // __AT24CXX_H
