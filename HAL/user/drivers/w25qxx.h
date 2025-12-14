/**
 * @file w25qxx.h
 * @brief W25Qxx SPI Flash Driver Header File
 * 
 * =================================================================================
 *                       >>> INTEGRATION GUIDE <<<
 * =================================================================================
 * 1. CubeMX Config (Connectivity -> SPIx):
 *    - Mode: Full-Duplex Master
 *    - Data Size: 8 bits
 *    - Prescaler: Low enough (e.g. 18MHz max usually safe, chip supports up to 80MHz)
 *    - CPOL: High, CPHA: 2 Edge (Mode 3) usually preferred for Flash, but Mode 0 often works.
 *      * CHECK DATASHEET: W25Qxx usually supports Mode 0 and Mode 3.
 * 
 * 2. GPIO:
 *    - CS (Chip Select): Any GPIO Output.
 * 
 * 3. Usage:
 *    W25Q_Init(&hspi1, GPIOB, GPIO_PIN_12);
 * =================================================================================
 */

#ifndef __W25QXX_H
#define __W25QXX_H

#include "main.h"

// Check if HAL SPI is included, otherwise include it manually (adjust for your specific MCU series)
#ifndef __STM32F1xx_HAL_SPI_H
#include "stm32f1xx_hal.h"
#endif

/* W25Qxx Commands */
#define W25QXX_WRITE_ENABLE      0x06
#define W25QXX_WRITE_DISABLE     0x04
#define W25QXX_READ_STATUS_REG1  0x05
#define W25QXX_READ_STATUS_REG2  0x35
#define W25QXX_WRITE_STATUS_REG  0x01
#define W25QXX_READ_DATA         0x03
#define W25QXX_FAST_READ         0x0B
#define W25QXX_PAGE_PROGRAM      0x02
#define W25QXX_BLOCK_ERASE_64K   0xD8
#define W25QXX_BLOCK_ERASE_32K   0x52
#define W25QXX_SECTOR_ERASE      0x20
#define W25QXX_CHIP_ERASE        0xC7
#define W25QXX_POWER_DOWN        0xB9
#define W25QXX_RELEASE_POWER_DOWN 0xAB
#define W25QXX_DEVICE_ID         0xAB
#define W25QXX_MANUFACTURER_ID   0x90
#define W25QXX_JEDEC_ID          0x9F

#define W25Q80  0x4014
#define W25Q16  0x4015
#define W25Q32  0x4016
#define W25Q64  0x4017
#define W25Q128 0x4018
#define W25Q256 0x4019

typedef struct {
	uint16_t  ID;
	uint8_t   UniqID[8];
	uint16_t  PageSize;
	uint32_t  PageCount;
	uint32_t  SectorSize;
	uint32_t  SectorCount;
	uint32_t  BlockSize;
	uint32_t  BlockCount;
	uint32_t  CapacityInKiloByte;
	uint8_t   Lock;
} W25QXX_Info_t;

typedef struct {
    SPI_HandleTypeDef *hspi;
    GPIO_TypeDef      *CsPort;
    uint16_t          CsPin;
    W25QXX_Info_t     Info;
} W25QXX_HandleTypeDef;

/* Function Prototypes */
uint8_t W25QXX_Init(W25QXX_HandleTypeDef *hflash, SPI_HandleTypeDef *hspi, GPIO_TypeDef *cs_port, uint16_t cs_pin);

void    W25QXX_Read(W25QXX_HandleTypeDef *hflash, uint8_t* pBuffer, uint32_t ReadAddr, uint32_t NumByteToRead);
void    W25QXX_Write(W25QXX_HandleTypeDef *hflash, uint8_t* pBuffer, uint32_t WriteAddr, uint32_t NumByteToWrite);

void    W25QXX_Erase_Sector(W25QXX_HandleTypeDef *hflash, uint32_t Address);
void    W25QXX_Erase_Block(W25QXX_HandleTypeDef *hflash, uint32_t Address);
void    W25QXX_Erase_Chip(W25QXX_HandleTypeDef *hflash); // CAUTION: Long duration

uint32_t W25QXX_ReadID(W25QXX_HandleTypeDef *hflash);
void     W25QXX_ReadUniqID(W25QXX_HandleTypeDef *hflash);

uint8_t  W25QXX_IsEmpty_Sector(W25QXX_HandleTypeDef *hflash, uint32_t Sector_Address, uint32_t Offset_In_Byte, uint32_t NumByteToCheck_up_to_SectorSize);
uint8_t  W25QXX_IsEmpty_Block(W25QXX_HandleTypeDef *hflash, uint32_t Block_Address, uint32_t Offset_In_Byte, uint32_t NumByteToCheck_up_to_BlockSize);

#endif // __W25QXX_H
