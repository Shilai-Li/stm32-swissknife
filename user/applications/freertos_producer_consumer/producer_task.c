#include "app_main.h"
#include "main.h"   // HAL & Board Definitions
#include "uart.h"   // UART Driver
#include "usb_cdc.h" // USB Driver
#include <stdio.h>

// Helper: Sends string to both UART and USB CDC
void Log_String(const char *str) {
    if (!str) return;
    UART_SendString(CH_DEBUG, str);
    USB_CDC_SendString(str);
}

// Task 1: Serial Producer
// Reads from UART/USB and sends commands to the queue
void SerialProducerTask(void *pvParameters)
{
    uint8_t rx_data;
    char msg[64];

    vTaskDelay(2000); // Wait for system stabilization
    Log_String("[Producer] Started. Send '1', '2', '3' to change speed.\r\n");

    for (;;)
    {
        bool has_data = false;

        // Poll UART
        if (UART_Receive(CH_DEBUG, &rx_data, 10)) {
            has_data = true;
        } 
        // Poll USB CDC
        else if (USB_CDC_Receive(&rx_data, 10)) {
             has_data = true;
        }

        if (has_data) {
            // Send to Queue
            if (xQueueSend(xCmdQueue, &rx_data, 10) == pdTRUE) {
                sprintf(msg, "[Producer] Sent cmd: %c\r\n", rx_data);
                Log_String(msg);
            } else {
                Log_String("[Producer] Queue Full!\r\n");
            }
        } 
        else {
            // Yield
            vTaskDelay(10); 
        }
    }
}
