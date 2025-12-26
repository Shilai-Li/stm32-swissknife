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

/**
 * @brief  Register the RTC Handler
 * @param  hrtc Pointer to the HAL RTC Handle
 */
void RTC_Register(RTC_HandleTypeDef *hrtc);

/**
 * @brief  Set Alarm Callback
 * @param  cb Callback function
 */
void RTC_SetAlarmCallback(RTC_AlarmCallback cb);

/**
 * @brief  Set Current Time using Unix Timestamp
 * @param  timestamp Seconds since Jan 1 1970
 * @return true if successful
 */
bool RTC_SetTimeUnix(uint32_t timestamp);

/**
 * @brief  Get Current Time as Unix Timestamp
 * @return Seconds since Jan 1 1970
 */
uint32_t RTC_GetTimeUnix(void);

/**
 * @brief  Get Formatted Time String
 * @details Format: "YYYY-MM-DD HH:MM:SS" (19 chars + null)
 * @param  buffer Output buffer (must be at least 20 bytes)
 */
void RTC_GetTimeString(char *buffer);

/**
 * @brief  Init check helper
 * @return true if RTC hardware is running
 */
bool RTC_IsReady(void);

#ifdef __cplusplus
}
#endif

#endif // RTC_DRIVER_H
