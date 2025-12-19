/**
 * @file multitimer.h
 * @brief Wrapper header for MultiTimer
 */
#ifndef __MULTITIMER_WRAPPER_H__
#define __MULTITIMER_WRAPPER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "csrc/MultiTimer.h"

// Initialize the platform part (if any)
void MultiTimer_Platform_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* __MULTITIMER_WRAPPER_H__ */
