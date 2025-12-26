#ifndef RTC_DRIVER_H
#define RTC_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <time.h>
#include <stdbool.h>

/* ============================================================================
 * RTC Configuration & Types
 * ========================================================================= */

// Callback type for RTC Alarms (Optional, future proofing)
typedef void (*RTC_AlarmCallback)(void);

/* ============================================================================
 * RTC Public API
 * ========================================================================= */

void RTC_Register(RTC_HandleTypeDef *hrtc);
void RTC_SetAlarmCallback(RTC_AlarmCallback cb);
bool RTC_SetTimeUnix(uint32_t timestamp);
uint32_t RTC_GetTimeUnix(void);
void RTC_GetTimeString(char *buffer);
bool RTC_IsReady(void);

#ifdef __cplusplus
}
#endif

#endif // RTC_DRIVER_H
