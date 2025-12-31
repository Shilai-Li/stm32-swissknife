/**
 * @file rtc_driver.c
 * @brief RTC Driver Wrapper Implementation
 * Supports both F1 (Counter-based) and F4/F7/H7 (Calendar-based) RTC
 */

#include "rtc_driver.h"
#include <stdio.h>
#include <string.h>

#ifdef HAL_RTC_MODULE_ENABLED

// Internal State
static RTC_HandleTypeDef *phrtc = NULL;
static RTC_AlarmCallback AlarmCallback = NULL;

void RTC_Register(RTC_HandleTypeDef *hrtc)
{
    phrtc = hrtc;
}

void RTC_SetAlarmCallback(RTC_AlarmCallback cb)
{
    AlarmCallback = cb;
}

#if defined(STM32F1) || defined(STM32F103xB) || defined(STM32F103xE)
// ============================================================================
// STM32F1 Implementation (Counter-based RTC)
// ============================================================================

bool RTC_SetTimeUnix(uint32_t timestamp) {
    if (!phrtc) return false;

    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    RTC_TypeDef *rtc = phrtc->Instance;
    uint32_t timeout = 0;
    while (!(rtc->CRL & RTC_CRL_RTOFF)) {
        if (++timeout > 0xFFFF) {
            __set_PRIMASK(primask);
            return false;
        }
    }
    
    rtc->CRL |= RTC_CRL_CNF;
    rtc->CNTH = (timestamp >> 16);
    rtc->CNTL = (timestamp & 0xFFFF);
    rtc->CRL &= ~RTC_CRL_CNF;
    
    while (!(rtc->CRL & RTC_CRL_RTOFF));
    
    __set_PRIMASK(primask);
    return true;
}

uint32_t RTC_GetTimeUnix(void) {
    if (!phrtc) return 0;

    RTC_TypeDef *rtc = phrtc->Instance;
    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    uint16_t low = (uint16_t)rtc->CNTL;
    uint16_t high = (uint16_t)rtc->CNTH;
    
    __set_PRIMASK(primask);

    return ((uint32_t)high << 16) | low;
}

#else
// ============================================================================
// STM32F4/F7/H7 Implementation (Calendar-based RTC)
// ============================================================================

bool RTC_SetTimeUnix(uint32_t timestamp) {
    if (!phrtc) return false;

    // Convert Unix timestamp to struct tm
    time_t ts = (time_t)timestamp;
    struct tm *timeinfo = gmtime(&ts);
    
    if (!timeinfo) return false;

    // Fill HAL structures
    RTC_TimeTypeDef sTime = {0};
    sTime.Hours = timeinfo->tm_hour;
    sTime.Minutes = timeinfo->tm_min;
    sTime.Seconds = timeinfo->tm_sec;
    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;

    RTC_DateTypeDef sDate = {0};
    sDate.Year = timeinfo->tm_year - 100; // tm_year is years since 1900; RTC is years since 2000
    sDate.Month = timeinfo->tm_mon + 1;
    sDate.Date = timeinfo->tm_mday;
    sDate.WeekDay = (timeinfo->tm_wday == 0) ? 7 : timeinfo->tm_wday; // RTC: 1=Mon, 7=Sun

    if (HAL_RTC_SetTime(phrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK) {
        return false;
    }
    if (HAL_RTC_SetDate(phrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK) {
        return false;
    }

    return true;
}

uint32_t RTC_GetTimeUnix(void) {
    if (!phrtc) return 0;

    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};

    if (HAL_RTC_GetTime(phrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK) {
        return 0;
    }
    if (HAL_RTC_GetDate(phrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK) {
        return 0;
    }

    // Convert to struct tm
    struct tm timeinfo = {0};
    timeinfo.tm_year = sDate.Year + 100; // RTC years since 2000; tm_year is years since 1900
    timeinfo.tm_mon = sDate.Month - 1;
    timeinfo.tm_mday = sDate.Date;
    timeinfo.tm_hour = sTime.Hours;
    timeinfo.tm_min = sTime.Minutes;
    timeinfo.tm_sec = sTime.Seconds;
    timeinfo.tm_isdst = -1; // Auto-detect DST

    // Convert to Unix timestamp
    return (uint32_t)mktime(&timeinfo);
}

#endif

// ============================================================================
// Common Implementation (All Series)
// ============================================================================

void RTC_GetTimeString(char *buffer) {
    if (!buffer) return;
    
    time_t now = (time_t)RTC_GetTimeUnix();
    struct tm *t = localtime(&now);
    
    sprintf(buffer, "%04d-%02d-%02d %02d:%02d:%02d",
            t->tm_year + 1900,
            t->tm_mon + 1,
            t->tm_mday,
            t->tm_hour,
            t->tm_min,
            t->tm_sec);
}

bool RTC_IsReady(void) {
    if (!phrtc) return false;
    return (HAL_RTC_GetState(phrtc) == HAL_RTC_STATE_READY);
}

#else

// Stubs when RTC module not enabled
void RTC_Register(RTC_HandleTypeDef *hrtc) { (void)hrtc; }
void RTC_SetAlarmCallback(RTC_AlarmCallback cb) { (void)cb; }
bool RTC_SetTimeUnix(uint32_t timestamp) { (void)timestamp; return false; }
uint32_t RTC_GetTimeUnix(void) { return 0; }
void RTC_GetTimeString(char *buffer) { if(buffer) strcpy(buffer, "RTC_DISABLED"); }
bool RTC_IsReady(void) { return false; }

#endif
