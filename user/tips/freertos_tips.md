# FreeRTOS Debugging & Tips

This guide collects practical tips for debugging FreeRTOS applications on STM32, specifically focusing on common crashes (HardFaults) and stack issues.

## 1. Stack Overflow Detection (The "Silent Killer" Fix)

Stack overflows are the #1 cause of random crashes and HardFaults in RTOS. FreeRTOS has a built-in detector.

### Step 1: Enable in Config
Open `FreeRTOSConfig.h` (usually in `Core/Inc`) and add/modify:

```c
/* USER CODE BEGIN Defines */
#define configCHECK_FOR_STACK_OVERFLOW  2
/* USER CODE END Defines */
```
*   **Mode 1**: Checks stack pointer at swap out (Fast, misses some overflows).
*   **Mode 2**: Checks stack limit pattern (0xA5) for corruption (Robust, Recommended).

### Step 2: Implement the Hook
In any C file (e.g., `main.c`, `freertos.c`, or a debug utility file), implement the callback:

```c
// Called automatically by FreeRTOS when a stack overflow is detected
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    // ⚠️ CRITICAL: Stack is corrupted! Do MINIMAL work here.
    // 1. Log the error (Use Polling UART, not Interrupt/DMA if possible)
    printf("\r\n[FATAL] STACK OVERFLOW: Task '%s'\r\n", pcTaskName);
    
    // 2. Halt the system to preserve state for debugger
    __disable_irq();
    while(1); 
}
```

---

## 2. DefaultTask & CubeMX Initialization Pitfall

### The Issue
CubeMX often generates code where `MX_USB_DEVICE_Init()` or `MX_LWIP_Init()` is called inside the `StartDefaultTask`.

### The Symptom
*   USB doesn't enumerate.
*   "Device Descriptor Request Failed".
*   Random crash at startup.

### The Cause
The `defaultTask` created by CubeMX usually has a tiny stack (128 words = 512 bytes).
Heavy initialization functions (like USB logic) allocate large structures on the stack, instantly overflowing it before your main application logic even starts.

### The Fix
1.  **CubeMX**: Go to Middleware -> FREERTOS -> Tasks & Queues. Increase `defaultTask` stack size to **512 Words** (2048 Bytes) or more.
2.  **Code**: If manually editing `freertos.c`:
    ```c
    const osThreadAttr_t defaultTask_attributes = {
      .name = "defaultTask",
      .stack_size = 512 * 4, // WAS 128 * 4 -> Increased to avoid overflow
      .priority = (osPriority_t) osPriorityNormal,
    };
    ```

---

## 3. Creating Tasks Manually

When using `xTaskCreate` manually inside another task (e.g. inside `user_main` called by `defaultTask`):

*   **Remember**: `xTaskCreate` uses the **current task's stack** to perform the creation logic (allocating TCB, initializing stack frame).
*   If the current task's stack is running low (e.g., the 128-word defaultTask), `xTaskCreate` might fail or crash.
*   **Tip**: It is often cleaner to just let `defaultTask` be a one-time setup task that creates others and then deletes itself (`vTaskDelete(NULL)`).

---

## 4. HardFault Debugging (Quick Checklist)

If your system enters `HardFault_Handler` (infinite loop in `stm32f4xx_it.c`):

1.  **Check Stack Sizes**: Double them. Does the problem go away?
2.  **Check Interrupt Priorities**:
    *   STM32 HAL interrupts (USB, UART) must have logically **lower priority** (higher numerical value) than `configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY` (usually 5) if they call FreeRTOS API functions (`xQueueSendFromISR`, etc.).
    *   If you call FreeRTOS API from Priority 0 (Highest), you will crash the kernel.
3.  **Check `printf`**: Using standard `printf` without a thread-safe implementation inside multiple tasks is a common cause of lockups. Use a thread-safe logger or a single "Logger Task".
