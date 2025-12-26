#include "uart.h"
#include "main.h"  // For HAL_Delay
#include "usart.h" // For huart2
#include <stdio.h>

// Define Logic Channel Map
#define CH_DEBUG 0 

// Helper to print stats
void print_stats(void) {
    UART_Debug_Printf("\r\n--- UART Statistics ---\r\n");
    UART_Debug_Printf("Rx Overrun: %lu\r\n", UART_GetRxOverrunCount(CH_DEBUG));
    UART_Debug_Printf("Tx Dropped: %lu\r\n", UART_GetTxDropCount(CH_DEBUG));
    UART_Debug_Printf("HW Errors : %lu\r\n", UART_GetErrorCount(CH_DEBUG));
    UART_Debug_Printf("DMA Errors: %lu\r\n", UART_GetDMAErrorCount(CH_DEBUG));
    UART_Debug_Printf("-----------------------\r\n");
}

void app_main(void)
{
    // 1. Register Logic Channel 0 -> Hardware UART2
    UART_Register(CH_DEBUG, &huart2);

    // 2. Test SendString
    UART_SendString(CH_DEBUG, "\r\n===================================\r\n");
    UART_SendString(CH_DEBUG, "      UART Driver Test Suite       \r\n");
    UART_SendString(CH_DEBUG, "===================================\r\n");
    UART_Debug_Printf("Cmds: [s]SendBursts [f]Flush [b]BusyCheck [e]Errors [r]BlockRx\r\n");

    uint32_t last_tick = 0;

    while (1)
    {
        // 3. Essential Polling (Must be called frequently)
        UART_Poll();

        // 4. Heartbeat (Non-blocking TX test)
        if (HAL_GetTick() - last_tick > 1000) {
            last_tick = HAL_GetTick();
            UART_Debug_Printf("[Tick] System Alive: %lu ms\r\n", last_tick);
        }

        // 5. Check Reception
        if (UART_Available(CH_DEBUG))
        {
            uint8_t cmd;
            if (UART_Read(CH_DEBUG, &cmd)) 
            {
                switch (cmd) 
                {
                    case 's': // Test Burst Send
                        UART_SendString(CH_DEBUG, "[Test] Sending Burst 1...\r\n");
                        UART_SendString(CH_DEBUG, "[Test] Sending Burst 2...\r\n");
                        UART_SendString(CH_DEBUG, "[Test] Sending Burst 3...\r\n");
                        break;
                    
                    case 'f': // Test Flush
                        UART_Debug_Printf("[Test] Flushing Rx Buffer... ");
                        UART_Flush(CH_DEBUG);
                        UART_Debug_Printf("Done. (Any pending chars lost)\r\n");
                        break;

                    case 'b': // Test IsBusy
                        UART_SendString(CH_DEBUG, "A long string to make it busy...\r\n");
                        if (UART_IsTxBusy(CH_DEBUG)) {
                            UART_SendString(CH_DEBUG, "[Test] Bus is Busy! (Good)\r\n");
                        } else {
                            UART_SendString(CH_DEBUG, "[Test] Bus is Idle? (Maybe too fast)\r\n");
                        }
                        break;

                    case 'e': // Test Error Getters
                        print_stats();
                        break;

                    case 'r': // Test Blocking Receive
                        UART_Debug_Printf("[Test] Entering Blocking Rx mode (5s timeout). Send a char now!\r\n");
                        uint8_t blocked_char;
                        // Clear pending first to ensure we block
                        UART_Flush(CH_DEBUG); 
                        if (UART_Receive(CH_DEBUG, &blocked_char, 5000)) {
                            UART_Debug_Printf("[Test] Received: %c\r\n", blocked_char);
                        } else {
                            UART_Debug_Printf("[Test] Timed out!\r\n");
                        }
                        break;

                    default: // Echo back
                        UART_Debug_Printf("Echo: %c (0x%02X)\r\n", cmd, cmd);
                        break;
                }
            }
        }
    }
}
