#ifndef __DELAY_DRIVER_H
#define __DELAY_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef USE_STDPERIPH_DRIVER

#include <stdint.h>

#define STM32F1
//#define USE_SYSTICK_DELAY
#define USE_TIM2_DELAY

#include "stm32f10x.h"

void Delay_Init(void);
void Delay_NVIC_Init(void);
uint32_t micros(void);
uint32_t millis(void);
void Delay_us(uint32_t us);
void Delay_ms(uint32_t ms);
void TIM2_IRQHandler(void);

#endif

#ifdef __cplusplus
}
#endif

#endif
