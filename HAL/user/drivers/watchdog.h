/**
 * @file watchdog.h
 * @brief Independent Watchdog (IWDG) Driver
 * @details Simplifies IWDG setup by taking milliseconds instead of prescalers.
 */

#ifndef WATCHDOG_H
#define WATCHDOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"
#include <stdbool.h>

/**
 * @brief  Initialize the Independent Watchdog (IWDG)
 * @note   Once enabled, it cannot be disabled until reset!
 * @param  timeout_ms Time in milliseconds before reset (Approximate).
 *                    Max timeout is usually around 26-32 seconds (depending on LSI).
 * @return true if successful, false if timeout is out of range.
 */
bool Watchdog_Init(uint32_t timeout_ms);

/**
 * @brief  Feed the Dog (Reload Counter)
 * @note   Call this periodically to prevent reset.
 */
void Watchdog_Feed(void);

/**
 * @brief  Check if the system was reset by the Watchdog
 * @return true if the last reset was caused by IWDG
 */
bool Watchdog_WasResetByDog(void);

#ifdef __cplusplus
}
#endif

#endif // WATCHDOG_H
