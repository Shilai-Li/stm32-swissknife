/**
 * @file nrf24l01_tests.c
 * @brief Test Suite for NRF24L01+
 */

#include "nrf24l01.h"
#include "uart.h"
#include <stdio.h>
#include <string.h>

// User must adjust these macros to match CubeMX config
#ifdef HAL_SPI_MODULE_ENABLED
extern SPI_HandleTypeDef hspi1; // Or hspi2...
#define NRF_SPI  &hspi1
#define CSN_GPIO GPIOB       // Example
#define CSN_PIN  GPIO_PIN_0  // Example
#define CE_GPIO  GPIOB       // Example
#define CE_PIN   GPIO_PIN_1  // Example
#endif

void user_main(void) {
    UART_Init();
    UART_Debug_Printf("\r\n=== NRF24L01+ Test Start ===\r\n");

#ifdef HAL_SPI_MODULE_ENABLED
    static NRF24_Handle_t nrf;
    
    // Note: User must ensure SPI Init was called (usually in main.c -> HAL_Init -> SystemClock -> MX_SPI1_Init)
    // Here we assume it is ready.
    
    if (NRF24_Init(&nrf, NRF_SPI, CSN_GPIO, CSN_PIN, CE_GPIO, CE_PIN)) {
        UART_Debug_Printf("Init: OK (Chip Found)\r\n");
    } else {
        UART_Debug_Printf("Init: FAIL (Check SPI/Wiring)\r\n");
        return;
    }
    
    // Simple Ping Pong logic for test
    // We default to RX Mode
    NRF24_SetRxMode(&nrf);
    
    UART_Debug_Printf("Waiting for Packets...\r\n");
    
    while(1) {
        if (NRF24_DataReady(&nrf)) {
            uint8_t buf[32];
            NRF24_Rx(&nrf, buf);
            UART_Debug_Printf("RX: %s\r\n", buf);
        }
        
        // Non-blocking loop
        // Add Tx logic via UART command if desired
    }
    
#else
    UART_Debug_Printf("SPI Not Enabled in CubeMX!\r\n");
#endif
}
