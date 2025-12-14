/**
 * @file rtc_driver.h
 * @brief RTC Driver Wrapper for simplified Time/Date handling
 * 
 * =================================================================================
 *                       >>> INTEGRATION GUIDE <<<
 * =================================================================================
 * 1. CubeMX Config (Timers -> RTC):
 *    - Activate Clock Source: Checked
 *    - Calendar Time/Date: Optional set default
 * 
 * 2. Clock Config (Clock Configuration Tab):
 *    - Enable LSE (32.768kHz Crystal) -> RTC Clock Mux -> LSE (Recommended)
 *    - OR LSI (Internal Low Speed) -> Less accurate
 * 
 * 3. Note for F1 Series:
 *    STM32F1 RTC is a simple Counter. Date is software calculated or backup reg stored.
 *    This driver treats the counter as a Unix Timestamp (seconds since 1970).
 * =================================================================================
 */

#ifndef RTC_DRIVER_H
#define RTC_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"
#include <time.h>
#include <stdbool.h>

/* Helper structure to avoid dependencies on HAL structs in user code if desired, 
   but standard tm struct is standard. */

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
