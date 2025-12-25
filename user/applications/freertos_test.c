#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include <stdio.h>

// 任务句柄（可选，如果需要控制任务）
TaskHandle_t xTestTaskHandle = NULL;

// 任务函数
void TestTask(void *pvParameters)
{
    // 获取传递的参数（如果有）
    // char *taskName = (char *)pvParameters;

    for (;;)
    {
        // 这里的代码会无限循环执行
        // 例如：翻转 LED 或打印日志
        printf("Hello from FreeRTOS Task!\n");

        // 延时 1000 ticks (通常配置为 1ms 一跳，即 1秒)
        vTaskDelay(1000);
    }
}

void user_main(void)
{
    printf("Starting FreeRTOS Scheduler...\n");

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
        printf("Task created successfully.\n");
        
        // 启动调度器 - 这通常不会返回，除非内存不足
        vTaskStartScheduler();
    }
    else
    {
        printf("Failed to create task!\n");
    }

    // 如果运行到这里，说明调度器启动失败（通常是堆栈内存不足）
    while (1);
}
