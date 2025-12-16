/**
 * @file letter-shell_tests.c
 * @brief Test Letter Shell with UART
 */

#include "letter-shell_port.h"
#include "drivers/uart.h"
#include <stdio.h>

// Forward declarations for diagnostic functions
uint32_t UART_GetPEErrorCount(UART_Channel ch);
uint32_t UART_GetNEErrorCount(UART_Channel ch);
uint32_t UART_GetFEErrorCount(UART_Channel ch);
uint32_t UART_GetOREErrorCount(UART_Channel ch);
uint32_t UART_GetDMAErrorCount(UART_Channel ch);

void User_Entry(void) {
    // 1. Hardware Init
    UART_Init();
    
    // 2. Shell Init
    Shell_Port_Init();
    
    UART_Debug_Printf("Shell Ready. Type 'help' for commands.\r\n");
    
    // Diagnostic counters
    static uint32_t last_error_cnt = 0;
    static uint32_t last_ore_cnt = 0;
    static uint32_t loop_counter = 0;
    static uint32_t last_status_time = 0;
    
    // 3. Main Loop
    while(1) {
        // 关键：必须调用 UART_Poll 来驱动 TX/RX 状态机
        UART_Poll();
        
        // Poll for input
        Shell_Task();
        
        // 每5秒输出一次状态（诊断用）
        loop_counter++;
        uint32_t now = HAL_GetTick();
        if (now - last_status_time >= 5000) {
            last_status_time = now;
            
            uint32_t err = UART_GetErrorCount(UART_DEBUG_CHANNEL);
            uint32_t ore = UART_GetOREErrorCount(UART_DEBUG_CHANNEL);
            
            // 只在有新错误时输出
            if (err != last_error_cnt || ore != last_ore_cnt) {
                UART_Debug_Printf("\r\n[DIAG] Errors: %lu, ORE: %lu, DMA: %lu, loops: %lu\r\n",
                    err, ore, 
                    UART_GetDMAErrorCount(UART_DEBUG_CHANNEL),
                    loop_counter);
                last_error_cnt = err;
                last_ore_cnt = ore;
            }
            loop_counter = 0;
        }
        
        // 注意：不要在这里使用 HAL_Delay，它会阻塞主循环！
        // 如果需要降低 CPU 使用率，可以使用很短的延时
        // HAL_Delay(1); // 可选，但会降低响应速度
    }
}
