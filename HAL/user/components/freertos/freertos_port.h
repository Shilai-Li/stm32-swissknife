#ifndef FREERTOS_PORT_H
#define FREERTOS_PORT_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"

// Expose the hook functions that need to be implemented or are implemented in port.c
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName);
void vApplicationMallocFailedHook(void);
void vApplicationIdleHook(void);
void vAssertCalled(char *file, int line);


#endif // FREERTOS_PORT_H
