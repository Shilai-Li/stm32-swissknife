#include "FreeRTOS.h"
#include "task.h"
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

// Task handles
TaskHandle_t xTestTaskHandle = NULL;
TaskHandle_t xBlinkTaskHandle = NULL;

// Task 1: Slow Logger
void TestTask(void *pvParameters)
{
    // Wait for USB enumeration
    vTaskDelay(2000);

    for (;;)
    {
        Log_String("[Task1] Hello from Slow Task (1s)\r\n");
        vTaskDelay(1000);
    }
}

// Task 2: Fast Logger & Blinker
void BlinkTask(void *pvParameters)
{
    // Slightly offset start time
    vTaskDelay(2500);

    for (;;)
    {
        Log_String("[Task2] Fast Heartbeat (500ms)\r\n");
        LED_Toggle(LED_1); // Toggle the Nucleo LED
        vTaskDelay(500);
    }
}

void user_main(void)
{
    // Inject hardware handles
    UART_Register(CH_DEBUG, &huart2);
    // Initialize USB
    USB_CDC_Init();
    
    // 注册板载 LED (Nucleo F446RE: LD2 is PA5)
    LED_Register(LED_1, GPIOA, GPIO_PIN_5, LED_ACTIVE_HIGH);

    UART_SendString(CH_DEBUG, "Starting FreeRTOS Scheduler... (Creating Tasks)\r\n");

    // Create task 1
    xTaskCreate(TestTask, "TestTask", 128, NULL, 1, &xTestTaskHandle);
    
    // Create task 2
    xTaskCreate(BlinkTask, "BlinkTask", 128, NULL, 1, &xBlinkTaskHandle);

    UART_SendString(CH_DEBUG, "Tasks created successfully. Exiting user_main.\r\n");
}
