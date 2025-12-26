/**
 * @file key_tests.c
 * @brief Test for Key Driver
 */

#include "io/key.h"
#include "uart.h"
#include "usb_cdc.h" // For build consistency (usbd_cdc_if)

#define CH_DEBUG 2

void user_main(void) {
    UART_Register(CH_DEBUG, &huart2);
    // UART_Init(); // Deprecated
    
    // F446 Nucleo User Button (PC13 - Active Low? Wait, Nucleo User Button is usually PC13, Active LOW)
    // Actually, on Nucleo-64:
    // B1 USER: PC13.
    // Pressed = LOW ?  Let's check schematic usually.
    // Nucleo Manual: "User button (B1) connected to PC13... When the button is pressed the I/O is LOW (if R30 soldered) or HIGH?"
    // Checking Schematic: PC13 has external Pull-Up? No, it has a Capacitor to GND. 
    // Button connects PC13 to GND? Wait, B1 is usually Blue Button.
    // 
    // Manual says: "PC13 is LOW when pressed" (SB17 ON). 
    // Let's assume Active LOW.
    
    Key_Register(0, GPIOC, GPIO_PIN_13, KEY_ACTIVE_LOW); 
    
    UART_SendString(CH_DEBUG, "\r\n=== Key Driver Test ===\r\n");
    UART_SendString(CH_DEBUG, "Press User Button (PC13)...\r\n");
    
    while(1) {
        // Must assume we are calling this frequently for polling
        Key_Scan();
        
        // Check Real-time State
        /*
        if (Key_GetState(0) == KEY_STATE_PRESSED) {
            // UART_SendString(CH_DEBUG, "."); // Spammy
        }
        */
        
        // Check Events
        KeyEvent_t evt = Key_GetEvent(0);
        switch(evt) {
            case KEY_EVENT_PRESS:
                UART_SendString(CH_DEBUG, "[Event] PRESS\r\n");
                break;
            case KEY_EVENT_CLICK:
                UART_SendString(CH_DEBUG, "[Event] CLICK (Release)\r\n");
                break;
            case KEY_EVENT_LONG_PRESS:
                UART_SendString(CH_DEBUG, "[Event] LONG PRESS DETECTED!\r\n");
                break;
            case KEY_EVENT_LONG_RELEASE:
                UART_SendString(CH_DEBUG, "[Event] Long Press Released\r\n");
                break;
            default:
                break;
        }
        
        HAL_Delay(10); // Scan interval
    }
}
