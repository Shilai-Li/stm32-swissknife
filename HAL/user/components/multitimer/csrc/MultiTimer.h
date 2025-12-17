/**
 * @file MultiTimer.h
 * @brief MultiTimer library header
 * @author 0x1abin
 * @license MIT
 */
#ifndef _MULTI_TIMER_H_
#define _MULTI_TIMER_H_

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MultiTimerHandle MultiTimer;

typedef void (*MultiTimerCallback_t)(MultiTimer* timer, void* userData);

struct MultiTimerHandle {
    MultiTimer* next;
    uint32_t deadline;
    uint32_t period;
    MultiTimerCallback_t callback;
    void* userData;
};

/**
 * @brief Initialize a timer object
 * @param timer The timer to initialize
 * @param period Period in milliseconds (0 for one-shot, but usually set in Start)
 * @param callback Callback function
 * @param userData User data pointer passed to callback
 */
void MultiTimerInit(MultiTimer* timer, uint32_t period, MultiTimerCallback_t callback, void* userData);

/**
 * @brief Start the timer
 * @param timer The timer to start
 * @param startTime Current time (usually from MultiTimerTicks)
 * @param period New period (optional, if 0 uses initialized period)
 */
int MultiTimerStart(MultiTimer* timer, uint32_t startTime, uint32_t period);

/**
 * @brief Stop the timer
 * @param timer The timer to stop
 */
void MultiTimerStop(MultiTimer* timer);

/**
 * @brief Check if timer is active
 */
int MultiTimerIsActive(MultiTimer* timer);

/**
 * @brief Main loop function, call this frequently
 */
void MultiTimerYield(void);

/**
 * @brief Platform tick function - To be implemented by user
 * @return Current system time in milliseconds
 */
uint32_t MultiTimerTicks(void);

#ifdef __cplusplus
}
#endif

#endif
