#include "freertos_port.h"
#include "main.h" // Includes stm32f1xx_hal.h
#include <stdio.h>

/*
 * This file implements the hooks and port-specific glue code for FreeRTOS
 */

// If we are using static allocation, we need to provide these callbacks
// configSUPPORT_STATIC_ALLOCATION is 1 in our config
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer,
                                    StackType_t **ppxIdleTaskStackBuffer,
                                    uint32_t *pulIdleTaskStackSize )
{
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer,
                                     StackType_t **ppxTimerTaskStackBuffer,
                                     uint32_t *pulTimerTaskStackSize )
{
    static StaticTask_t xTimerTaskTCB;
    static StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];

    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

// Hook called on stack overflow (configCHECK_FOR_STACK_OVERFLOW > 0)
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    // Log error or breakpoint
    // printf("Stack Overflow in task: %s\n", pcTaskName);
    __disable_irq();
    while(1);
}

// Hook called on malloc failure (configUSE_MALLOC_FAILED_HOOK > 0)
void vApplicationMallocFailedHook(void)
{
    // Log error or breakpoint
    // printf("Malloc failed!\n");
    __disable_irq();
    while(1);
}

// Hook called on idle task (configUSE_IDLE_HOOK > 0)
void vApplicationIdleHook(void)
{
    // Put CPU to sleep/low power mode if desired
}

// Assert handler
void vAssertCalled(char *file, int line)
{
    // printf("Assert failed in %s:%d\n", file, line);
    __disable_irq();
    while(1);
}

/**
  * @brief  This function handles SysTick Handler.
  *         IMPORTANT: This function is named SysTick_Handler in the vector table.
  *         If FreeRTOS's xPortSysTickHandler is mapped to SysTick_Handler, 
  *         FreeRTOS handles the ISR.
  *         We might need to call HAL_IncTick() within FreeRTOS tick hook or 
  *         via xPortSysTickHandler modification.
  *         
  *         HOWEVER, the standard FreeRTOS GCC port xPortSysTickHandler DOES NOT call HAL_IncTick().
  *         
  *         Option A: Modify startup file to call a wrapper that calls both.
  *         Option B: Use a Tick Hook (vApplicationTickHook) to call HAL_IncTick().
  *         
  *         We enabled configUSE_TICK_HOOK = 0 in config, let's stick to standard practice:
  *         Overriding HAL_Delay/HAL_InitTick is often cleaner for strict integration,
  *         BUT for simple integration:
  *         We will depend on the user ensuring SysTick_Handler calls xPortSysTickHandler.
  *         
  *         Since we defined #define xPortSysTickHandler SysTick_Handler in FreeRTOSConfig.h,
  *         FreeRTOS implementation of SysTick_Handler will be used.
  *         This means HAL_IncTick() will NOT be called unless we do it elsewhere.
  *         
  *         Wait, if HAL_IncTick is not called, HAL_Delay won't work.
  *         
  *         SOLUTION: overwrite HAL_InitTick to use a dedicated timer, 
  *         OR implement a vApplicationTickHook that calls HAL_IncTick().
  *         
  *         Let's try the Tick Hook approach, but it runs in interrupt context?
  *         Yes, vApplicationTickHook runs inside the tick ISR.
  *         
  *         Let's re-enable configUSE_TICK_HOOK in config if we go this route.
  *         
  *         Better Component Way: 
  *         Leave SysTick alone in the vector table (STM32 HAL default).
  *         Call xPortSysTickHandler() FROM the Project's SysTick_Handler.
  *         
  *         So in `stm32f1xx_it.c`:
  *         void SysTick_Handler(void) {
  *             HAL_IncTick();
  *             if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
  *                 xPortSysTickHandler();
  *             }
  *         }
  *         
  *         This requires REMOVING the `#define xPortSysTickHandler SysTick_Handler` from FreeRTOSConfig.h
  */

// For this specific 'component' integration without modifying 'stm32f1xx_it.c' too much immediately:
// We can't easily inject code into existing stm32f1xx_it.c unless the user does it.
// 
// I will keep the Mapping in FreeRTOSConfig.h for now so it compiles and "works" for FreeRTOS.
// I will provide instructions to the user to fix the HAL_Tick issue, 
// OR I can use a different method.
// 
// Actually, simple fix:
// We don't define xPortSysTickHandler as SysTick_Handler in Config.
// We let the user add `xPortSysTickHandler();` to their `SysTick_Handler` in `stm32f1xx_it.c`.
// This is the standard, documented way for STM32+FreeRTOS manual integration.
