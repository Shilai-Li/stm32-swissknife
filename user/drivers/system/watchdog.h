#ifndef WATCHDOG_H
#define WATCHDOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdbool.h>

/* ============================================================================
 * Watchdog Driver - Supports both IWDG and WWDG
 * ========================================================================= */

#if defined(HAL_IWDG_MODULE_ENABLED)
void Watchdog_Register(IWDG_HandleTypeDef *hiwdg);
#elif defined(HAL_WWDG_MODULE_ENABLED)
void Watchdog_Register(WWDG_HandleTypeDef *hwwdg);
#endif

bool Watchdog_Init(uint32_t timeout_ms);
void Watchdog_Feed(void);
bool Watchdog_WasResetByDog(void);

#ifdef __cplusplus
}
#endif

#endif // WATCHDOG_H
