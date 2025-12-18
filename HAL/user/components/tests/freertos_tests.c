#include "freertos_port.h"
#include <stdio.h>
#include "main.h"

// Task handles
TaskHandle_t xLedTaskHandle = NULL;
TaskHandle_t xPrintTaskHandle = NULL;

// LED Task: Blinks an LED (assuming PC13 for BluePill, or check main.h)
void vLedTask(void *pvParameters)
{
    (void)pvParameters;
    
    // Attempt to detect LED pin, defaulting to typical PC13 if not defined
    // Users might need to adjust this
    #ifndef LED_PIN
        #define LED_PIN GPIO_PIN_13
        #define LED_PORT GPIOC
    #endif

    for (;;)
    {
        // Toggle LED
        // HAL_GPIO_TogglePin(LED_PORT, LED_PIN);
        // printf("Tick: %lu\n", (uint32_t)xTaskGetTickCount());
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// Print Task: Prints stack usage
void vPrintTask(void *pvParameters)
{
    (void)pvParameters;
    
    for (;;)
    {
        // printf("FreeRTOS Running... Free Heap: %u bytes\n", xPortGetFreeHeapSize());
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void FreeRTOS_Demo_Init(void)
{
    // Create Tasks
    xTaskCreate(vLedTask, "LED_Task", 128, NULL, 1, &xLedTaskHandle);
    xTaskCreate(vPrintTask, "Print_Task", 256, NULL, 1, &xPrintTaskHandle);

    // Start Scheduler
    // Note: This function generally does not return
    vTaskStartScheduler();
}
