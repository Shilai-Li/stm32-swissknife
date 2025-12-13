/**
 * @file rtc_driver.h
 * @brief RTC Driver Wrapper for simplified Time/Date handling
 * @details Provides Unix Timestamp support and formatted strings
 *          Requires CubeMX RTC Config enabled.
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
