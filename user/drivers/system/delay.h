#ifndef __DELAY_DRIVER_H
#define __DELAY_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/* ============================================================================
 * Delay Driver Public API
 * ========================================================================= */
void Delay_Init(void);
uint32_t micros(void);
uint32_t millis(void);
void Delay_us(uint32_t us);
void Delay_ms(uint32_t ms);

#ifdef __cplusplus
}
#endif

#endif // __DELAY_DRIVER_H
