/**
 * @file usb_serial_tests.c
 * @brief Test for USB Serial
 * @details This test requires USB cable connected to PC
 */

#include "usb_serial.h"
#include "delay.h"

void Test_USB_Serial_Entry(void) {
    // Note: USB Init is handled by MX_USB_DEVICE_Init() in main.c
    // We just init our soft buffer
    USB_Serial_Init();
    
    // Wait for enumeration? Usually takes 1-2 sec for PC to detect.
    Delay_ms(2000);
    
    USB_Serial_Printf("\r\n=== USB Serial Test Start ===\r\n");
    USB_Serial_Printf("Please type characters in your Serial Terminal...\r\n");
    
    while(1) {
        // Echo Back
        if (USB_Serial_Available() > 0) {
            uint8_t c;
            if (USB_Serial_Read(&c)) {
                USB_Serial_Printf("Echo: %c\r\n", c);
            }
        }
        
        // Optional heartbeat
        static uint32_t last = 0;
        if (millis() - last > 1000) {
            last = millis();
            // USB_Serial_Printf("Ping\r\n");
        }
    }
}
