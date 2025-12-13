/**
 * @file at24cxx.c
 * @brief AT24Cxx I2C EEPROM Driver Implementation
 * @author Standard Implementation
 * @date 2024
 */

#include "drivers/at24cxx.h"
#include <string.h>

/**
 * @brief Initialize the AT24Cxx EEPROM driver
 */
uint8_t AT24CXX_Init(AT24CXX_HandleTypeDef *hat24, I2C_HandleTypeDef *hi2c, uint32_t type, uint8_t address) {
    hat24->hi2c = hi2c;
    hat24->I2C_Address = address;
    hat24->Capacity = type + 1;
    
    // Setup Page Size and Address Width defaults
    if (type <= AT24C02) {
        hat24->PageSize = 8;
        hat24->AddressByteWidth = 1;
    } else if (type <= AT24C16) {
        hat24->PageSize = 16;
        hat24->AddressByteWidth = 1;
    } else {
        // AT24C32 to AT24C512
        if (type <= AT24C64) hat24->PageSize = 32;
        else if (type <= AT24C256) hat24->PageSize = 64;
        else hat24->PageSize = 128;
        
        hat24->AddressByteWidth = 2;
    }
    
    return AT24CXX_Check(hat24);
}

/**
 * @brief Check if device is connected and ready
 */
uint8_t AT24CXX_Check(AT24CXX_HandleTypeDef *hat24) {
    return HAL_I2C_IsDeviceReady(hat24->hi2c, hat24->I2C_Address, 10, 100);
}

/**
 * @brief Write data to EEPROM with page management
 */
void AT24CXX_Write(AT24CXX_HandleTypeDef *hat24, uint32_t WriteAddr, uint8_t *pBuffer, uint16_t NumByteToWrite) {
    uint16_t pageremain;
    uint8_t devAddr;
    uint16_t memAddr;
    uint32_t currentWriteAddr = WriteAddr;
    uint8_t *currentBuf = pBuffer;
    uint16_t currentLen = NumByteToWrite;

    while (currentLen > 0) {
        pageremain = hat24->PageSize - (currentWriteAddr % hat24->PageSize);
        if (currentLen <= pageremain) {
            pageremain = currentLen;
        }

        // Calculate Device Address and Memory Address based on chip type
        if (hat24->AddressByteWidth == 1) {
            // For C01-C16, use Block Select bits in DevAddress
            // Block is calculated from upper bits of address
            // C04: A8 (1 bit), C08: A9-A8 (2 bits), C16: A10-A8 (3 bits)
            // Effectively: (currentWriteAddr / 256) << 1
            devAddr = hat24->I2C_Address | (uint8_t)((currentWriteAddr / 256) << 1);
            memAddr = (uint8_t)(currentWriteAddr % 256);
            
            HAL_I2C_Mem_Write(hat24->hi2c, devAddr, memAddr, I2C_MEMADD_SIZE_8BIT, currentBuf, pageremain, 1000);
        } else {
            // For C32+, standard 16-bit address
            devAddr = hat24->I2C_Address;
            memAddr = (uint16_t)currentWriteAddr;
            
            HAL_I2C_Mem_Write(hat24->hi2c, devAddr, memAddr, I2C_MEMADD_SIZE_16BIT, currentBuf, pageremain, 1000);
        }
        
        // Wait for internal write cycle to finish
        HAL_I2C_IsDeviceReady(hat24->hi2c, devAddr, 10, 1000); // 5ms typical, polling covers it

        currentWriteAddr += pageremain;
        currentBuf += pageremain;
        currentLen -= pageremain;
    }
}

/**
 * @brief Read data from EEPROM
 */
void AT24CXX_Read(AT24CXX_HandleTypeDef *hat24, uint32_t ReadAddr, uint8_t *pBuffer, uint16_t NumByteToRead) {
    // For small chips (C01-C16), we might need to handle block boundaries depending on behavior.
    // However, sequential read usually wraps within the block or device.
    // To be safe and consistent with Write logic: split if crossing 256-byte blocks for small chips.
    // For large chips, just read.
    
    if (hat24->AddressByteWidth == 2) {
        HAL_I2C_Mem_Read(hat24->hi2c, hat24->I2C_Address, ReadAddr, I2C_MEMADD_SIZE_16BIT, pBuffer, NumByteToRead, 1000);
    } else {
        // Small chips mechanism
        uint32_t currentReadAddr = ReadAddr;
        uint8_t *currentBuf = pBuffer;
        uint16_t currentLen = NumByteToRead;
        uint16_t blockremain;
        uint8_t devAddr;
        uint8_t memAddr;

        while (currentLen > 0) {
            // Block size is 256 bytes for addressing context
             blockremain = 256 - (currentReadAddr % 256);
            if (currentLen <= blockremain) {
                blockremain = currentLen;
            }

            devAddr = hat24->I2C_Address | (uint8_t)((currentReadAddr / 256) << 1);
            memAddr = (uint8_t)(currentReadAddr % 256);

            HAL_I2C_Mem_Read(hat24->hi2c, devAddr, memAddr, I2C_MEMADD_SIZE_8BIT, currentBuf, blockremain, 1000);

            currentReadAddr += blockremain;
            currentBuf += blockremain;
            currentLen -= blockremain;
        }
    }
}

/**
 * @brief Erase Chip (Fill with 0xFF)
 */
void AT24CXX_Erase_Chip(AT24CXX_HandleTypeDef *hat24) {
    uint32_t addr;
    uint8_t data[128]; // Buffer for faster writing
    uint32_t chunk;
    
    memset(data, 0xFF, sizeof(data));
    
    // Choose chunk size based on page size to avoid read-modify-write if we were doing random writes, but here we overwrite.
    // Safest is to write page by page or just use our Write function which handles pages.
    // Using Write function efficiently:
    
    for (addr = 0; addr < hat24->Capacity; addr += sizeof(data)) {
        chunk = (hat24->Capacity - addr < sizeof(data)) ? (hat24->Capacity - addr) : sizeof(data);
        AT24CXX_Write(hat24, addr, data, (uint16_t)chunk);
    }
}
