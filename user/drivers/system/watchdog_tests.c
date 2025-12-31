/**
 * @file watchdog_tests.c
 * @brief Test Suite for Watchdog Driver (IWDG or WWDG)
 */

#include "watchdog.h"
#include "uart.h"
#include "usb_cdc.h"
#include <stdio.h>
#include <stdarg.h>

#define CH_DEBUG 2
#define FEED_COUNT 10

static void Test_Printf(const char *fmt, ...) {
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

void app_main(void) {
    static uint8_t rx_dma_buf[64];
    static uint8_t rx_ring_buf[256];
    static uint8_t tx_ring_buf[512];
    
    extern UART_HandleTypeDef huart2;
    UART_Register(CH_DEBUG, &huart2, 
                  rx_dma_buf, sizeof(rx_dma_buf),
                  rx_ring_buf, sizeof(rx_ring_buf),
                  tx_ring_buf, sizeof(tx_ring_buf));
    
    USB_CDC_Init();
    HAL_Delay(200);
    
    Test_Printf("\r\n===================================\r\n");
    Test_Printf("   Watchdog Driver Test Suite    \r\n");
    Test_Printf("===================================\r\n\r\n");

#if defined(HAL_IWDG_MODULE_ENABLED)
    Test_Printf("Mode: IWDG (Independent Watchdog)\r\n\r\n");
    
    if (Watchdog_WasResetByDog()) {
        Test_Printf("Last Reset: IWDG TIMEOUT [OK]\r\n");
        Test_Printf("=== Test PASSED ===\r\n");
        while(1) { UART_Poll(); HAL_Delay(1000); Test_Printf("."); }
    }
    
    extern IWDG_HandleTypeDef hiwdg;
    Watchdog_Register(&hiwdg);
    
    if (Watchdog_Init(2000)) {
        Test_Printf("IWDG Init: 2000ms [OK]\r\n");
    } else {
        Test_Printf("IWDG Init: FAILED\r\n");
        while(1) { HAL_Delay(1000); }
    }
    
    for (int i = 1; i <= FEED_COUNT; i++) {
        HAL_Delay(1000);
        UART_Poll();
        Watchdog_Feed();
        Test_Printf("[Feed %d/%d] [OK]\r\n", i, FEED_COUNT);
    }
    
    Test_Printf("\r\nStopping feed... reset in ~2s\r\n");
    while(1) { UART_Poll(); }

#elif defined(HAL_WWDG_MODULE_ENABLED)
    Test_Printf("Mode: WWDG (Window Watchdog)\r\n\r\n");
    
    if (Watchdog_WasResetByDog()) {
        Test_Printf("Last Reset: WWDG TIMEOUT [OK]\r\n");
        Test_Printf("=== Test PASSED ===\r\n");
        while(1) { UART_Poll(); HAL_Delay(1000); Test_Printf("."); }
    }
    
    extern WWDG_HandleTypeDef hwwdg;
    Watchdog_Register(&hwwdg);
    
    Test_Printf("WWDG Registered [OK]\r\n");
    Test_Printf("Note: WWDG timing configured in CubeMX\r\n\r\n");
    
    // WWDG needs frequent feeding (typically < 50ms window)
    Test_Printf("Feeding WWDG for 5 seconds...\r\n");
    uint32_t start = HAL_GetTick();
    int feed_count = 0;
    
    while ((HAL_GetTick() - start) < 5000) {
        Watchdog_Feed();
        feed_count++;
        HAL_Delay(10); // Feed every 10ms (within window)
        UART_Poll();
    }
    
    Test_Printf("Fed %d times in 5s [OK]\r\n", feed_count);
    Test_Printf("\r\nStopping feed... reset soon\r\n");
    while(1) { UART_Poll(); }

#else
    Test_Printf("ERROR: No watchdog enabled!\r\n");
    Test_Printf("Enable IWDG or WWDG in CubeMX\r\n");
    while(1) { UART_Poll(); HAL_Delay(1000); }
#endif
}
