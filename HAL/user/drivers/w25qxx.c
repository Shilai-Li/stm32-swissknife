/**
 * @file w25qxx.c
 * @brief W25Qxx SPI Flash Driver Source File
 * @author Standard Implementation
 * @date 2024
 */

#include "w25qxx.h"
#include <stdio.h> // For printf debugging if needed, usually not used in pure driver

#define W25QXX_DUMMY_BYTE 0xA5

// Helper functions for CS control
static void W25QXX_CS_Low(W25QXX_HandleTypeDef *hflash) {
    HAL_GPIO_WritePin(hflash->CsPort, hflash->CsPin, GPIO_PIN_RESET);
}

static void W25QXX_CS_High(W25QXX_HandleTypeDef *hflash) {
    HAL_GPIO_WritePin(hflash->CsPort, hflash->CsPin, GPIO_PIN_SET);
}

// Low level SPI exchange
static uint8_t W25QXX_Spi(W25QXX_HandleTypeDef *hflash, uint8_t Data) {
    uint8_t ret;
    HAL_SPI_TransmitReceive(hflash->hspi, &Data, &ret, 1, 100);
    return ret;
}

// Enable write operations
static void W25QXX_WriteEnable(W25QXX_HandleTypeDef *hflash) {
    W25QXX_CS_Low(hflash);
    W25QXX_Spi(hflash, W25QXX_WRITE_ENABLE);
    W25QXX_CS_High(hflash);
}

// Wait for write/erase to complete
static void W25QXX_WaitForWriteEnd(W25QXX_HandleTypeDef *hflash) {
    uint8_t Status = 0;
    
    W25QXX_CS_Low(hflash);
    W25QXX_Spi(hflash, W25QXX_READ_STATUS_REG1);
    
    do {
        Status = W25QXX_Spi(hflash, W25QXX_DUMMY_BYTE);
    } while ((Status & 0x01) == 0x01); // Wait while BUSY bit is set
    
    W25QXX_CS_High(hflash);
}

uint8_t W25QXX_Init(W25QXX_HandleTypeDef *hflash, SPI_HandleTypeDef *hspi, GPIO_TypeDef *cs_port, uint16_t cs_pin) {
    hflash->hspi = hspi;
    hflash->CsPort = cs_port;
    hflash->CsPin = cs_pin;
    
    W25QXX_CS_High(hflash);
    HAL_Delay(100); // Wait for stabilization on power up
    
    uint32_t id = W25QXX_ReadID(hflash);
    
    // Identify chip and fill parameters
    // Identify chip and fill parameters using JEDEC ID (Memory Type + Capacity)
    switch(id & 0xFFFF) {
        case W25Q256:
            hflash->Info.ID = W25Q256;
            hflash->Info.BlockCount = 512;
            hflash->Info.CapacityInKiloByte = 32768;
            break;
        case W25Q128:
            hflash->Info.ID = W25Q128;
            hflash->Info.BlockCount = 256;
            hflash->Info.CapacityInKiloByte = 16384;
            break;
        case W25Q64:
            hflash->Info.ID = W25Q64;
            hflash->Info.BlockCount = 128;
            hflash->Info.CapacityInKiloByte = 8192;
            break;
        case W25Q32:
            hflash->Info.ID = W25Q32;
            hflash->Info.BlockCount = 64;
            hflash->Info.CapacityInKiloByte = 4096;
            break;
        case W25Q16:
            hflash->Info.ID = W25Q16;
            hflash->Info.BlockCount = 32;
            hflash->Info.CapacityInKiloByte = 2048;
            break;
        case W25Q80:
            hflash->Info.ID = W25Q80;
            hflash->Info.BlockCount = 16;
            hflash->Info.CapacityInKiloByte = 1024;
            break;
        default:
            hflash->Info.ID = 0; // Unknown
            return 0; // Initialization failed
    }
    
    hflash->Info.PageSize = 256;
    hflash->Info.SectorSize = 4096;
    hflash->Info.SectorCount = hflash->Info.BlockCount * 16;
    hflash->Info.PageCount = (hflash->Info.SectorCount * hflash->Info.SectorSize) / hflash->Info.PageSize;
    hflash->Info.BlockSize = hflash->Info.SectorSize * 16;
    
    return 1; // Success
}

uint32_t W25QXX_ReadID(W25QXX_HandleTypeDef *hflash) {
    uint32_t Temp = 0, Temp0 = 0, Temp1 = 0, Temp2 = 0;
    
    W25QXX_CS_Low(hflash);
    W25QXX_Spi(hflash, W25QXX_JEDEC_ID);
    Temp0 = W25QXX_Spi(hflash, W25QXX_DUMMY_BYTE);
    Temp1 = W25QXX_Spi(hflash, W25QXX_DUMMY_BYTE);
    Temp2 = W25QXX_Spi(hflash, W25QXX_DUMMY_BYTE);
    W25QXX_CS_High(hflash);
    
    Temp = (Temp0 << 16) | (Temp1 << 8) | Temp2;
    return Temp;
}

void W25QXX_Read(W25QXX_HandleTypeDef *hflash, uint8_t* pBuffer, uint32_t ReadAddr, uint32_t NumByteToRead) {
    W25QXX_CS_Low(hflash);
    W25QXX_Spi(hflash, W25QXX_READ_DATA);
    
    // Address (24-bit)
    W25QXX_Spi(hflash, (ReadAddr & 0xFF0000) >> 16);
    W25QXX_Spi(hflash, (ReadAddr & 0xFF00) >> 8);
    W25QXX_Spi(hflash, ReadAddr & 0xFF);
    
    // Read data directly
    HAL_SPI_Receive(hflash->hspi, pBuffer, NumByteToRead, 2000);
    
    W25QXX_CS_High(hflash);
}

void W25QXX_Write_Page(W25QXX_HandleTypeDef *hflash, uint8_t* pBuffer, uint32_t WriteAddr, uint32_t NumByteToWrite) {
    W25QXX_WriteEnable(hflash);
    
    W25QXX_CS_Low(hflash);
    W25QXX_Spi(hflash, W25QXX_PAGE_PROGRAM);
    W25QXX_Spi(hflash, (WriteAddr & 0xFF0000) >> 16);
    W25QXX_Spi(hflash, (WriteAddr & 0xFF00) >> 8);
    W25QXX_Spi(hflash, WriteAddr & 0xFF);
    
    // Using HAL_SPI_Transmit is faster for larger blocks than a loop of single byte transfers
    HAL_SPI_Transmit(hflash->hspi, pBuffer, NumByteToWrite, 100);
    
    W25QXX_CS_High(hflash);
    W25QXX_WaitForWriteEnd(hflash);
}

void W25QXX_Write(W25QXX_HandleTypeDef *hflash, uint8_t* pBuffer, uint32_t WriteAddr, uint32_t NumByteToWrite) {
    uint32_t pageremain;
    
    // Calculate space remaining in current page
    pageremain = 256 - (WriteAddr % 256); 
    
    if (NumByteToWrite <= pageremain) {
        pageremain = NumByteToWrite;
    }

    while (NumByteToWrite > 0) {
        W25QXX_Write_Page(hflash, pBuffer, WriteAddr, pageremain);
        
        // Update pointers and counters
        if (NumByteToWrite == pageremain) {
            NumByteToWrite = 0;
        } else {
            pBuffer += pageremain;
            WriteAddr += pageremain;
            NumByteToWrite -= pageremain; 
            
            // Next write will be a full page or the remainder
            pageremain = (NumByteToWrite > 256) ? 256 : NumByteToWrite;
        }
    }
}

void W25QXX_Erase_Sector(W25QXX_HandleTypeDef *hflash, uint32_t Address) {
    W25QXX_WaitForWriteEnd(hflash);
    W25QXX_WriteEnable(hflash);
    
    W25QXX_CS_Low(hflash);
    W25QXX_Spi(hflash, W25QXX_SECTOR_ERASE);
    W25QXX_Spi(hflash, (Address & 0xFF0000) >> 16);
    W25QXX_Spi(hflash, (Address & 0xFF00) >> 8);
    W25QXX_Spi(hflash, Address & 0xFF);
    W25QXX_CS_High(hflash);
    
    W25QXX_WaitForWriteEnd(hflash);
}

void W25QXX_Erase_Block(W25QXX_HandleTypeDef *hflash, uint32_t Address) {
    W25QXX_WaitForWriteEnd(hflash);
    W25QXX_WriteEnable(hflash);
    
    W25QXX_CS_Low(hflash);
    W25QXX_Spi(hflash, W25QXX_BLOCK_ERASE_64K);
    W25QXX_Spi(hflash, (Address & 0xFF0000) >> 16);
    W25QXX_Spi(hflash, (Address & 0xFF00) >> 8);
    W25QXX_Spi(hflash, Address & 0xFF);
    W25QXX_CS_High(hflash);
    
    W25QXX_WaitForWriteEnd(hflash);
}

void W25QXX_Erase_Chip(W25QXX_HandleTypeDef *hflash) {
    W25QXX_WaitForWriteEnd(hflash);
    W25QXX_WriteEnable(hflash);
    
    W25QXX_CS_Low(hflash);
    W25QXX_Spi(hflash, W25QXX_CHIP_ERASE);
    W25QXX_CS_High(hflash);
    
    W25QXX_WaitForWriteEnd(hflash);
}

uint8_t W25QXX_IsEmpty_Sector(W25QXX_HandleTypeDef *hflash, uint32_t Sector_Address, uint32_t Offset_In_Byte, uint32_t NumByteToCheck_up_to_SectorSize) {
    // Implementation can simply read and check for 0xFF
    // For brevity, basic reading logic
    uint8_t pBuffer[256]; // Read in chunks
    uint32_t workAddress = Sector_Address + Offset_In_Byte;
    uint32_t remaining = NumByteToCheck_up_to_SectorSize;
    if (remaining > hflash->Info.SectorSize) remaining = hflash->Info.SectorSize;
    
    while (remaining > 0) {
        uint32_t toRead = (remaining > 256) ? 256 : remaining;
        W25QXX_Read(hflash, pBuffer, workAddress, toRead);
        for(uint32_t i=0; i<toRead; i++) {
            if (pBuffer[i] != 0xFF) return 0; // Not empty
        }
        workAddress += toRead;
        remaining -= toRead;
    }
    return 1;
}

uint8_t W25QXX_IsEmpty_Block(W25QXX_HandleTypeDef *hflash, uint32_t Block_Address, uint32_t Offset_In_Byte, uint32_t NumByteToCheck_up_to_BlockSize) {
      uint8_t pBuffer[256]; 
    uint32_t workAddress = Block_Address + Offset_In_Byte;
    uint32_t remaining = NumByteToCheck_up_to_BlockSize;
    if (remaining > hflash->Info.BlockSize) remaining = hflash->Info.BlockSize;
    
    while (remaining > 0) {
        uint32_t toRead = (remaining > 256) ? 256 : remaining;
        W25QXX_Read(hflash, pBuffer, workAddress, toRead);
        for(uint32_t i=0; i<toRead; i++) {
            if (pBuffer[i] != 0xFF) return 0; // Not empty
        }
        workAddress += toRead;
        remaining -= toRead;
    }
    return 1;
}

void W25QXX_ReadUniqID(W25QXX_HandleTypeDef *hflash) {
    W25QXX_CS_Low(hflash);
    W25QXX_Spi(hflash, 0x4B); // RUID CMD
    uint8_t dummy[4] = {0};
    HAL_SPI_Transmit(hflash->hspi, dummy, 4, 100);
    HAL_SPI_Receive(hflash->hspi, hflash->Info.UniqID, 8, 100);
    W25QXX_CS_High(hflash);
}
