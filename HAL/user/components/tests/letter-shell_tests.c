/**
 * @file letter-shell_tests.c
 * @brief Test Letter Shell with UART
 */

#include "letter-shell_port.h"
#include "drivers/uart.h"
#include <stdio.h>

// Forward declarations for new functions to get specific error counts
uint32_t UART_GetPEErrorCount(UART_Channel ch);
uint32_t UART_GetNEErrorCount(UART_Channel ch);
uint32_t UART_GetFEErrorCount(UART_Channel ch);
uint32_t UART_GetOREErrorCount(UART_Channel ch);
uint32_t UART_GetDMAErrorCount(UART_Channel ch);

void User_Entry(void) {
    // 1. Hardware Init
    // Usually done in main.c, but Ensure UART Driver is ready
    UART_Init();
    
    // 2. Shell Init
    Shell_Port_Init();
    
    // 3. Main Loop
    static uint32_t last_overrun = 0;
    static uint32_t last_pe_errors = 0;
    static uint32_t last_ne_errors = 0;
    static uint32_t last_fe_errors = 0;
    static uint32_t last_ore_errors = 0;
    static uint32_t last_dma_errors = 0;
    static uint32_t last_tx_drop = 0;
    static uint32_t last_errors = 0;

    while(1) {
        UART_Poll();
        
        // Poll for input
        Shell_Task();
        
        // Monitor Overrun
        uint32_t curr = UART_GetRxOverrunCount(UART_DEBUG_CHANNEL);
        uint32_t tx_drops = UART_GetTxDropCount(UART_DEBUG_CHANNEL);
        uint32_t errors = UART_GetErrorCount(UART_DEBUG_CHANNEL);
        uint32_t pe_errors = UART_GetPEErrorCount(UART_DEBUG_CHANNEL);
        uint32_t ne_errors = UART_GetNEErrorCount(UART_DEBUG_CHANNEL);
        uint32_t fe_errors = UART_GetFEErrorCount(UART_DEBUG_CHANNEL);
        uint32_t ore_errors = UART_GetOREErrorCount(UART_DEBUG_CHANNEL);
        uint32_t dma_errors = UART_GetDMAErrorCount(UART_DEBUG_CHANNEL);

        if (curr > last_overrun) {
             UART_Debug_Printf("\r\nUART RX Overrun! Total: %u\r\n", curr);
             last_overrun = curr;
        }
        if (tx_drops > last_tx_drop) {
             UART_Debug_Printf("\r\nUART TX Dropped! Total: %u\r\n", tx_drops);
             last_tx_drop = tx_drops;
        }
        if (errors > last_errors) {
             UART_Debug_Printf("\r\nUART HW Errors! Total: %u\r\n", errors);
             last_errors = errors;
        }
        if (pe_errors > last_pe_errors) {
             UART_Debug_Printf("\r\nUART PE Errors! Total: %u\r\n", pe_errors);
             last_pe_errors = pe_errors;
        }
        if (ne_errors > last_ne_errors) {
             UART_Debug_Printf("\r\nUART NE Errors! Total: %u\r\n", ne_errors);
             last_ne_errors = ne_errors;
        }
        if (fe_errors > last_fe_errors) {
             UART_Debug_Printf("\r\nUART FE Errors! Total: %u\r\n", fe_errors);
             last_fe_errors = fe_errors;
        }
        if (ore_errors > last_ore_errors) {
             UART_Debug_Printf("\r\nUART ORE Errors! Total: %u\r\n", ore_errors);
             last_ore_errors = ore_errors;
        }
        if (dma_errors > last_dma_errors) {
             UART_Debug_Printf("\r\nUART DMA Errors! Total: %u\r\n", dma_errors);
             last_dma_errors = dma_errors;
        }
    }
}
