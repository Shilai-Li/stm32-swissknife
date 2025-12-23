/**
 * @file flexible_button_tests.c
 * @brief Test application for FlexibleButton component
 */

#include "flexible_button.h"
#include "uart.h"
#include "stm32f1xx_hal.h"

// Define a scan interval
#define SCAN_INTERVAL_MS 20

// Custom callback for testing specific scenarios if needed
static void test_btn_callback(void *arg)
{
    flexible_button_t *btn = (flexible_button_t *)arg;
    UART_Debug_Printf("TEST CB: Button %d Event %d\r\n", btn->id, btn->event);
}

void FlexibleButton_Test_Entry(void)
{
    UART_Init();
    UART_Debug_Printf("\r\n=== FlexibleButton Test Start ===\r\n");
    UART_Debug_Printf("Press the button to see events.\r\n");
    UART_Debug_Printf("Try: Single Click, Double Click, Long Press\r\n");

    // Initialize the library and hardware
    FlexibleButton_Init();

    // Loop
    uint32_t last_scan = 0;
    
    while(1) {
        uint32_t now = HAL_GetTick();
        
        if (now - last_scan >= SCAN_INTERVAL_MS) {
            last_scan = now;
            
            // Core scan function
            FlexibleButton_Scan();
        }
    }
}
