#ifndef __TIM_H__
#define __TIM_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef USE_STDPERIPH_DRIVER

#include <stdint.h>
#include <stdbool.h>

#define STM32F1
//#define STM32F4
#define LED_MAX_COUNT 2  /* Number of LEDs supported */

#ifdef STM32F1
#include "stm32f10x.h"
#endif
    
void TIM3_PWM_Init(uint16_t prescaler, uint16_t period);

#endif

#ifdef __cplusplus
}
#endif

#endif 