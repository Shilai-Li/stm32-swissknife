#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include <stdio.h>
#include "usart.h"
#include "uart.h"
#include "usb_cdc.h" // Added USB Support

#define CH_DEBUG 2

// Dual-Output Logger Helper
void Log_String(const char *str) {
    if (!str) return;
    UART_SendString(CH_DEBUG, str);
    USB_CDC_SendString(str);
}

// 任务句柄
TaskHandle_t xTestTaskHandle = NULL;
TaskHandle_t xBlinkTaskHandle = NULL;

// Task 1: Slow Logger
void TestTask(void *pvParameters)
{
    // 等到USB枚举
    vTaskDelay(2000);

    for (;;)
    {
        Log_String("[Task1] Hello from Slow Task (1s)\r\n");
        vTaskDelay(1000);
    }
}

// Task 2: Fast Logger
void BlinkTask(void *pvParameters)
{
    // 稍微错开启动时间
    vTaskDelay(2500);

    for (;;)
    {
        Log_String("[Task2] Fast Heartbeat (500ms)\r\n");
        vTaskDelay(500);
    }
}

void user_main(void)
{
    // 注入硬件句柄
    UART_Register(CH_DEBUG, &huart2);
    // 初始化 USB
    USB_CDC_Init();

    UART_SendString(CH_DEBUG, "Starting FreeRTOS Scheduler... (Creating Tasks)\r\n");

    // 创建任务 1
    BaseType_t ret1 = xTaskCreate(TestTask, "TestTask", 128, NULL, 1, &xTestTaskHandle);
    
    // 创建任务 2
    BaseType_t ret2 = xTaskCreate(BlinkTask, "BlinkTask", 128, NULL, 1, &xBlinkTaskHandle);

    if (ret1 == pdPASS && ret2 == pdPASS)
    {
        UART_SendString(CH_DEBUG, "All Tasks created successfully. Exiting user_main.\r\n");
    }
    else
    {
        UART_SendString(CH_DEBUG, "Failed to create tasks! (Stack/Heap Limit)\r\n");
        while(1);
    }
}
