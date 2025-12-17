/**
 * @file internal_flash.c
 * @brief STM32 Internal Flash Driver Implementation
 */

#include "internal_flash.h"
#include <string.h>

/* Defines for Flash Page Size if not in HAL */
#ifndef FLASH_PAGE_SIZE
  #if defined(STM32F103xB)
    #define FLASH_PAGE_SIZE 0x400U  // 1KB for Medium Density
  #elif defined(STM32F103xE)
    #define FLASH_PAGE_SIZE 0x800U  // 2KB for High Density
  #else
    #define FLASH_PAGE_SIZE 0x800U  // Default to 2KB safer? Or dynamic check.
  #endif
#endif

// Helper to get flash size from hardware register on F1 series
// 0x1FFFF7E0 holds Flash size in KBytes on F103
#define FLASH_SIZE_REG_ADDR 0x1FFFF7E0

uint32_t InternalFlash_GetLastPageAddress(void) {
    uint16_t flash_size_kb = *(__IO uint16_t *)FLASH_SIZE_REG_ADDR;
    uint32_t flash_end_addr = 0x08000000 + (uint32_t)flash_size_kb * 1024;
    
    // For F103 High Density, Page size is 2KB, others 1KB.
    // Simplifying assumption: Standard F103C8T6 (MD) = 1KB Pages. F103ZET6 (HD) = 2KB.
    // It's safer to configure this in a project config, but let's try to deduce or use the defined macro.
    
    return flash_end_addr - FLASH_PAGE_SIZE;
}

bool InternalFlash_ErasePage(uint32_t pageAddress) {
    if (HAL_FLASH_Unlock() != HAL_OK) return false;

    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t PageError = 0;

    EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.PageAddress = pageAddress;
    EraseInitStruct.NbPages     = 1;

    bool success = (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) == HAL_OK);

    HAL_FLASH_Lock();
    return success;
}

bool InternalFlash_WriteBytes(uint32_t address, const uint8_t *data, uint32_t length) {
    if (HAL_FLASH_Unlock() != HAL_OK) return false;

    // STM32F1 programming is done by Half-Word (16-bit)
    // We need to handle odd lengths or unaligned streams carefully.
    // For simplicity, we assume we can write 16-bit chunks.
    
    HAL_StatusTypeDef status = HAL_OK;
    uint32_t current_addr = address;
    uint32_t i = 0;

    while (i < length) {
        uint16_t halfword = 0;
        
        // Low byte
        halfword |= data[i];
        i++;
        
        // High byte (pad with 0xFF if end of stream, to not disturb existing if possible, 
        // but flash erase sets 0xFF, so writing 0xFF is a No-Op safely)
        if (i < length) {
            halfword |= (uint16_t)(data[i] << 8);
            i++;
        } else {
            halfword |= 0xFF00; 
        }

        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, current_addr, halfword);
        
        if (status != HAL_OK) {
            break;
        }
        current_addr += 2;
    }

    HAL_FLASH_Lock();
    return (status == HAL_OK);
}

void InternalFlash_ReadBytes(uint32_t address, uint8_t *buffer, uint32_t length) {
    // Flash is memory mapped, direct copy
    // Cast address to pointer
    uint8_t *src = (uint8_t *)address;
    for (uint32_t i = 0; i < length; i++) {
        buffer[i] = src[i];
    }
}
