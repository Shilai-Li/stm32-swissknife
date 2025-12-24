/**
 * @file internal_flash_tests.c
 * @brief Test Suite for Internal Flash Driver
 */

#include "internal_flash.h"
#include "uart.h"
#include <stdio.h>
#include <string.h>

void user_main(void) {
    UART_Init();
    UART_Debug_Printf("\r\n=== Internal Flash Test Start ===\r\n");

    // 1. Identify Target Address
    uint32_t target_addr = InternalFlash_GetLastPageAddress();
    UART_Debug_Printf("Target Page Address: 0x%08X\r\n", target_addr);

    // 2. Prepare Data
    UserConfig_t config_write;
    config_write.magic_number = 0xCAFEBABE;
    config_write.boot_count = 123;
    config_write.pid_p = 1.5f;
    config_write.pid_i = 0.02f;
    config_write.pid_d = 0.1f;
    strncpy(config_write.wifi_ssid, "TestWiFi_SSID", 32);
    
    // 3. Erase
    UART_Debug_Printf("Erasing Page...\r\n");
    if (InternalFlash_ErasePage(target_addr)) {
        UART_Debug_Printf("Erase OK.\r\n");
    } else {
        UART_Debug_Printf("Erase FAILED.\r\n");
        return;
    }

    // 4. Write
    UART_Debug_Printf("Writing Data (%d bytes)...\r\n", sizeof(UserConfig_t));
    if (InternalFlash_WriteBytes(target_addr, (uint8_t*)&config_write, sizeof(UserConfig_t))) {
        UART_Debug_Printf("Write OK.\r\n");
    } else {
        UART_Debug_Printf("Write FAILED.\r\n");
        return;
    }

    // 5. Read Back & Verify
    UserConfig_t config_read;
    InternalFlash_ReadBytes(target_addr, (uint8_t*)&config_read, sizeof(UserConfig_t));

    UART_Debug_Printf("Read Back Check:\r\n");
    UART_Debug_Printf("Magic: 0x%08X %s\r\n", config_read.magic_number, 
                      (config_read.magic_number == 0xCAFEBABE) ? "(MATCH)" : "(FAIL)");
    UART_Debug_Printf("Boot Count: %d\r\n", config_read.boot_count);
    // Integer comparison for float roughly
    UART_Debug_Printf("PID P: %d/10\r\n", (int)(config_read.pid_p * 10)); 
    UART_Debug_Printf("Details: SSID=%s\r\n", config_read.wifi_ssid);

    if (memcmp(&config_write, &config_read, sizeof(UserConfig_t)) == 0) {
        UART_Debug_Printf("=== TEST PASSED: Integrity Verified ===\r\n");
    } else {
        UART_Debug_Printf("=== TEST FAILED: Data Mismatch ===\r\n");
    }
}
