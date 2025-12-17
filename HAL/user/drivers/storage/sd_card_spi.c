/**
 * @file sd_card_spi.c
 * @brief SD Card SPI Driver Implementation
 * @author Standard Implementation
 * @date 2024
 */

#include "drivers/sd_card_spi.h"
#include <string.h>

// --- Definitions ---
#define SD_DUMMY_BYTE   0xFF

// SD Commands
#define CMD0    0
#define CMD1    1
#define CMD8    8
#define CMD9    9
#define CMD10   10
#define CMD12   12
#define CMD16   16
#define CMD17   17
#define CMD18   18
#define CMD23   23
#define CMD24   24
#define CMD25   25
#define CMD41   41
#define CMD55   55
#define CMD58   58
#define CMD59   59

// R1 Response Flags
#define R1_IDLE_STATE   0x01
#define R1_ERASE_RESET  0x02
#define R1_ILLEGAL_CMD  0x04
#define R1_CRC_ERR      0x08
#define R1_ERASE_SEQ_ERR 0x10
#define R1_ADDR_ERR     0x20
#define R1_PARAM_ERR    0x40

// --- Private Functions ---

static void SD_SPI_Select(SD_Card_SPI_HandleTypeDef *hsd) {
    HAL_GPIO_WritePin(hsd->CsPort, hsd->CsPin, GPIO_PIN_RESET);
}

static void SD_SPI_Deselect(SD_Card_SPI_HandleTypeDef *hsd) {
    HAL_GPIO_WritePin(hsd->CsPort, hsd->CsPin, GPIO_PIN_SET);
    // Send a dummy byte after deselecting to release MISO
    uint8_t dummy = SD_DUMMY_BYTE;
    HAL_SPI_Transmit(hsd->hspi, &dummy, 1, 10);
}

static uint8_t SD_SPI_TxRx(SD_Card_SPI_HandleTypeDef *hsd, uint8_t data) {
    uint8_t received;
    HAL_SPI_TransmitReceive(hsd->hspi, &data, &received, 1, 100);
    return received;
}

static uint8_t SD_SPI_WaitReady(SD_Card_SPI_HandleTypeDef *hsd) {
    uint8_t res;
    // Wait for card to release data line (0xFF)
    // Timeout of 500ms
    uint32_t start = HAL_GetTick();
    do {
        res = SD_SPI_TxRx(hsd, SD_DUMMY_BYTE);
        if (res == 0xFF) return 0;
    } while (HAL_GetTick() - start < 500);
    return 1;
}

static uint8_t SD_SPI_SendCommand(SD_Card_SPI_HandleTypeDef *hsd, uint8_t cmd, uint32_t arg, uint8_t crc) {
    // Wait for card ready
    SD_SPI_WaitReady(hsd);
    
    // Transmit command
    uint8_t frame[6];
    frame[0] = 0x40 | cmd;
    frame[1] = (arg >> 24) & 0xFF;
    frame[2] = (arg >> 16) & 0xFF;
    frame[3] = (arg >> 8)  & 0xFF;
    frame[4] = arg & 0xFF;
    frame[5] = crc;
    
    // For specific commands, calculate CRC if needed, but in SPI mode CRC is disabled by default 
    // except for CMD0 and CMD8 which require valid CRC before init is complete.
    if (cmd == CMD0) frame[5] = 0x95;
    if (cmd == CMD8) frame[5] = 0x87;
    
    HAL_SPI_Transmit(hsd->hspi, frame, 6, 100);
    
    // Wait for response (R1)
    // Up to 10 bytes wait
    uint8_t res;
    for (int i = 0; i < 10; i++) {
        res = SD_SPI_TxRx(hsd, SD_DUMMY_BYTE);
        if ((res & 0x80) == 0) break; // Valid R1 response found
    }
    
    return res;
}


// --- Public Functions ---

uint8_t SD_SPI_Init(SD_Card_SPI_HandleTypeDef *hsd, SPI_HandleTypeDef *hspi, GPIO_TypeDef *cs_port, uint16_t cs_pin) {
    hsd->hspi = hspi;
    hsd->CsPort = cs_port; hsd->CsPin = cs_pin;
    hsd->Type = SD_CARD_TYPE_UKN;
    
    // 1. Initial Deselect and explicit dummy clocks
    SD_SPI_Deselect(hsd);
    
    // Power up delay
    HAL_Delay(10);
    
    // Send 80 dummy clock cycles (at least 74 required)
    uint8_t dummy[10];
    memset(dummy, 0xFF, sizeof(dummy));
    HAL_SPI_Transmit(hsd->hspi, dummy, 10, 100);
    
    // 2. Enter Idle State (CMD0)
    // CS Low
    SD_SPI_Select(hsd);
    
    uint8_t n, type, ocr[4];
    uint8_t res;

    // Retry CMD0 multiple times if needed
    for(n=0; n<10; n++) {
        res = SD_SPI_SendCommand(hsd, CMD0, 0, 0x95);
        if (res == R1_IDLE_STATE) break;
    }
    
    if (res != R1_IDLE_STATE) {
        SD_SPI_Deselect(hsd);
        return 1; // Initialization failed at CMD0
    }
    
    // 3. Check V2 (CMD8)
    if (SD_SPI_SendCommand(hsd, CMD8, 0x1AA, 0x87) == R1_IDLE_STATE) {
        // Get remaining 4 bytes of R7 response
        for (n = 0; n < 4; n++) ocr[n] = SD_SPI_TxRx(hsd, SD_DUMMY_BYTE);
        
        if (ocr[2] == 0x01 && ocr[3] == 0xAA) {
            // Card is V2. Wait for Init (ACMD41) with HCS bit set
            // Timeout 1s
            uint32_t start = HAL_GetTick();
            do {
                 // APP CMD
                 SD_SPI_SendCommand(hsd, CMD55, 0, 0);
                 res = SD_SPI_SendCommand(hsd, CMD41, 0x40000000, 0); // HCS = 1
            } while (res && (HAL_GetTick() - start < 1000));
            
            if (res == 0) {
                // Read CCS (CMD58)
                if (SD_SPI_SendCommand(hsd, CMD58, 0, 0) == 0) {
                    for (n = 0; n < 4; n++) ocr[n] = SD_SPI_TxRx(hsd, SD_DUMMY_BYTE);
                    type = (ocr[0] & 0x40) ? SD_CARD_TYPE_V2HC : SD_CARD_TYPE_V2;
                }
            }
        }
    } else {
        // SD V1 or MMC
        // Try SD V1 (ACMD41)
        type = SD_CARD_TYPE_V1;
        uint32_t start = HAL_GetTick();
        do {
            SD_SPI_SendCommand(hsd, CMD55, 0, 0);
            res = SD_SPI_SendCommand(hsd, CMD41, 0, 0);
        } while (res && (HAL_GetTick() - start < 1000));
        
        // If fail, try MMC (CMD1)
        if (res != 0) {
            type = SD_CARD_TYPE_MMC;
            start = HAL_GetTick();
            do {
                res = SD_SPI_SendCommand(hsd, CMD1, 0, 0);
            } while (res != 0 && (HAL_GetTick() - start < 1000));
        }
        
        if (res != 0) type = SD_CARD_TYPE_UKN; // Failed
    }
    
    hsd->Type = type;
    
    // Set Block Size to 512 (CMD16) - standard for V1/MMC, fixed for V2HC
    if (type != SD_CARD_TYPE_UKN) {
        SD_SPI_SendCommand(hsd, CMD16, 512, 0);
    }
    
    SD_SPI_Deselect(hsd);
    
    // Optionally read CSD to get capacity (omitted for brevity, can implement if needed)
    hsd->Capacity = 0; // Filled if CSD read implemented
    
    return (type == SD_CARD_TYPE_UKN) ? 2 : 0;
}

// Read Block (Single)
uint8_t SD_SPI_ReadBlock(SD_Card_SPI_HandleTypeDef *hsd, uint32_t sector, uint8_t *buffer) {
    if (hsd->Type == SD_CARD_TYPE_UKN) return 1;
    
    uint32_t addr = sector;
    if (hsd->Type != SD_CARD_TYPE_V2HC) {
        addr *= 512; // Byte address for non-HC cards
    }
    
    SD_SPI_Select(hsd);
    
    if (SD_SPI_SendCommand(hsd, CMD17, addr, 0) == 0) {
        // Wait for data token (0xFE)
        // Timeout 100ms
        uint32_t start = HAL_GetTick();
        uint8_t token;
        do {
            token = SD_SPI_TxRx(hsd, SD_DUMMY_BYTE);
        } while (token == 0xFF && (HAL_GetTick() - start < 200));
        
        if (token == 0xFE) {
            // Read 512 bytes
            HAL_SPI_Receive(hsd->hspi, buffer, 512, 500);
            // Read CRC (2 bytes) - throw away
            SD_SPI_TxRx(hsd, SD_DUMMY_BYTE);
            SD_SPI_TxRx(hsd, SD_DUMMY_BYTE);
            
            SD_SPI_Deselect(hsd);
            return 0; // Success
        }
    }
    
    SD_SPI_Deselect(hsd);
    return 2; // Read error
}

// Write Block (Single)
uint8_t SD_SPI_WriteBlock(SD_Card_SPI_HandleTypeDef *hsd, uint32_t sector, const uint8_t *buffer) {
    if (hsd->Type == SD_CARD_TYPE_UKN) return 1;
    
    uint32_t addr = sector;
    if (hsd->Type != SD_CARD_TYPE_V2HC) {
        addr *= 512;
    }
    
    SD_SPI_Select(hsd);
    
    if (SD_SPI_SendCommand(hsd, CMD24, addr, 0) == 0) {
        // Send Data Token (0xFE)
        SD_SPI_TxRx(hsd, 0xFE);
        
        // Write 512 bytes
        HAL_SPI_Transmit(hsd->hspi, (uint8_t*)buffer, 512, 500); // cast const away safely here for HAL API
        
        // Send Dummy CRC
        SD_SPI_TxRx(hsd, 0xFF);
        SD_SPI_TxRx(hsd, 0xFF);
        
        // Receive Data Response ((xxx0010x) & 0x1F) -> 0x05 accepted
        uint8_t resp = SD_SPI_TxRx(hsd, SD_DUMMY_BYTE);
        
        if ((resp & 0x1F) == 0x05) {
            // Wait while busy (store finishes)
            SD_SPI_WaitReady(hsd);
            SD_SPI_Deselect(hsd);
            return 0; // Success
        }
    }
    
    SD_SPI_Deselect(hsd);
    return 2; // Write Error
}

uint8_t SD_SPI_ReadBlocks(SD_Card_SPI_HandleTypeDef *hsd, uint32_t sector, uint8_t *buffer, uint32_t count) {
    for (uint32_t i=0; i<count; i++) {
        if(SD_SPI_ReadBlock(hsd, sector+i, buffer + (i*512)) != 0) return 1;
    }
    return 0;
}

uint8_t SD_SPI_WriteBlocks(SD_Card_SPI_HandleTypeDef *hsd, uint32_t sector, const uint8_t *buffer, uint32_t count) {
    for (uint32_t i=0; i<count; i++) {
        if(SD_SPI_WriteBlock(hsd, sector+i, buffer + (i*512)) != 0) return 1;
    }
    return 0;
}
