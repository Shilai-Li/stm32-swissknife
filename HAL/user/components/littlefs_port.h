/**
 * @file littlefs_port.h
 * @brief LittleFS Port for W25Qxx SPI Flash
 */

#ifndef LITTLEFS_PORT_H
#define LITTLEFS_PORT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "littlefs/lfs.h"
#include "drivers/w25qxx.h"

// --- Configuration ---
// Define the W25Qxx handle that LittleFS should use.
// You must initialize this handle in your main code before mounting LFS.
extern W25QXX_HandleTypeDef w25qxx_handle; 

/**
 * @brief Initialize the LittleFS configuration structure
 *        and Mount the filesystem.
 *        If mount fails (corrupt/first time), it will format and remount.
 * @return 0 on success, <0 on error
 */
int LittleFS_Port_Init(void);

/**
 * @brief Unmount filesystem
 */
void LittleFS_Port_DeInit(void);

// Expose the global lfs instance so user code can use lfs_open, lfs_read etc.
extern lfs_t lfs;
extern struct lfs_config cfg;

#ifdef __cplusplus
}
#endif

#endif
