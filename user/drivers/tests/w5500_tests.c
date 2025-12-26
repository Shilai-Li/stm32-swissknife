/**
 * @file w5500_tests.c  
 * @brief W5500 Ethernet Driver Test Cases
 * 
 * CubeMX Configuration: See w5500.h
 * 
 * Test Modes:
 * - TEST_LINK_CHECK: Check W5500 hardware and link status
 * - TEST_TCP_ECHO: TCP client echo test (requires echo server)
 */

#include "stm32f1xx_hal.h"
#include "w5500.h"
#include "uart.h"
#include "delay.h"
#include <stdio.h>
#include <string.h>

/* Test mode selection */
#define TEST_LINK_CHECK  1
#define TEST_TCP_ECHO    0

/* External SPI handle */
extern SPI_HandleTypeDef hspi1;

/* Print helper */
#define PRINT(fmt, ...) do { \
    char buf[128]; \
    snprintf(buf, sizeof(buf), fmt "\r\n", ##__VA_ARGS__); \
    UART_Send(UART_DEBUG_CHANNEL, (uint8_t*)buf, strlen(buf)); \
} while(0)

/* ========================================================================
 * Test Case 1: Link Check 
 * ======================================================================== */
#if TEST_LINK_CHECK

void app_main(void) {
    HAL_Delay(100);
    
    PRINT("\r\n\r\n");
    PRINT("╔════════════════════════════════════════╗");
    PRINT("║   W5500 Ethernet Link Check Test      ║");
    PRINT("╚════════════════════════════════════════╝");
    PRINT("");
    
    // Hardware configuration
    W5500_Config_t hw_cfg = {
        .hspi = &hspi1,
        .cs_port = GPIOA,
        .cs_pin = GPIO_PIN_4,
        .rst_port = NULL,  // No reset pin
        .rst_pin = 0
    };
    
    // Network configuration
    W5500_NetConfig_t net_cfg = {
        .mac = {0x00, 0x08, 0xDC, 0xAB, 0xCD, 0xEF},
        .ip = {192, 168, 1, 100},
        .gateway = {192, 168, 1, 1},
        .subnet = {255, 255, 255, 0}
    };
    
    PRINT("Initializing W5500...");
    if (W5500_Init(&hw_cfg, &net_cfg) != W5500_OK) {
        PRINT("❌ Init failed!");
        while(1) HAL_Delay(1000);
    }
    PRINT("✓ W5500 Initialized");
    
    // Check chip version
    uint8_t version = W5500_GetVersion();
    PRINT("Chip Version: 0x%02X %s", version, 
          (version == 0x04) ? "✓ (W5500)" : "❌ (Unknown)");
    
    // Check link status
    PRINT("");
    PRINT("Checking Ethernet link...");
    PRINT("(Connect Ethernet cable to see link up)");
    
    while(1) {
        bool link = W5500_IsLinkUp();
        PRINT("Link Status: %s", link ? "UP ✓" : "DOWN ❌");
        HAL_Delay(2000);
    }
}

#endif /* TEST_LINK_CHECK */

/* ========================================================================
 * Test Case 2: TCP Echo Client
 * ======================================================================== */
#if TEST_TCP_ECHO

void User_Entry(void) {
    // TODO: Implement after W5500_TCP functions are complete
    PRINT("TCP Echo test - To be implemented");
    while(1) HAL_Delay(1000);
}

#endif /* TEST_TCP_ECHO */
