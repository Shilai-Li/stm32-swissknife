#include "uart.h"
#include "main.h"  // For HAL_Delay
#include "usart.h" // For huart2
#include "usb_cdc.h" // For Virtual Serial Port
#include <stdio.h>
#include <stdarg.h> // For va_list

// Define Logic Channel Map
#define CH_DEBUG 2

// Helper to print to both UART and USB CDC
void Test_Printf(const char *fmt, ...) {
    char buffer[256];
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    if (len > 0) {
        UART_Send(CH_DEBUG, (uint8_t *)buffer, len);
        USB_CDC_Send((uint8_t *)buffer, len);
    }
}

// Helper to print stats
void print_stats(void) {
    Test_Printf("\r\n--- UART Statistics ---\r\n");
    Test_Printf("Rx Overrun: %lu\r\n", UART_GetRxOverrunCount(CH_DEBUG));
    Test_Printf("Tx Dropped: %lu\r\n", UART_GetTxDropCount(CH_DEBUG));
    Test_Printf("HW Errors : %lu\r\n", UART_GetErrorCount(CH_DEBUG));
    Test_Printf("DMA Errors: %lu\r\n", UART_GetDMAErrorCount(CH_DEBUG));
    Test_Printf("-----------------------\r\n");
}

// Define Buffers for Channel DEBUG
static uint8_t rx_dma_buf[64]; // Small DMA buffer
static uint8_t rx_ring_buf[256]; // Large software Ring Buffer
static uint8_t tx_ring_buf[256]; // TX Buffer

void app_main(void)
{
    // 1. Register Logic Channel 0 -> Hardware UART2
    UART_Register(CH_DEBUG, &huart2, 
                  rx_dma_buf, sizeof(rx_dma_buf),
                  rx_ring_buf, sizeof(rx_ring_buf),
                  tx_ring_buf, sizeof(tx_ring_buf));
    
    // 2. Initialize USB CDC
    USB_CDC_Init();

    // 3. Test SendString
    Test_Printf("\r\n===================================\r\n");
    Test_Printf("      UART Driver Test Suite       \r\n");
    Test_Printf("===================================\r\n");
    Test_Printf("Cmds: [s]SendBursts [f]Flush [b]BusyCheck [e]Errors [r]BlockRx\r\n");

    uint32_t last_tick = 0;

    while (1)
    {
        // 3. Essential Polling (Must be called frequently)
        UART_Poll();

        // 4. Heartbeat (Non-blocking TX test)
        if (HAL_GetTick() - last_tick > 1000) {
            last_tick = HAL_GetTick();
            Test_Printf("[Tick] System Alive: %lu ms\r\n", last_tick);
        }

        // 5. Check Reception (UART or USB CDC)
        uint8_t cmd;
        bool has_data = false;

        if (UART_Read(CH_DEBUG, &cmd)) {
             has_data = true;
        } else if (USB_CDC_Read(&cmd)) {
             has_data = true;
        }

        if (has_data)
        {
            switch (cmd) 
            {
                case 's': // Test Burst Send
                    Test_Printf("[Test] Sending Burst 1...\r\n");
                    Test_Printf("[Test] Sending Burst 2...\r\n");
                    Test_Printf("[Test] Sending Burst 3...\r\n");
                    break;
                
                case 'f': // Test Flush
                    Test_Printf("[Test] Flushing Rx Buffer... ");
                    UART_Flush(CH_DEBUG);
                    Test_Printf("Done. (Any pending chars lost)\r\n");
                    break;

                case 'b': // Test IsBusy
                    Test_Printf("A long string to make it busy...\r\n");
                    if (UART_IsTxBusy(CH_DEBUG)) {
                        Test_Printf("[Test] Bus is Busy! (Good)\r\n");
                    } else {
                        Test_Printf("[Test] Bus is Idle? (Maybe too fast)\r\n");
                    }
                    break;

                case 'e': // Test Error Getters
                    print_stats();
                    break;

                case 'r': // Test Blocking Receive
                    Test_Printf("[Test] Entering Blocking Rx mode (5s timeout). Send a char now!\r\n");
                    uint8_t blocked_char;
                    // Clear pending first to ensure we block
                    UART_Flush(CH_DEBUG); 
                    if (UART_Receive(CH_DEBUG, &blocked_char, 5000)) {
                        Test_Printf("[Test] Received: %c\r\n", blocked_char);
                    } else {
                        Test_Printf("[Test] Timed out!\r\n");
                    }
                    break;

                default: // Echo back
                    Test_Printf("Echo: %c (0x%02X)\r\n", cmd, cmd);
                    break;
            }
        }
    }
}
