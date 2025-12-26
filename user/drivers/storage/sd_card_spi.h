/**
 * @file sd_card_spi.h
 * @brief SD Card SPI Driver Header File
 * @author Standard Implementation
 * @date 2024
 */

#ifndef __SD_CARD_SPI_H
#define __SD_CARD_SPI_H

#include "main.h"

#ifndef __STM32F1xx_HAL_SPI_H
#include "main.h"
#endif

// --- Card Types ---
#define SD_CARD_TYPE_UKN    0
#define SD_CARD_TYPE_MMC    1
#define SD_CARD_TYPE_V1     2
#define SD_CARD_TYPE_V2     4
#define SD_CARD_TYPE_V2HC   6

typedef struct {
    SPI_HandleTypeDef *hspi;
    GPIO_TypeDef      *CsPort;
    uint16_t          CsPin;
    uint8_t           Type;     // Card Type (SD_CARD_TYPE_...)
    uint32_t          Capacity; // Card Capacity in sectors (multiply by 512 for bytes)
} SD_Card_SPI_HandleTypeDef;

/* Function Prototypes */

/**
 * @brief Initialize the SD Card in SPI mode
 * @return 0 on success, >0 on error
 */
uint8_t SD_SPI_Init(SD_Card_SPI_HandleTypeDef *hsd, SPI_HandleTypeDef *hspi, GPIO_TypeDef *cs_port, uint16_t cs_pin);

/**
 * @brief Read a single block (512 bytes)
 * @param sector Sector address (LBA)
 * @param buffer Pointer to data buffer (must be at least 512 bytes)
 * @return 0 on success
 */
uint8_t SD_SPI_ReadBlock(SD_Card_SPI_HandleTypeDef *hsd, uint32_t sector, uint8_t *buffer);

/**
 * @brief Write a single block (512 bytes)
 * @param sector Sector address (LBA)
 * @param buffer Pointer to data buffer (512 bytes)
 * @return 0 on success
 */
uint8_t SD_SPI_WriteBlock(SD_Card_SPI_HandleTypeDef *hsd, uint32_t sector, const uint8_t *buffer);

/**
 * @brief Read multiple blocks
 */
uint8_t SD_SPI_ReadBlocks(SD_Card_SPI_HandleTypeDef *hsd, uint32_t sector, uint8_t *buffer, uint32_t count);

/**
 * @brief Write multiple blocks
 */
uint8_t SD_SPI_WriteBlocks(SD_Card_SPI_HandleTypeDef *hsd, uint32_t sector, const uint8_t *buffer, uint32_t count);

/**
 * @brief Get Card Capacity in KBytes
 */
uint32_t SD_SPI_GetCapacityKB(SD_Card_SPI_HandleTypeDef *hsd);

#endif // __SD_CARD_SPI_H
