/**
 * @file watchdog_tests.c
 * @brief Watchdog Test
 * @warning This test WILL RESET YOUR BOARD if successful.
 */

#include "watchdog.h"
#include "uart.h"

void user_main(void) {
    UART_Init();
    
    if (Watchdog_WasResetByDog()) {
        UART_Debug_Printf("\r\n=== System Rebooted by Watchdog! (Test Success) ===\r\n");
        UART_Debug_Printf("Running normally now. I won't enable watchdog again to prevent loop.\r\n");
        // Clear flag happened in Check function
        
        // Blink fast to indicate success state
        // assuming LED? 
        return; 
    }
    
    UART_Debug_Printf("\r\n=== Watchdog Test Start ===\r\n");
    UART_Debug_Printf("In 5 seconds, I will stop feeding the dog. Expect a reset.\r\n");

    // Initialize Watchdog for 2 seconds timeout
    if (!Watchdog_Init(2000)) {
        UART_Debug_Printf("Error: Time value too large?\r\n");
        return;
    }

    // Feed for 5 seconds
    for (int i=0; i<10; i++) {
        UART_Debug_Printf("Feeding Dog... %d/10\r\n", i+1);
        Watchdog_Feed();
        
        // Toggle LED if possible
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13); 
        
        HAL_Delay(500);
    }
    
    UART_Debug_Printf("I am STOPPING feeding now. Bye bye!\r\n");
    UART_Debug_Printf("System should reset in ~2 seconds...\r\n");
    
    while(1) {
        // Do nothing, let it bite
        HAL_Delay(100);
        UART_Debug_Printf(".");
    }
}
