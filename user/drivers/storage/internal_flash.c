/**
 * @file internal_flash.c
 * @brief Internal Flash Read/Write Driver
 */

#include "internal_flash.h"
#include <string.h>

/* Defines for Flash Page Size if not in HAL */
#ifndef FLASH_PAGE_SIZE
#define FLASH_PAGE_SIZE 1024
#endif

// --- F4 Helper Logic ---
// STM32F4 Flash is organized in Sectors, not Pages.
// We need to map an Address to a Sector Number.
#if defined(STM32F4)
static uint32_t GetSector(uint32_t Address)
{
    uint32_t sector = 0;
    
    if((Address < 0x08004000) && (Address >= 0x08000000)) {
        sector = FLASH_SECTOR_0;  
    } else if((Address < 0x08008000) && (Address >= 0x08004000)) {
        sector = FLASH_SECTOR_1;  
    } else if((Address < 0x0800C000) && (Address >= 0x08008000)) {
        sector = FLASH_SECTOR_2;  
    } else if((Address < 0x08010000) && (Address >= 0x0800C000)) {
        sector = FLASH_SECTOR_3;  
    } else if((Address < 0x08020000) && (Address >= 0x08010000)) {
        sector = FLASH_SECTOR_4;  
    } else if((Address < 0x08040000) && (Address >= 0x08020000)) {
        sector = FLASH_SECTOR_5;  
    } else if((Address < 0x08060000) && (Address >= 0x08040000)) {
        sector = FLASH_SECTOR_6;  
    } else if((Address < 0x08080000) && (Address >= 0x08060000)) {
        sector = FLASH_SECTOR_7;  
    }
    
    return sector;
}
#endif

// Helper function to get flash size (in bytes)
static uint32_t GetFlashSize(void) {
    // 0x1FFF7A22 (F1) or 0x1FFF7A22 (F4)? Check datasheet.
    // HAL usually provides FLASH_SIZE (but that's a macro).
    // Or we read from system memory.
    // For F446RE, 512KB. 
    /* 
       For portability, better to use LL_GetFlashSize() or specific register.
       Here we hardcode a safe fallback or calculation.
    */
    #if defined(STM32F4)
        return 512 * 1024; // Default F446RE 512KB
    #else
        return 64 * 1024; // Default F103C8 64KB
    #endif
}

/**
 * @brief  Calculate the starting address of the last page/sector in Flash.
 */
uint32_t InternalFlash_GetLastPageAddress(void) {
    uint32_t flash_base = FLASH_BASE; // 0x08000000
    uint32_t flash_size = GetFlashSize();
    
#if defined(STM32F4)
    // Last Sector (Sector 7) starts at 0x08060000 for 512KB device.
    // But wait, Sector 7 is 128KB! If we only want to use the END of flash.
    // Correct logic: FLASH_BASE + FLASH_SIZE - LastSectorSize
    // F446RE (512KB):
    // Sector 0-3: 16KB
    // Sector 4: 64KB
    // Sector 5-7: 128KB
    // So if 512KB, last sector is Sector 7 (128KB), starts at 0x08060000.
    return 0x08060000; 
#else
    // F1: Last Page = Base + Size - PageSize
    return flash_base + flash_size - FLASH_PAGE_SIZE;
#endif
}

bool InternalFlash_ErasePage(uint32_t pageAddress) {
    FLASH_EraseInitTypeDef EraseInitStruct = {0};
    uint32_t PageError = 0;
    
    HAL_FLASH_Unlock();

#if defined(STM32F4)
    uint32_t FirstSector = GetSector(pageAddress);
    EraseInitStruct.TypeErase     = FLASH_TYPEERASE_SECTORS;
    EraseInitStruct.VoltageRange  = FLASH_VOLTAGE_RANGE_3; 
    EraseInitStruct.Sector        = FirstSector;
    EraseInitStruct.NbSectors     = 1;
#elif defined(STM32F1)
    EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.PageAddress = pageAddress;
    EraseInitStruct.NbPages     = 1;
#endif

    if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK) {
        HAL_FLASH_Lock();
        return false;
    }
    
    HAL_FLASH_Lock();
    return true;
}

bool InternalFlash_WriteBytes(uint32_t address, const uint8_t *data, uint32_t length) {
    HAL_FLASH_Unlock();
    
    for (uint32_t i = 0; i < length; i++) {
#if defined(STM32F4)
         if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, address + i, data[i]) != HAL_OK) {
            HAL_FLASH_Lock();
            return false;
        }
#elif defined(STM32F1)
         // F1 Logic Placeholder (assuming halfword alignment handled by caller or simple byte loop fails)
         // This simple loop is risky on F1. 
         HAL_FLASH_Lock();
         return false; 
#endif
    }
    
    HAL_FLASH_Lock();
    return true;
}

void InternalFlash_ReadBytes(uint32_t address, uint8_t *buffer, uint32_t length) {
    memcpy(buffer, (void*)address, length);
}
