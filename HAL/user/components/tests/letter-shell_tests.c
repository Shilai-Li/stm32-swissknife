/**
 * @file letter-shell_tests.c
 * @brief 最简单的 UART 回显测试 - 用于诊断卡顿问题
 */

#include "main.h"
#include "drivers/uart.h"
#include <stdio.h>

// 简单的回显测试，完全绕过 letter-shell
void User_Entry(void) {
    // 初始化 UART
    UART_Init();
    
    // 使用板载 LED (通常是 PC13) 指示程序运行状态
    // 如果 LED 停止闪烁，说明程序卡住了
    
    UART_Debug_Printf("\r\n\r\n=== Simple Echo Test ===\r\n");
    UART_Debug_Printf("Type anything and it should echo back immediately.\r\n");
    UART_Debug_Printf("LED should blink every 100ms.\r\n\r\n");
    
    uint32_t last_blink = 0;
    uint32_t loop_count = 0;
    uint32_t rx_count = 0;
    
    while(1) {
        // 必须调用 UART_Poll 驱动状态机
        UART_Poll();
        
        // 简单回显：收到什么就发什么
        uint8_t ch;
        while (UART_Read(UART_DEBUG_CHANNEL, &ch)) {
            rx_count++;
            // 直接回显
            UART_Send(UART_DEBUG_CHANNEL, &ch, 1);
        }
        
        // LED 闪烁指示程序正在运行（每100ms翻转一次）
        uint32_t now = HAL_GetTick();
        if (now - last_blink >= 100) {
            last_blink = now;
            HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);  // 假设 LED 在 PC13
            loop_count++;
            
            // 每秒输出一次状态
            if (loop_count % 10 == 0) {
                // 不输出状态，避免干扰回显测试
                // 如果需要诊断，可以取消下面的注释
                // UART_Debug_Printf("[%lu] RX=%lu\r\n", now/1000, rx_count);
            }
        }
    }
}

