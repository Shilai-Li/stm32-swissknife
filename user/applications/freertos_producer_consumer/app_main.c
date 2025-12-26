#include "app_main.h"
#include "main.h"
#include "uart.h"
#include "usb_cdc.h"
#include "led.h"

// Definition of the Queue Handle
QueueHandle_t xCmdQueue = NULL;

TaskHandle_t xProducerHandle = NULL;
TaskHandle_t xConsumerHandle = NULL;

// Buffers for UART
static uint8_t u_rx_dma[64];
static uint8_t u_rx_ring[256];
static uint8_t u_tx_ring[512]; // Bigger TX for logs

void app_main(void)
{
    // Inject hardware handles
    UART_Register(CH_DEBUG, &huart2, 
                  u_rx_dma, sizeof(u_rx_dma),
                  u_rx_ring, sizeof(u_rx_ring),
                  u_tx_ring, sizeof(u_tx_ring));
    
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
