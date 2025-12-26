/**
 * @file delay_tests.c
 * @brief Comprehensive Test Suite for Delay Driver
 */

#include "delay.h"
#include "uart.h"
#include "usb_cdc.h"
#include <stdio.h>
#include <stdarg.h>
#include <math.h>

// Test configuration
#define CH_DEBUG 2

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

// Helper to calculate error percentage
static float CalcError(uint32_t target, uint32_t actual) {
    return ((float)actual - (float)target) / (float)target * 100.0f;
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
    Test_Printf("    Delay Driver Test Suite      \r\n");
    Test_Printf("===================================\r\n\r\n");
    
    // ========================================================================
    // Test 1: DWT Initialization
    // ========================================================================
    Test_Printf("--- Test 1: DWT Initialization ---\r\n");
    
    Delay_Init();
    
    uint32_t cpu_freq = HAL_RCC_GetHCLKFreq();
    Test_Printf("CPU Frequency: %lu Hz (%.1f MHz) [OK]\r\n", 
                cpu_freq, (float)cpu_freq / 1e6f);
    
    // Check if DWT is enabled
    if (DWT->CTRL & DWT_CTRL_CYCCNTENA_Msk) {
        Test_Printf("DWT Status: ENABLED [OK]\r\n");
    } else {
        Test_Printf("DWT Status: DISABLED [FAIL]\r\n");
        Test_Printf("ERROR: DWT not enabled!\r\n");
        while(1) { HAL_Delay(1000); }
    }
    
    // Calculate overflow period
    float overflow_period = (float)0xFFFFFFFF / (float)cpu_freq;
    Test_Printf("micros() Overflow Period: %.2f seconds\r\n", overflow_period);
    
    // ========================================================================
    // Test 2: Microsecond Delays
    // ========================================================================
    Test_Printf("\r\n--- Test 2: Microsecond Delays ---\r\n");
    
    struct {
        uint32_t target_us;
        float tolerance_percent;
    } us_tests[] = {
        {10,   10.0f},  // +/-10% for very short delays
        {50,    5.0f},  // +/-5%
        {100,   2.0f},  // +/-2%
        {500,   1.0f},  // +/-1%
        {1000,  0.5f},  // +/-0.5%
        {5000,  0.5f},  // +/-0.5%
    };
    
    bool all_us_passed = true;
    
    for (int i = 0; i < sizeof(us_tests) / sizeof(us_tests[0]); i++) {
        uint32_t target = us_tests[i].target_us;
        
        uint32_t start = micros();
        Delay_us(target);
        uint32_t end = micros();
        
        uint32_t actual = end - start;
        float error = CalcError(target, actual);
        
        const char *status = (fabsf(error) <= us_tests[i].tolerance_percent) ? "[OK]" : "[FAIL]";
        
        Test_Printf("Delay_us(%5lu): Actual: %5lu us  (Error: %+.1f%%) %s\r\n", 
                    target, actual, error, status);
        
        if (fabsf(error) > us_tests[i].tolerance_percent) {
            all_us_passed = false;
        }
    }
    
    // ========================================================================
    // Test 3: Millisecond Delays
    // ========================================================================
    Test_Printf("\r\n--- Test 3: Millisecond Delays ---\r\n");
    
    struct {
        uint32_t target_ms;
        float tolerance_percent;
    } ms_tests[] = {
        {10,   15.0f},  // +/-15% (HAL_Delay has +/-1 tick margin)
        {50,    5.0f},  // +/-5%
        {100,   2.0f},  // +/-2%
    };
    
    bool all_ms_passed = true;
    
    for (int i = 0; i < sizeof(ms_tests) / sizeof(ms_tests[0]); i++) {
        uint32_t target = ms_tests[i].target_ms;
        
        uint32_t start = HAL_GetTick();
        Delay_ms(target);
        uint32_t end = HAL_GetTick();
        
        uint32_t actual = end - start;
        float error = CalcError(target, actual);
        
        const char *status = (fabsf(error) <= ms_tests[i].tolerance_percent) ? "[OK]" : "[FAIL]";
        
        Test_Printf("Delay_ms(%3lu):  Actual: %3lu ms   (Error: %+.1f%%) %s\r\n", 
                    target, actual, error, status);
        
        if (fabsf(error) > ms_tests[i].tolerance_percent) {
            all_ms_passed = false;
        }
    }
    
    // ========================================================================
    // Test 4: Timestamp Functions
    // ========================================================================
    Test_Printf("\r\n--- Test 4: Timestamp Functions ---\r\n");
    
    uint32_t micros_start = micros();
    uint32_t millis_start = millis();
    
    HAL_Delay(100);
    
    uint32_t micros_end = micros();
    uint32_t millis_end = millis();
    
    uint32_t micros_elapsed = micros_end - micros_start;
    uint32_t millis_elapsed = millis_end - millis_start;
    
    Test_Printf("micros() delta: %lu us (expected ~100000) ", micros_elapsed);
    if (micros_elapsed > 95000 && micros_elapsed < 105000) {
        Test_Printf("[OK]\r\n");
    } else {
        Test_Printf("[FAIL]\r\n");
        all_ms_passed = false;
    }
    
    Test_Printf("millis() delta: %lu ms (expected ~100) ", millis_elapsed);
    if (millis_elapsed >= 99 && millis_elapsed <= 101) {
        Test_Printf("[OK]\r\n");
    } else {
        Test_Printf("[FAIL]\r\n");
        all_ms_passed = false;
    }
    
    // ========================================================================
    // Test 5: Performance Benchmark
    // ========================================================================
    Test_Printf("\r\n--- Test 5: Performance Benchmark ---\r\n");
    
    const uint32_t iterations = 10000;
    
    uint32_t bench_start = micros();
    for (uint32_t i = 0; i < iterations; i++) {
        Delay_us(1);
    }
    uint32_t bench_end = micros();
    
    uint32_t total_time = bench_end - bench_start;
    float avg_time = (float)total_time / (float)iterations;
    
    Test_Printf("Delay_us(1) x %lu iterations:\r\n", iterations);
    Test_Printf("  Total Time: %lu us\r\n", total_time);
    Test_Printf("  Average: %.2f us/call\r\n", avg_time);
    Test_Printf("  Overhead: ~%.2f us\r\n", avg_time - 1.0f);
    
    // ========================================================================
    // Test Summary
    // ========================================================================
    Test_Printf("\r\n===================================\r\n");
    if (all_us_passed && all_ms_passed) {
        Test_Printf("=== All Tests PASSED ===\r\n");
    } else {
        Test_Printf("=== Some Tests FAILED ===\r\n");
    }
    Test_Printf("===================================\r\n\r\n");
    
    // ========================================================================
    // Interactive Delay Test
    // ========================================================================
    Test_Printf("Entering interactive test mode...\r\n");
    Test_Printf("Send commands:\r\n");
    Test_Printf("  '1' = Delay_us(100)\r\n");
    Test_Printf("  '2' = Delay_ms(100)\r\n");
    Test_Printf("  't' = Show current timestamps\r\n\r\n");
    
    while (1) {
        UART_Poll();
        
        uint8_t cmd;
        if (UART_Read(CH_DEBUG, &cmd) || USB_CDC_Read(&cmd)) {
            switch (cmd) {
                case '1': {
                    uint32_t start = micros();
                    Delay_us(100);
                    uint32_t end = micros();
                    Test_Printf("[Delay_us(100)] Actual: %lu us\r\n", end - start);
                    break;
                }
                
                case '2': {
                    uint32_t start = millis();
                    Delay_ms(100);
                    uint32_t end = millis();
                    Test_Printf("[Delay_ms(100)] Actual: %lu ms\r\n", end - start);
                    break;
                }
                
                case 't': {
                    Test_Printf("[Timestamp] micros: %lu | millis: %lu\r\n", 
                                micros(), millis());
                    break;
                }
                
                default:
                    Test_Printf("Unknown command: '%c'\r\n", cmd);
                    break;
            }
        }
        
        HAL_Delay(10);
    }
}
