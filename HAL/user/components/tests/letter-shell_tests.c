/**
 * @file letter-shell_tests.c
 * @brief Test Letter Shell with UART DMA
 */

#include "../letter-shell/letter-shell_port.h"
#include "uart.h"
#include <stdio.h>

void User_Entry(void) {
    // 1. 初始化 UART 驱动
    UART_Init();
    
    // 2. 初始化 Letter Shell
    Shell_Port_Init();
    
    UART_Debug_Printf("\r\n");
    UART_Debug_Printf("===========================================\r\n");
    UART_Debug_Printf("  STM32 SwissKnife - Letter Shell Ready\r\n");
    UART_Debug_Printf("===========================================\r\n");
    UART_Debug_Printf("Type 'help' for available commands.\r\n\r\n");
    
    // 3. 主循环
    while(1) {
        // 驱动 UART 状态机（必须调用）
        UART_Poll();
        
        // 处理 Shell 输入
        Shell_Task();
    }
}
