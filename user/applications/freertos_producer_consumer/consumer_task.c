#include "app_main.h"
#include "led.h"    // LED Driver
#include <stdio.h>

// Task 2: LED Consumer
// Reads from queue to update delay, and handles blinking
void LEDConsumerTask(void *pvParameters)
{
    uint32_t blink_delay = 500; // Default 500ms
    uint8_t cmd;
    char msg[64];

    vTaskDelay(2000); // Wait for system stabilization
    Log_String("[Consumer] Started. Default delay: 500ms\r\n");

    for (;;)
    {
        // Try to receive from queue (Non-blocking check)
        if (xQueueReceive(xCmdQueue, &cmd, 0) == pdTRUE) {
            switch (cmd) {
                case '1':
                    blink_delay = 100; // Fast
                    break;
                case '2':
                    blink_delay = 500; // Medium
                    break;
                case '3':
                    blink_delay = 1000; // Slow
                    break;
                default:
                    Log_String("[Consumer] Unknown Command\r\n");
                    break;
            }
            
            if (cmd == '1' || cmd == '2' || cmd == '3') {
                sprintf(msg, "[Consumer] Speed updated to %lu ms\r\n", blink_delay);
                Log_String(msg);
            }
        }

        LED_Toggle(LED_1); 
        vTaskDelay(blink_delay);
    }
}
