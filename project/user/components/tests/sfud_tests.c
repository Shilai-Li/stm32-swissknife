/**
 * @file sfud_tests.c
 * @brief SFUD Component Test Cases
 * @author stm32-swissknife Project
 * @date 2025
 * 
 * CubeMX Configuration for SPI Flash (W25Q64):
 * ═══════════════════════════════════════════════════════════════════════════
 * 
 * 1. SPI1 Configuration:
 *    ┌─────────────────┬───────────────────────────────────────────┐
 *    │ Parameter       │ Value                                     │
 *    ├─────────────────┼───────────────────────────────────────────┤
 *    │ Mode            │ Full-Duplex Master                        │
 *    │ NSS             │ Disable (use software CS)                 │
 *    │ Clock           │ 36 MHz (APB2/2) or as high as supported   │
 *    │ CPOL            │ Low                                       │
 *    │ CPHA            │ 1 Edge                                    │
 *    │ Data Size       │ 8 Bits                                    │
 *    │ First Bit       │ MSB First                                 │
 *    └─────────────────┴───────────────────────────────────────────┘
 * 
 * 2. GPIO Pin Configuration:
 *    ┌────────┬──────────┬────────────────┐
 *    │ Pin    │ Mode     │ Label          │
 *    ├────────┼──────────┼────────────────┤
 *    │ PA5    │ SPI1_SCK │ -              │
 *    │ PA6    │ SPI1_MISO│ -              │
 *    │ PA7    │ SPI1_MOSI│ -              │
 *    │ PA4    │ GPIO_Out │ FLASH_CS       │
 *    └────────┴──────────┴────────────────┘
 * 
 * 3. Important Notes:
 *    - Make sure PA4 is initialized to HIGH before SPI init
 *    - W25Q64 maximum SPI clock is 104 MHz (W25Q128 is 104 MHz, W25Q256 is 133 MHz)
 *    - Enable printf support for debug output (retarget to UART)
 * 
 * Hardware Connection:
 * ═══════════════════════════════════════════════════════════════════════════
 * 
 *     STM32F103           W25Q64 (SOP-8)
 *    ┌──────────┐        ┌──────────────┐
 *    │          │        │  1 CS   VCC 8│──── 3.3V
 *    │      PA4 ├────────┤  2 DO   GND 7│──── GND
 *    │          │        │  3 WP   HOLD 6│── (Pull High or NC)
 *    │     GND  ├────────┤  4 GND  CLK 5│
 *    │          │        └──────────────┘
 *    │      PA5 ├──────────────────────┘
 *    │      PA6 ├──────────────────────────┘
 *    │      PA7 ├─────────────────────────────────┘
 *    └──────────┘
 * 
 * Integration Steps:
 * ═══════════════════════════════════════════════════════════════════════════
 * 
 * 1. Add to CMakeLists.txt (TEST_CASE "sfud"):
 *    elseif(TEST_CASE STREQUAL "sfud")
 *        target_sources(${CMAKE_PROJECT_NAME} PRIVATE components/tests/sfud_tests.c)
 *        list(APPEND USER_SOURCES
 *                components/sfud/csrc/sfud.c
 *                components/sfud/csrc/sfud_sfdp.c
 *                components/sfud/sfud_port.c
 *        )
 *        list(APPEND USER_INCLUDES
 *                ${CMAKE_CURRENT_SOURCE_DIR}/components/sfud
 *                ${CMAKE_CURRENT_SOURCE_DIR}/components/sfud/csrc
 *        )
 * 
 * 2. In main.c, call:
 *    SFUD_Test_Entry();
 */

#include "sfud.h"
#include "sfud_port.h"
#include "main.h"
#include <stdio.h>
#include <string.h>

/*******************************************************************************
 * TEST CONFIGURATION
 ******************************************************************************/

#define TEST_SECTOR_ADDR    0x000000    // Test sector address
#define TEST_DATA_SIZE      256         // Test data size (must be <= page size)

/*******************************************************************************
 * TEST FUNCTIONS
 ******************************************************************************/

/**
 * @brief Test Flash identification
 */
static void test_flash_probe(void)
{
    printf("\r\n========== Test 1: Flash Probe ==========\r\n");
    
    const sfud_flash *flash = SFUD_Port_GetDefaultFlash();
    if (flash == NULL) {
        printf("[FAIL] No Flash device found!\r\n");
        return;
    }
    
    printf("[INFO] Flash Name: %s\r\n", flash->name);
    printf("[INFO] Chip Info:\r\n");
    printf("       - Manufacturer ID: 0x%02X\r\n", flash->chip.mf_id);
    printf("       - Type ID: 0x%02X\r\n", flash->chip.type_id);
    printf("       - Capacity ID: 0x%02X\r\n", flash->chip.capacity_id);
    printf("       - Capacity: %ld Bytes (%ld KB)\r\n", 
           flash->chip.capacity, flash->chip.capacity / 1024);
    printf("       - Write Granularity: %d Bytes\r\n", flash->chip.write_gran);
    printf("       - Erase Granularity: %ld Bytes (%ld KB)\r\n", 
           flash->chip.erase_gran, flash->chip.erase_gran / 1024);
    printf("[PASS] Flash probe successful\r\n");
}

/**
 * @brief Test Flash erase operation
 */
static void test_flash_erase(void)
{
    printf("\r\n========== Test 2: Flash Erase ==========\r\n");
    
    const sfud_flash *flash = SFUD_Port_GetDefaultFlash();
    if (flash == NULL) {
        printf("[FAIL] No Flash device found!\r\n");
        return;
    }
    
    printf("[INFO] Erasing sector at 0x%06X...\r\n", TEST_SECTOR_ADDR);
    
    sfud_err result = sfud_erase(flash, TEST_SECTOR_ADDR, flash->chip.erase_gran);
    if (result == SFUD_SUCCESS) {
        printf("[PASS] Sector erased successfully\r\n");
    } else {
        printf("[FAIL] Erase failed with error: %d\r\n", result);
    }
}

/**
 * @brief Test Flash write operation
 */
static void test_flash_write(void)
{
    printf("\r\n========== Test 3: Flash Write ==========\r\n");
    
    const sfud_flash *flash = SFUD_Port_GetDefaultFlash();
    if (flash == NULL) {
        printf("[FAIL] No Flash device found!\r\n");
        return;
    }
    
    // Prepare test data
    uint8_t write_data[TEST_DATA_SIZE];
    for (uint16_t i = 0; i < TEST_DATA_SIZE; i++) {
        write_data[i] = i & 0xFF;
    }
    
    printf("[INFO] Writing %d bytes at 0x%06X...\r\n", TEST_DATA_SIZE, TEST_SECTOR_ADDR);
    
    sfud_err result = sfud_write(flash, TEST_SECTOR_ADDR, TEST_DATA_SIZE, write_data);
    if (result == SFUD_SUCCESS) {
        printf("[PASS] Data written successfully\r\n");
    } else {
        printf("[FAIL] Write failed with error: %d\r\n", result);
    }
}

/**
 * @brief Test Flash read operation and verify data
 */
static void test_flash_read(void)
{
    printf("\r\n========== Test 4: Flash Read & Verify ==========\r\n");
    
    const sfud_flash *flash = SFUD_Port_GetDefaultFlash();
    if (flash == NULL) {
        printf("[FAIL] No Flash device found!\r\n");
        return;
    }
    
    // Prepare expected data
    uint8_t expected_data[TEST_DATA_SIZE];
    for (uint16_t i = 0; i < TEST_DATA_SIZE; i++) {
        expected_data[i] = i & 0xFF;
    }
    
    // Read data
    uint8_t read_data[TEST_DATA_SIZE];
    printf("[INFO] Reading %d bytes from 0x%06X...\r\n", TEST_DATA_SIZE, TEST_SECTOR_ADDR);
    
    sfud_err result = sfud_read(flash, TEST_SECTOR_ADDR, TEST_DATA_SIZE, read_data);
    if (result != SFUD_SUCCESS) {
        printf("[FAIL] Read failed with error: %d\r\n", result);
        return;
    }
    
    // Verify data
    printf("[INFO] Verifying data...\r\n");
    uint16_t errors = 0;
    for (uint16_t i = 0; i < TEST_DATA_SIZE; i++) {
        if (read_data[i] != expected_data[i]) {
            if (errors < 10) {  // Only print first 10 errors
                printf("[WARN] Mismatch at offset %d: expected 0x%02X, got 0x%02X\r\n",
                       i, expected_data[i], read_data[i]);
            }
            errors++;
        }
    }
    
    if (errors == 0) {
        printf("[PASS] Data verified successfully\r\n");
        
        // Print first 16 bytes as hex dump
        printf("[INFO] First 16 bytes:\r\n");
        printf("       ");
        for (uint8_t i = 0; i < 16; i++) {
            printf("%02X ", read_data[i]);
        }
        printf("\r\n");
    } else {
        printf("[FAIL] Data verification failed (%d/%d bytes mismatch)\r\n", 
               errors, TEST_DATA_SIZE);
    }
}

/**
 * @brief Test Flash erase-write operation
 */
static void test_flash_erase_write(void)
{
    printf("\r\n========== Test 5: Flash Erase-Write ==========\r\n");
    
    const sfud_flash *flash = SFUD_Port_GetDefaultFlash();
    if (flash == NULL) {
        printf("[FAIL] No Flash device found!\r\n");
        return;
    }
    
    // Prepare test pattern
    uint8_t pattern[TEST_DATA_SIZE];
    for (uint16_t i = 0; i < TEST_DATA_SIZE; i++) {
        pattern[i] = 0xAA;  // Alternating pattern
    }
    
    printf("[INFO] Erase-Write %d bytes at 0x%06X with pattern 0xAA...\r\n", 
           TEST_DATA_SIZE, TEST_SECTOR_ADDR);
    
    sfud_err result = sfud_erase_write(flash, TEST_SECTOR_ADDR, TEST_DATA_SIZE, pattern);
    if (result != SFUD_SUCCESS) {
        printf("[FAIL] Erase-Write failed with error: %d\r\n", result);
        return;
    }
    
    // Read back and verify
    uint8_t verify_data[TEST_DATA_SIZE];
    result = sfud_read(flash, TEST_SECTOR_ADDR, TEST_DATA_SIZE, verify_data);
    if (result != SFUD_SUCCESS) {
        printf("[FAIL] Read verification failed\r\n");
        return;
    }
    
    // Check pattern
    bool match = true;
    for (uint16_t i = 0; i < TEST_DATA_SIZE; i++) {
        if (verify_data[i] != 0xAA) {
            match = false;
            break;
        }
    }
    
    if (match) {
        printf("[PASS] Erase-Write verified successfully\r\n");
    } else {
        printf("[FAIL] Pattern verification failed\r\n");
    }
}

/**
 * @brief Test chip erase (WARNING: This will erase the entire chip!)
 */
static void test_flash_chip_erase(void)
{
    printf("\r\n========== Test 6: Chip Erase (SKIPPED) ==========\r\n");
    printf("[INFO] Chip erase test is commented out for safety.\r\n");
    printf("[INFO] Uncomment the code below if you want to test full chip erase.\r\n");
    
    /* UNCOMMENT TO ENABLE CHIP ERASE TEST - WARNING: DESTRUCTIVE!
    const sfud_flash *flash = SFUD_Port_GetDefaultFlash();
    if (flash == NULL) {
        printf("[FAIL] No Flash device found!\r\n");
        return;
    }
    
    printf("[WARN] This will erase the ENTIRE chip! Progress will take time...\r\n");
    printf("[INFO] Starting chip erase...\r\n");
    
    sfud_err result = sfud_chip_erase(flash);
    if (result == SFUD_SUCCESS) {
        printf("[PASS] Chip erased successfully\r\n");
    } else {
        printf("[FAIL] Chip erase failed with error: %d\r\n", result);
    }
    */
}

/*******************************************************************************
 * MAIN TEST ENTRY
 ******************************************************************************/

/**
 * @brief SFUD test entry point
 * 
 * Call this function from main() after system initialization.
 */
void SFUD_Test_Entry(void)
{
    printf("\r\n");
    printf("╔═══════════════════════════════════════════════════════════╗\r\n");
    printf("║         SFUD (Serial Flash Universal Driver) Tests       ║\r\n");
    printf("╚═══════════════════════════════════════════════════════════╝\r\n");
    
    // Initialize SFUD
    printf("[INFO] Initializing SFUD...\r\n");
    sfud_err result = SFUD_Port_Init();
    if (result != SFUD_SUCCESS) {
        printf("[FAIL] SFUD initialization failed with error: %d\r\n", result);
        printf("[HINT] Check your SPI and CS pin configuration in sfud_port.c\r\n");
        return;
    }
    
    // Run tests
    test_flash_probe();
    test_flash_erase();
    test_flash_write();
    test_flash_read();
    test_flash_erase_write();
    test_flash_chip_erase();
    
    // Summary
    printf("\r\n");
    printf("╔═══════════════════════════════════════════════════════════╗\r\n");
    printf("║                    Tests Complete                         ║\r\n");
    printf("╚═══════════════════════════════════════════════════════════╝\r\n");
    printf("\r\n");
}

/**
 * @brief User application entry point
 */
void User_Entry(void)
{
    // Run SFUD tests
    SFUD_Test_Entry();
    
    // Main loop
    while (1) {
        HAL_Delay(1000);
    }
}
