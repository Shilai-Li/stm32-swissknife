#include "app_main.h"
#include "main.h"
#include "uart.h"
#include "usb_cdc.h"
#include "led.h"

// Definition of the Queue Handle
QueueHandle_t xCmdQueue = NULL;

TaskHandle_t xProducerHandle = NULL;
TaskHandle_t xConsumerHandle = NULL;

void user_main(void)
{
    // Inject hardware handles
    UART_Register(CH_DEBUG, &huart2);
    
    // Initialize Drivers
    USB_CDC_Init();
    LED_Register(LED_1, GPIOB, GPIO_PIN_2, LED_ACTIVE_LOW);

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
