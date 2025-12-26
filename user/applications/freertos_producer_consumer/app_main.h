#ifndef APP_MAIN_H
#define APP_MAIN_H

#include "usart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define CH_DEBUG 2

// Command Queue Handle
extern QueueHandle_t xCmdQueue;

// Task Function Prototypes
void SerialProducerTask(void *pvParameters);
void LEDConsumerTask(void *pvParameters);

// Common Helper
void Log_String(const char *str);

#endif // APP_MAIN_H
