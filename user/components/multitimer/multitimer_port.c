/**
 * @file multitimer_port.c
 * @brief Platform adaptation for MultiTimer
 */
#include "multitimer.h"
#include "stm32f1xx_hal.h" // Replace with appropriate HAL header if needed, or use generic logic

/**
 * @brief Get current system tick in ms
 * Implements the external reference needed by MultiTimer.c
 */
uint32_t MultiTimerTicks(void) {
    return HAL_GetTick();
}

/**
 * @brief Initialize platform specific things
 * Currently MultiTimer just needs HAL_GetTick which is standard.
 */
void MultiTimer_Platform_Init(void) {
    // Nothing special needed if HAL is used
}
