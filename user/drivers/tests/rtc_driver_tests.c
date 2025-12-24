/**
 * @file rtc_driver_tests.c
 * @brief Test for RTC Driver
 */

#include "rtc_driver.h"
#include "uart.h"
#include "delay.h" // For delay
#include <stdio.h>

void user_main(void) {
    UART_Init();
    UART_Debug_Printf("\r\n=== RTC Test Start ===\r\n");
    
    // Check Readiness
    // Note: Depends on if user enabled RTC in CubeMX
    // If not, this might hang or fail implicitly in driver stub
    
    // 1. Set Time to a known date: 2025-01-01 00:00:00 UTC
    // Timestamp: 1735689600
    uint32_t test_time = 1735689600;
    
    UART_Debug_Printf("Setting Time to Unix: %u (2025-01-01)\r\n", test_time);
    RTC_SetTimeUnix(test_time);
    
    // 2. Read Loop
    char buf[32];
    for (int i=0; i<5; i++) {
        HAL_Delay(1000);
        uint32_t now = RTC_GetTimeUnix();
        RTC_GetTimeString(buf);
        
        UART_Debug_Printf("Time: %u | String: %s | Diff: %d s\r\n", 
                          now, buf, (int)(now - test_time));
    }
    
    UART_Debug_Printf("=== RTC Test Done ===\r\n");
}
