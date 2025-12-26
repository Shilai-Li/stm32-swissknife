#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "main.h"
#include <stdio.h>
#include "usart.h"
#include "uart.h"
#include "usb_cdc.h"
#include "io/led.h" // LED Driver

#define CH_DEBUG 2

// Dual-Output Logger Helper
void Log_String(const char *str) {
    if (!str) return;
    UART_SendString(CH_DEBUG, str);
    USB_CDC_SendString(str);
}

// Queue Handle
QueueHandle_t xCmdQueue = NULL;

// Task handles
TaskHandle_t xProducerHandle = NULL;
TaskHandle_t xConsumerHandle = NULL;

// Task 1: Serial Producer
// Reads from UART and sends commands to the queue
void SerialProducerTask(void *pvParameters)
{
    uint8_t rx_data;
    char msg[64];

    Log_String("[Producer] Started. Send '1', '2', '3' to change speed.\r\n");

    for (;;)
    {
        // Check if data is available (non-blocking read would be nicer, but let's poll or use Receive)
        // Using UART_Receive with a short timeout to allow other tasks to run if blocked inside driver
        // Assuming UART_Receive is implemented with some blocking mechanism or just returns false.
        // We add vTaskDelay to yield if no data.
        
        if (UART_Receive(CH_DEBUG, &rx_data, 10)) {
            // Echo back
            // UART_Send(CH_DEBUG, &rx_data, 1); 

            // Send to Queue
            if (xQueueSend(xCmdQueue, &rx_data, 10) == pdTRUE) {
                sprintf(msg, "[Producer] Sent cmd: %c\r\n", rx_data);
                Log_String(msg);
            } else {
                Log_String("[Producer] Queue Full!\r\n");
            }
        } else {
            // Yield to avoid starving other tasks if UART_Receive doesn't block
             vTaskDelay(10); 
        }
    }
}

// Task 2: LED Consumer
// Reads from queue to update delay, and handles blinking
void LEDConsumerTask(void *pvParameters)
{
    uint32_t blink_delay = 500; // Default 500ms
    uint8_t cmd;
    char msg[64];

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

        LED_Toggle(LED_1); // Toggle the Nucleo LED
        vTaskDelay(blink_delay);
    }
}

void user_main(void)
{
    // Inject hardware handles
    // Ensure huart2 is initialized in main.c before calling user_main or guaranteed here
    UART_Register(CH_DEBUG, &huart2);
    
    // Initialize USB
    USB_CDC_Init();
    
    // 注册板载 LED (Nucleo F446RE: LD2 is PA5)
    LED_Register(LED_1, GPIOA, GPIO_PIN_5, LED_ACTIVE_HIGH);

    UART_SendString(CH_DEBUG, "Starting FreeRTOS Queue Demo...\r\n");

    // Create Queue (Depth 10, Item Size: uint8_t)
    xCmdQueue = xQueueCreate(10, sizeof(uint8_t));

    if (xCmdQueue == NULL) {
        UART_SendString(CH_DEBUG, "Error: Failed to create queue!\r\n");
        return;
    }

    // Create tasks
    xTaskCreate(SerialProducerTask, "Producer", 256, NULL, 1, &xProducerHandle);
    xTaskCreate(LEDConsumerTask, "Consumer", 256, NULL, 1, &xConsumerHandle);

    UART_SendString(CH_DEBUG, "Tasks created. Scheduler should be running.\r\n");
}
