/**
 * @file at24cxx_tests.c
 * @brief AT24Cxx I2C EEPROM Driver Test Code
 * 
 * CubeMX Configuration Guide:
 * 1. Connectivity -> I2C1 (or other I2C)
 *    - Mode: I2C
 *    - Speed: Standard (100kHz) or Fast (400kHz)
 *    - GPIO: PB6 (SCL), PB7 (SDA) - or your specific pinout
 *    - Pull-up resistors might be needed on SCL/SDA if not on board.
 * 
 * 2. System Core -> UART (for debug output)
 *    - Configure USART1 (or whichever is used by uart.c)
 */

#include "at24cxx.h"
#include "uart.h"
#include <string.h>

// --- Configuration ---
// Define the I2C handle used (defined in main.c)
extern I2C_HandleTypeDef hi2c1;

// Define the EEPROM model connected
#define EEPROM_TYPE  AT24C02  // Change to AT24C32, AT24C256 etc. as needed

static AT24CXX_HandleTypeDef hat24;

void app_main(void)
{
    UART_Init();
    UART_Debug_Printf("\r\n--- AT24Cxx EEPROM Test Start ---\r\n");

    // 1. Initialize
    UART_Debug_Printf("Initializing AT24Cxx (Type: %d)...\r\n", EEPROM_TYPE);
    
    // Default address 0xA0
    if (AT24CXX_Init(&hat24, &hi2c1, EEPROM_TYPE, 0xA0) == 0) {
        UART_Debug_Printf("Init Success! Device Ready.\r\n");
        UART_Debug_Printf("Capacity: %lu Bytes\r\n", hat24.Capacity);
        UART_Debug_Printf("Page Size: %d Bytes\r\n", hat24.PageSize);
    } else {
        UART_Debug_Printf("Init Failed! Check wiring and I2C config.\r\n");
        UART_Debug_Printf("Stalling...\r\n");
        while(1) HAL_Delay(1000);
    }

    // 2. Erase/Fill Test (Optional, we will just write a block)
    // Writing across page boundary test (Critical for EEPROM drivers)
    
    uint32_t start_addr = 0x00;
    // Create a buffer larger than page size to test page wrapping
    // AT24C02 Page is 8 bytes. Let's write 20 bytes.
    uint8_t write_buf[32]; 
    uint8_t read_buf[32];
    
    for(int i=0; i<sizeof(write_buf); i++) {
        write_buf[i] = 0x50 + i; // Fill with dummy data
    }
    
    UART_Debug_Printf("Writing %d bytes to Address 0x%X...\r\n", sizeof(write_buf), start_addr);
    AT24CXX_Write(&hat24, start_addr, write_buf, sizeof(write_buf));
    UART_Debug_Printf("Write Done.\r\n");

    // 3. Read Verification
    memset(read_buf, 0, sizeof(read_buf));
    UART_Debug_Printf("Reading back...\r\n");
    AT24CXX_Read(&hat24, start_addr, read_buf, sizeof(read_buf));
    
    // Verify
    if (memcmp(write_buf, read_buf, sizeof(write_buf)) == 0) {
        UART_Debug_Printf("Verification PASSED!\r\n");
        UART_Debug_Printf("Data: ");
        for(int i=0; i<10; i++) UART_Debug_Printf("%02X ", read_buf[i]);
        UART_Debug_Printf("...\r\n");
    } else {
        UART_Debug_Printf("Verification FAILED!\r\n");
        UART_Debug_Printf("Expected: ");
        for(int i=0; i<10; i++) UART_Debug_Printf("%02X ", write_buf[i]);
        UART_Debug_Printf("\r\nRead    : ");
        for(int i=0; i<10; i++) UART_Debug_Printf("%02X ", read_buf[i]);
        UART_Debug_Printf("\r\n");
    }

    // 4. Persistence Test (Optional: Restart board to check if data stays)
    
    UART_Debug_Printf("--- Test Loop Start ---\r\n");
    while (1) {
        // Blink LED to show life
        // HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13); 
        HAL_Delay(1000);
    }
}
