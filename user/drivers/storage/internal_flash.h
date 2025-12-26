/**
 * @file internal_flash.h
 * @brief STM32 Internal Flash Driver for saving configuration/data
 * @details Provides erase, read, and write operations for internal flash memory.
 *          Automatically handles unlocking and locking.
 * @author Standard Implementation
 */

#ifndef INTERNAL_FLASH_H
#define INTERNAL_FLASH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief  Calculate the starting address of the last page in Flash.
 * @note   Useful for storing persistent user data.
 * @return Address of the last page.
 */
uint32_t InternalFlash_GetLastPageAddress(void);

/**
 * @brief  Erase a specific page in Flash.
 * @param  pageAddress Any address belonging to the page to be erased.
 * @return true if successful, false otherwise.
 */
bool InternalFlash_ErasePage(uint32_t pageAddress);

/**
 * @brief  Write a buffer of bytes to Flash.
 * @note   The destination area must be erased (0xFF) before writing!
 *         This function handles 16-bit/32-bit alignment automatically.
 * @param  address Start address in Flash (must be 2-byte aligned for F1).
 * @param  data Pointer to source data buffer.
 * @param  length Number of bytes to write.
 * @return true if successful, false otherwise.
 */
bool InternalFlash_WriteBytes(uint32_t address, const uint8_t *data, uint32_t length);

/**
 * @brief  Read a buffer of bytes from Flash.
 * @note   Flash is memory mapped, so this just does a memcpy, but provides a clean API.
 * @param  address Start address in Flash.
 * @param  buffer Pointer to destination buffer.
 * @param  length Number of bytes to read.
 */
void InternalFlash_ReadBytes(uint32_t address, uint8_t *buffer, uint32_t length);

/* 
 * Utility structure to easy save/load configuration
 */
typedef struct {
    uint32_t magic_number;  // 0xCAFEBABE to verify validity
    // Add your custom fields here
    uint32_t boot_count;
    float pid_p;
    float pid_i;
    float pid_d;
    char wifi_ssid[32];
    char wifi_pwd[32];
} UserConfig_t;

#ifdef __cplusplus
}
#endif

#endif // INTERNAL_FLASH_H
