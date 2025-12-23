# FreeRTOS Component for STM32 Swissknife

This component integrates the FreeRTOS Kernel (Cortex-M3 port) into the project.

## ⚠️ Critical Integration Step (Must Do!)

To ensure FreeRTOS runs correctly alongside the STM32 HAL library, you **MUST** manually modify your project's interrupt handler file.

**File:** `HAL/Core/Src/stm32f1xx_it.c` (or equivalent for your chip)

Find the `SysTick_Handler` function and modify it to look like this:

```c
/* USER CODE BEGIN Includes */
#include "FreeRTOS.h"
#include "task.h"
/* USER CODE END Includes */

// ...

/* USER CODE BEGIN 0 */
extern void xPortSysTickHandler(void);
/* USER CODE END 0 */

void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */
  
  /* 1. Maintain HAL Timebase (Required for HAL_Delay) */
  HAL_IncTick();

  /* 2. Call FreeRTOS Tick Handler (Only if scheduler is running) */
  if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
      xPortSysTickHandler();
  }

  /* USER CODE END SysTick_IRQn 0 */
  
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}
```

### Why is this necessary?
- **HAL_IncTick()**: Updates the HAL's global tick counter. Without this, `HAL_Delay()` will look like it hangs forever.
- **xPortSysTickHandler()**: Drives the FreeRTOS scheduler (task switching).
- We cannot map `SysTick_Handler` directly to `xPortSysTickHandler` in `FreeRTOSConfig.h` because that would overwrite the HAL's ability to count time.

## 📁 Directory Structure

```
freertos/
├── csrc/                # Official FreeRTOS Kernel Source (Do not edit directly)
│   ├── tasks.c
│   ├── queue.c
│   ├── ...
│   └── portable/        # GCC/ARM_CM3 Port & Heap_4
├── FreeRTOSConfig.h     # Configuration file for STM32F1
├── freertos_port.c      # Hooks (Stack Overflow, Malloc Failed)
├── freertos_port.h
├── update_freertos.ps1  # Update script
└── README.md            # This file
```

## 🔄 Updating FreeRTOS

To update or re-download the kernel source:

1.  Clone the FreeRTOS Kernel repository locally (e.g., to `temp_downloads/FreeRTOS-Kernel`).
2.  Run the update script:
    ```powershell
    cd HAL/user/components/freertos
    .\update_freertos.ps1
    ```

## ⚙️ Configuration Notes

- **Heap Management**: Uses `heap_4.c`.
- **Total Heap Size**: Configured to **15 KB** in `FreeRTOSConfig.h` (`configTOTAL_HEAP_SIZE`). Adjust if your MCU has less RAM.
- **Hooks**: Stack Overflow and Malloc Failed hooks are enabled and implemented in `freertos_port.c`. They will disable interrupts and loop forever if triggered (for debugging).

## 🧪 Testing

1.  In `HAL/user/CMakeLists.txt`, set:
    ```cmake
    set(TEST_CASE "freertos" CACHE STRING "Select which test to run" FORCE)
    ```
2.  Compile and Run.
3.  The test (`tests/freertos_tests.c`) launches two tasks:
    - **LED_Task**: Blinks LED (Default PC13).
    - **Print_Task**: Prints status every 2 seconds.
