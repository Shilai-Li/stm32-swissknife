#include "rtc_driver.h"
#include "uart.h"
#include "usb_cdc.h"
#include "rtc.h"  // For hrtc
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

// Test configuration
#define CH_DEBUG 2
#define TEST_ITERATIONS 10

// Forward declaration for callback
static void TestAlarmCallback(void);
static volatile bool callback_triggered = false;

// Helper to print to both UART and USB CDC
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

// Callback implementation (must be outside main function)
static void TestAlarmCallback(void) {
    callback_triggered = true;
}

void app_main(void) {
    // Initialize buffers for UART
    static uint8_t rx_dma_buf[64];
    static uint8_t rx_ring_buf[256];
    static uint8_t tx_ring_buf[512];
    
    // Register UART
    extern UART_HandleTypeDef huart2;
    UART_Register(CH_DEBUG, &huart2, 
                  rx_dma_buf, sizeof(rx_dma_buf),
                  rx_ring_buf, sizeof(rx_ring_buf),
                  tx_ring_buf, sizeof(tx_ring_buf));
    
    // Initialize USB CDC
    USB_CDC_Init();
    
    HAL_Delay(100); // Wait for USB enumeration
    
    Test_Printf("\r\n===================================\r\n");
    Test_Printf("     RTC Driver Test Suite       \r\n");
    Test_Printf("===================================\r\n\r\n");
    
    // ========================================================================
    // Test 1: Registration and Status Check
    // ========================================================================
    Test_Printf("--- Test 1: RTC Registration ---\r\n");
    RTC_Register(&hrtc);
    
    if (RTC_IsReady()) {
        Test_Printf("RTC Status: READY [OK]\r\n");
    } else {
        Test_Printf("RTC Status: NOT READY [FAIL]\r\n");
        Test_Printf("ERROR: RTC not initialized. Check CubeMX config!\r\n");
        Test_Printf("=== Test FAILED ===\r\n");
        while(1) { HAL_Delay(1000); }
    }
    
    // ========================================================================
    // Test 2: Set Time
    // ========================================================================
    Test_Printf("\r\n--- Test 2: Set RTC Time ---\r\n");
    
    // Set to 2025-01-01 00:00:00 UTC (Unix: 1735689600)
    uint32_t test_timestamp = 1735689600;
    char time_str[20];
    
    Test_Printf("Setting Time: 2025-01-01 00:00:00 (Unix: %u)\r\n", test_timestamp);
    
    if (RTC_SetTimeUnix(test_timestamp)) {
        Test_Printf("Time Set: SUCCESS [OK]\r\n");
    } else {
        Test_Printf("Time Set: FAILED [FAIL]\r\n");
        Test_Printf("=== Test FAILED ===\r\n");
        while(1) { HAL_Delay(1000); }
    }
    
    // ========================================================================
    // Test 3: Read Time and Verify Increment
    // ========================================================================
    Test_Printf("\r\n--- Test 3: Time Increment Test ---\r\n");
    Test_Printf("Reading time every 1s for %d iterations...\r\n\r\n", TEST_ITERATIONS);
    
    uint32_t last_time = test_timestamp;
    bool all_passed = true;
    
    for (int i = 1; i <= TEST_ITERATIONS; i++) {
        HAL_Delay(1000);
        UART_Poll(); // Keep UART alive
        
        uint32_t now = RTC_GetTimeUnix();
        RTC_GetTimeString(time_str);
        
        int32_t delta = (int32_t)(now - last_time);
        const char *status = (delta == 1) ? "[OK]" : "[FAIL]";
        
        Test_Printf("[%2d] Unix: %10u | Time: %s | Delta: %+2ds %s\r\n", 
                    i, now, time_str, delta, status);
        
        if (delta != 1) {
            all_passed = false;
        }
        
        last_time = now;
    }
    
    // ========================================================================
    // Test 4: Time String Formatting
    // ========================================================================
    Test_Printf("\r\n--- Test 4: String Formatting ---\r\n");
    
    RTC_GetTimeString(time_str);
    Test_Printf("Current Time String: %s\r\n", time_str);
    
    // Verify format length (should be 19 chars: "YYYY-MM-DD HH:MM:SS")
    int len = strlen(time_str);
    if (len == 19) {
        Test_Printf("String Length: %d chars [OK]\r\n", len);
    } else {
        Test_Printf("String Length: %d chars (expected 19) [FAIL]\r\n", len);
        all_passed = false;
    }
    
    // ========================================================================
    // Test 5: Callback Registration (No Trigger Test)
    // ========================================================================
    Test_Printf("\r\n--- Test 5: Callback Registration ---\r\n");
    
    RTC_SetAlarmCallback(TestAlarmCallback);
    Test_Printf("Alarm Callback Registered [OK]\r\n");
    Test_Printf("(Note: Alarm functionality not yet implemented)\r\n");
    
    // ========================================================================
    // Test Summary
    // ========================================================================
    Test_Printf("\r\n===================================\r\n");
    if (all_passed) {
        Test_Printf("=== All Tests PASSED ===\r\n");
    } else {
        Test_Printf("=== Some Tests FAILED ===\r\n");
    }
    Test_Printf("===================================\r\n\r\n");
    
    // ========================================================================
    // Continuous Monitoring Loop
    // ========================================================================
    Test_Printf("Entering continuous time monitoring...\r\n");
    Test_Printf("Press any key to see current time.\r\n\r\n");
    
    while (1) {
        UART_Poll();
        
        uint8_t cmd;
        if (UART_Read(CH_DEBUG, &cmd) || USB_CDC_Read(&cmd)) {
            uint32_t now = RTC_GetTimeUnix();
            RTC_GetTimeString(time_str);
            Test_Printf("[Live] Unix: %u | Time: %s\r\n", now, time_str);
        }
        
        HAL_Delay(100);
    }
}
