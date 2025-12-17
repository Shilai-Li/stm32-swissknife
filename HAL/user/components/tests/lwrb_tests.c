/**
 * @file lwrb_tests.c
 * @brief lwrb (Lightweight Ring Buffer) component test suite
 * 
 * This test demonstrates lwrb usage for buffering and data management:
 * - Basic read/write operations
 * - Peek and skip operations
 * - DMA-optimized linear block access
 * - Overflow handling
 * - Performance test
 * 
 * CubeMX Configuration:
 * ====================
 * Only UART2 is needed for debug output (see uart.h).
 * 
 * How to Test:
 * ============
 * 1. Connect UART2 to PC via USB-Serial adapter
 * 2. Open serial terminal (115200 baud)
 * 3. Observe test results
 */

#include "lwrb.h"
#include "lwrb_port.h"
#include "uart.h"
#include "stm32f1xx_hal.h"
#include <stdio.h>
#include <string.h>

/* ============================================================================
 * Test Configuration
 * ========================================================================= */

#define BUFFER_SIZE  256

/* ============================================================================
 * Test Helpers
 * ========================================================================= */

static uint32_t test_passed = 0;
static uint32_t test_failed = 0;

#define TEST_ASSERT(condition, msg) \
    do { \
        if (condition) { \
            test_passed++; \
            UART_Debug_Printf("  [PASS] %s\r\n", msg); \
        } else { \
            test_failed++; \
            UART_Debug_Printf("  [FAIL] %s\r\n", msg); \
        } \
    } while (0)

/* ============================================================================
 * Test Cases
 * ========================================================================= */

/**
 * @brief Test 1: Basic initialization
 */
void test_init(void)
{
    uint8_t buffer_data[BUFFER_SIZE];
    lwrb_t buff;
    
    UART_Debug_Printf("\r\n[Test 1] Initialization\r\n");
    
    uint8_t result = lwrb_init(&buff, buffer_data, sizeof(buffer_data));
    TEST_ASSERT(result == 1, "Buffer initialization");
    TEST_ASSERT(lwrb_get_free(&buff) == (BUFFER_SIZE - 1), "Initial free space");
    TEST_ASSERT(lwrb_get_full(&buff) == 0, "Initial full space");
}

/**
 * @brief Test 2: Basic write and read
 */
void test_write_read(void)
{
    uint8_t buffer_data[BUFFER_SIZE];
    lwrb_t buff;
    
    UART_Debug_Printf("\r\n[Test 2] Write and Read\r\n");
    
    lwrb_init(&buff, buffer_data, sizeof(buffer_data));
    
    // Write test data
    const char *test_str = "Hello, lwrb!";
    size_t written = lwrb_write(&buff, test_str, strlen(test_str));
    TEST_ASSERT(written == strlen(test_str), "Write data");
    TEST_ASSERT(lwrb_get_full(&buff) == strlen(test_str), "Data available after write");
    
    // Read test data
    char read_buf[32] = {0};
    size_t read = lwrb_read(&buff, read_buf, sizeof(read_buf));
    TEST_ASSERT(read == strlen(test_str), "Read data");
    TEST_ASSERT(strcmp(read_buf, test_str) == 0, "Data integrity");
    TEST_ASSERT(lwrb_get_full(&buff) == 0, "Buffer empty after read");
}

/**
 * @brief Test 3: Wrap-around behavior
 */
void test_wraparound(void)
{
    uint8_t buffer_data[16];  // Small buffer to test wrap-around
    lwrb_t buff;
    
    UART_Debug_Printf("\r\n[Test 3] Wrap-around\r\n");
    
    lwrb_init(&buff, buffer_data, sizeof(buffer_data));
    
    // Fill buffer
    uint8_t data1[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    lwrb_write(&buff, data1, sizeof(data1));
    
    // Read half
    uint8_t out1[5];
    lwrb_read(&buff, out1, sizeof(out1));
    
    // Write more (will cause wrap-around)
    uint8_t data2[10] = {10, 11, 12, 13, 14, 15, 16, 17, 18, 19};
    size_t written = lwrb_write(&buff, data2, sizeof(data2));
    TEST_ASSERT(written == 10, "Write after wrap-around");
    
    // Read all
    uint8_t out2[20];
    size_t read = lwrb_read(&buff, out2, sizeof(out2));
    TEST_ASSERT(read == 15, "Read wrap-around data");
    
    // Verify data integrity
    bool integrity_ok = true;
    for (size_t i = 0; i < 5; i++) {
        if (out2[i] != (5 + i)) integrity_ok = false;
    }
    for (size_t i = 0; i < 10; i++) {
        if (out2[5 + i] != (10 + i)) integrity_ok = false;
    }
    TEST_ASSERT(integrity_ok, "Wrap-around data integrity");
}

/**
 * @brief Test 4: Peek and skip
 */
void test_peek_skip(void)
{
    uint8_t buffer_data[BUFFER_SIZE];
    lwrb_t buff;
    
    UART_Debug_Printf("\r\n[Test 4] Peek and Skip\r\n");
    
    lwrb_init(&buff, buffer_data, sizeof(buffer_data));
    
    // Write test data
    const char *test_str = "ABCDEFGH";
    lwrb_write(&buff, test_str, strlen(test_str));
    
    // Peek without removing
    char peek_buf[4] = {0};
    size_t peeked = lwrb_peek(&buff, 0, peek_buf, sizeof(peek_buf));
    TEST_ASSERT(peeked == 4, "Peek data");
    TEST_ASSERT(strncmp(peek_buf, "ABCD", 4) == 0, "Peek integrity");
    TEST_ASSERT(lwrb_get_full(&buff) == strlen(test_str), "Buffer unchanged after peek");
    
    // Skip some bytes
    size_t skipped = lwrb_skip(&buff, 2);
    TEST_ASSERT(skipped == 2, "Skip bytes");
    TEST_ASSERT(lwrb_get_full(&buff) == (strlen(test_str) - 2), "Buffer size after skip");
    
    // Read remaining
    char read_buf[10] = {0};
    lwrb_read(&buff, read_buf, sizeof(read_buf));
    TEST_ASSERT(strcmp(read_buf, "CDEFGH") == 0, "Read after skip");
}

/**
 * @brief Test 5: Overflow handling
 */
void test_overflow(void)
{
    uint8_t buffer_data[16];
    lwrb_t buff;
    
    UART_Debug_Printf("\r\n[Test 5] Overflow Handling\r\n");
    
    lwrb_init(&buff, buffer_data, sizeof(buffer_data));
    
    // Try to write more than buffer can hold
    uint8_t large_data[20];
    memset(large_data, 0xAA, sizeof(large_data));
    
    size_t written = lwrb_write(&buff, large_data, sizeof(large_data));
    TEST_ASSERT(written == (sizeof(buffer_data) - 1), "Overflow prevention");
    TEST_ASSERT(lwrb_get_free(&buff) == 0, "Buffer full");
    
    // Try to write to full buffer
    uint8_t extra_data[1] = {0xBB};
    written = lwrb_write(&buff, extra_data, sizeof(extra_data));
    TEST_ASSERT(written == 0, "Write to full buffer rejected");
}

/**
 * @brief Test 6: DMA-optimized linear block access
 */
void test_linear_block(void)
{
    uint8_t buffer_data[16];
    lwrb_t buff;
    
    UART_Debug_Printf("\r\n[Test 6] Linear Block Access (DMA)\r\n");
    
    lwrb_init(&buff, buffer_data, sizeof(buffer_data));
    
    // Write some data
    uint8_t data[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    lwrb_write(&buff, data, sizeof(data));
    
    // Get linear read length
    size_t linear_read = lwrb_get_linear_block_read_length(&buff);
    TEST_ASSERT(linear_read > 0, "Linear block read length");
    UART_Debug_Printf("  Info: Linear read block = %d bytes\r\n", linear_read);
    
    // Skip the linear block (simulating DMA read)
    lwrb_skip(&buff, linear_read);
    
    size_t remaining = lwrb_get_full(&buff);
    UART_Debug_Printf("  Info: Remaining after skip = %d bytes\r\n", remaining);
    TEST_ASSERT(remaining == (10 - linear_read), "Correct remaining data");
}

/**
 * @brief Test 7: Performance test
 */
void test_performance(void)
{
    uint8_t buffer_data[BUFFER_SIZE];
    lwrb_t buff;
    
    UART_Debug_Printf("\r\n[Test 7] Performance Test\r\n");
    
    lwrb_init(&buff, buffer_data, sizeof(buffer_data));
    
    uint32_t iterations = 10000;
    uint8_t test_byte = 0xAA;
    
    uint32_t start = HAL_GetTick();
    
    for (uint32_t i = 0; i < iterations; i++) {
        lwrb_write(&buff, &test_byte, 1);
        uint8_t out;
        lwrb_read(&buff, &out, 1);
    }
    
    uint32_t elapsed = HAL_GetTick() - start;
    
    UART_Debug_Printf("  Info: %lu iterations in %lu ms\r\n", iterations, elapsed);
    UART_Debug_Printf("  Info: ~%lu operations/second\r\n", 
                     elapsed > 0 ? (iterations * 2 * 1000 / elapsed) : 0);
    
    TEST_ASSERT(elapsed < 1000, "Performance acceptable");
}

/* ============================================================================
 * Test Entry Point
 * ========================================================================= */

void User_Entry(void)
{
    // Initialize UART
    UART_Init();
    HAL_Delay(100);
    
    UART_Debug_Printf("\r\n");
    UART_Debug_Printf("================================\r\n");
    UART_Debug_Printf("  lwrb Test Suite\r\n");
    UART_Debug_Printf("================================\r\n");
    UART_Debug_Printf("Lightweight Ring Buffer Library\r\n");
    UART_Debug_Printf("Test Buffer Size: %d bytes\r\n", BUFFER_SIZE);
    UART_Debug_Printf("\r\n");
    
    // Run all tests
    test_init();
    test_write_read();
    test_wraparound();
    test_peek_skip();
    test_overflow();
    test_linear_block();
    test_performance();
    
    // Print summary
    UART_Debug_Printf("\r\n");
    UART_Debug_Printf("================================\r\n");
    UART_Debug_Printf("  Test Summary\r\n");
    UART_Debug_Printf("================================\r\n");
    UART_Debug_Printf("PASSED: %lu\r\n", test_passed);
    UART_Debug_Printf("FAILED: %lu\r\n", test_failed);
    UART_Debug_Printf("TOTAL:  %lu\r\n", test_passed + test_failed);
    UART_Debug_Printf("\r\n");
    
    if (test_failed == 0) {
        UART_Debug_Printf("Result: ALL TESTS PASSED! ✓\r\n");
    } else {
        UART_Debug_Printf("Result: SOME TESTS FAILED! ✗\r\n");
    }
    
    UART_Debug_Printf("================================\r\n");
    
    // Infinite loop
    while (1) {
        UART_Poll();
        HAL_Delay(100);
    }
}
