/**
 * @file letter-shell_tests.c
 * @brief Test Letter Shell with UART
 */

#include "letter-shell_port.h"
#include "drivers/uart.h"
#include <stdio.h>

void User_Entry(void) {
    // 1. Hardware Init
    // Usually done in main.c, but Ensure UART Driver is ready
    UART_Init();
    
    // 2. Shell Init
    Shell_Port_Init();
    
    UART_Debug_Printf("Shell Ready. Please connect UART terminal.\r\n");
    UART_Debug_Printf("Debug Channel is usually USART2 (PA2/PA3) or as config in uart.h\r\n");
    
    // 3. Main Loop
    while(1) {
        // Poll for input
        Shell_Task();
        
        // Blink or logic
        HAL_Delay(10);
    }
}
