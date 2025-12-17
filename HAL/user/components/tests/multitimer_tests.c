/**
 * @file multitimer_tests.c
 * @brief Test application for MultiTimer component
 */
#include "multitimer.h"
#include "uart.h"
#include "delay.h"
#include <stdio.h>
#include <string.h>

// Timer handles
static MultiTimer timer1;
static MultiTimer timer2;
static MultiTimer timer3;

// Callbacks
static void Timer1_Callback(MultiTimer* timer, void* userData) {
    const char* str = (const char*)userData;
    UART_Send((uint8_t*)str, strlen(str));
    // Example: Toggle LED1
}

static void Timer2_Callback(MultiTimer* timer, void* userData) {
    static int count = 0;
    char buf[64];
    snprintf(buf, sizeof(buf), "[Timer2] Count: %d\r\n", count++);
    UART_Send((uint8_t*)buf, strlen(buf));
}

static void Timer3_OneShot_Callback(MultiTimer* timer, void* userData) {
    UART_Send((uint8_t*)"[Timer3] One-shot fired! Stopping Timer 1.\r\n", 43);
    MultiTimerStop(&timer1);
}

// Entry point
void User_Entry(void) {
    // Init system drivers
    Delay_Init();
    // Assuming UART is initialized in main before User_Entry or we rely on default settings
    // If we need to init UART:
    // UART_Init(115200); 
    
    UART_Send((uint8_t*)"\r\n=== MultiTimer Test ===\r\n", 25);

    // Init Platform reference (if needed)
    MultiTimer_Platform_Init();

    // Init Timers
    // Timer 1: 500ms period
    MultiTimerInit(&timer1, 500, Timer1_Callback, "[Timer1] 500ms tick\r\n");
    
    // Timer 2: 1000ms period
    MultiTimerInit(&timer2, 1000, Timer2_Callback, NULL);
    
    // Timer 3: 5000ms, one-shot (period=0)
    MultiTimerInit(&timer3, 0, Timer3_OneShot_Callback, NULL);

    // Start Timers
    uint32_t now = MultiTimerTicks();
    MultiTimerStart(&timer1, now, 500);
    MultiTimerStart(&timer2, now, 1000);
    MultiTimerStart(&timer3, now, 5000); // 5 seconds later

    UART_Send((uint8_t*)"Timers started. Enter loop.\r\n", 29);

    while (1) {
        MultiTimerYield();
        
        // Simulate some background work or sleep could be added here
        // But do not block for long!
        // Delay_ms(1); // Optional, to save power? But MultiTimerYield needs frequent calls.
    }
}
