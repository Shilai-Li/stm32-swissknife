#ifndef __DELAY_DRIVER_H
#define __DELAY_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

/* --- Configuration Start --- */
/**
 * @brief Select the timer to use for the delay driver.
 * Options: 1 (TIM1), 2 (TIM2), 3 (TIM3), 4 (TIM4)
 */
#define USE_DELAY_TIM 3

#if USE_DELAY_TIM == 1
    extern TIM_HandleTypeDef    htim1;
    #define DELAY_TIM_INSTANCE  TIM1
#elif USE_DELAY_TIM == 2
    extern TIM_HandleTypeDef    htim2;
    #define DELAY_TIM_INSTANCE  TIM2
#elif USE_DELAY_TIM == 3
    extern TIM_HandleTypeDef    htim3;
    #define DELAY_TIM_HANDLE htim3
    #define DELAY_TIM_INSTANCE  TIM3
#elif USE_DELAY_TIM == 4
    extern TIM_HandleTypeDef    htim4;
    #define DELAY_TIM_INSTANCE  TIM4
#else
    //#error "Invalid USE_DELAY_TIM selection! Choose 1, 2, 3, or 4."
#endif
/* --- Configuration End --- */

void Delay_Init(void);
uint32_t micros(void);
uint32_t millis(void);
void Delay_us(uint32_t us);
void Delay_ms(uint32_t ms);
void Delay_ms(uint32_t ms);
void Delay_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);

#ifdef __cplusplus
}
#endif

#endif
