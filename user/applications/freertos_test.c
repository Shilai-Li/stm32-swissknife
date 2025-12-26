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

// 任务句柄（可选，如果需要控制任务）
TaskHandle_t xTestTaskHandle = NULL;

// 任务函数
void TestTask(void *pvParameters)
{
    // 获取传递的参数（如果有）
    // char *taskName = (char *)pvParameters;
    
    // 等到USB枚举（简单延时）
    vTaskDelay(2000);

    for (;;)
    {
        // 这里的代码会无限循环执行
        // 例如：翻转 LED 或打印日志
        Log_String("Hello from FreeRTOS Task! (USB + UART)\r\n");

        // 延时 1000 ticks (通常配置为 1ms 一跳，即 1秒)
        vTaskDelay(1000);
    }
}

void user_main(void)
{
    // 注入硬件句柄
    UART_Register(CH_DEBUG, &huart2);
    // 初始化 USB
    USB_CDC_Init();

    // 此时 USB 可能还没枚举好，这句可能只在 UART 上看到
    Log_String("Starting FreeRTOS Scheduler... (USB might be pending)\r\n");

    // 创建任务
    // 参数说明：
    // 1. 任务函数指针
    // 2. 任务名称（字符串，用于调试）
    // 3. 堆栈大小（字数，不是字节数，例如 128 words = 512 bytes on 32-bit）
    // 4. 传递给任务的参数
    // 5. 优先级（数字越大优先级越高，或者越低，取决于 FreeRTOSConfig.h 定义，通常越大越高）
    // 6. 任务句柄保存位置
    BaseType_t ret = xTaskCreate(TestTask, "TestTask", 128, NULL, 1, &xTestTaskHandle);

    if (ret == pdPASS)
    {
        Log_String("Task created successfully.\r\n");
        // Scheduler is already running (started in main.c), so we just exit user_main.
        // StartDefaultTask will then call vTaskDelete(NULL) and our new TestTask will continue.
    }
    else
    {
        Log_String("Failed to create task!\r\n");
    }

    // 如果运行到这里，说明调度器启动失败（通常是堆栈内存不足）
    while (1);
}
