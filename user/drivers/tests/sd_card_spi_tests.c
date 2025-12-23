/**
 * @file sd_card_spi_tests.c
 * @brief SD Card SPI Driver Test Code
 */

#include "sd_card_spi.h"
#include "uart.h"
#include <string.h>

// --- Configuration ---
extern SPI_HandleTypeDef hspi1;

#ifndef SD_CS_PORT
#define SD_CS_PORT    GPIOB
#define SD_CS_PIN     GPIO_PIN_12
#endif

SD_Card_SPI_HandleTypeDef hsd;

void User_Entry(void)
{
    UART_Init();
    UART_Debug_Printf("\r\n--- SD Card SPI Test Start ---\r\n");

    // 1. Initialize
    // Often best to use low speed for Init (< 400kHz), then high speed.
    // Assuming SPI is configured reasonably (e.g. < 400kHz or driver handles it? No driver assumes configured SPI)
    // User should ensure SPI Init prescaler is large enough for first init, or just try.
    // Modern SD cards usually handle higher speeds even at init, but spec says 400kHz.
    
    UART_Debug_Printf("Initializing SD Card...\r\n");
    uint8_t res = SD_SPI_Init(&hsd, &hspi1, SD_CS_PORT, SD_CS_PIN);
    
    if (res == 0) {
        UART_Debug_Printf("Init Success!\r\n");
        UART_Debug_Printf("Card Type: %d (2=V1, 4=V2, 6=V2HC)\r\n", hsd.Type);
    } else {
        UART_Debug_Printf("Init Failed! Error: %d\r\n", res);
        while(1) HAL_Delay(1000);
    }

    // 2. Write Test
    uint32_t test_sector = 100;
    uint8_t write_buff[512];
    uint8_t read_buff[512];
    
    // Fill buffer
    for(int i=0; i<512; i++) write_buff[i] = (uint8_t)(i & 0xFF);
    strcpy((char*)write_buff, "Hello SD Card via SPI!"); // Put specific string at start
    
    UART_Debug_Printf("Writing to Sector %lu...\r\n", test_sector);
    if(SD_SPI_WriteBlock(&hsd, test_sector, write_buff) == 0) {
        UART_Debug_Printf("Write Success.\r\n");
    } else {
        UART_Debug_Printf("Write Failed!\r\n");
    }

    // 3. Read Test
    memset(read_buff, 0, 512);
    UART_Debug_Printf("Reading from Sector %lu...\r\n", test_sector);
    
    if(SD_SPI_ReadBlock(&hsd, test_sector, read_buff) == 0) {
        UART_Debug_Printf("Read Success.\r\n");
        
        // Verify
        if (memcmp(write_buff, read_buff, 512) == 0) {
            UART_Debug_Printf("Data Verification Passed!\r\n");
            UART_Debug_Printf("Content: %s\r\n", read_buff);
        } else {
            UART_Debug_Printf("Data Verification FAILED!\r\n");
        }
    } else {
        UART_Debug_Printf("Read Failed!\r\n");
    }

    UART_Debug_Printf("Tests Complete.\r\n");

    while (1) {
        HAL_Delay(1000);
    }
}
