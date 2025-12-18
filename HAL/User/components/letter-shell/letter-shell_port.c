/**
 * @file letter-shell_port.c
 * @brief Letter Shell Port Implementation using drivers/uart
 */

#include "letter-shell_port.h"
#include "uart.h"
#include <stdio.h>

Shell shell;
char shell_buffer[512];

/**
 * @brief Write function for Shell
 */
short user_shell_write(char *data, unsigned short len)
{
    // Use the Debug Channel defined in drivers/uart.h
    // Ensure UART_Init() has been called in main.
    UART_Send(UART_DEBUG_CHANNEL, (uint8_t*)data, len);
    return len;
}

/**
 * @brief Shell Init
 */
void Shell_Port_Init(void)
{
    shell.write = user_shell_write;
    
    // shellInit assigns the buffer and sets up defaults
    shellInit(&shell, shell_buffer, 512);
}

/**
 * @brief Polling Task to feed Shell
 *        Since drivers/uart.c handles the ISR and stores data in a RingBuffer,
 *        we just poll the RingBuffer here and feed the shell parser.
 */
void Shell_Task(void)
{
    uint8_t rx_data;
    
    // Process all pending bytes in the RingBuffer
    while (UART_Available(UART_DEBUG_CHANNEL) > 0) {
        if (UART_Read(UART_DEBUG_CHANNEL, &rx_data)) {
            shellHandler(&shell, rx_data);
        }
    }
}

/* ===================================================
   Example Command Registration
   =================================================== */

// Simple Hello Command
int hello_world(int argc, char *argv[]) {
    shellPrint(&shell, "Hello from STM32 Swissknife (UART Mode)!\r\n");
    if (argc > 1) {
        shellPrint(&shell, "Arg 1: %s\r\n", argv[1]);
    }
    return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), hello, hello_world, Test Hello Command);
