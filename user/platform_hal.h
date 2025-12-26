#ifndef __PLATFORM_HAL_H__
#define __PLATFORM_HAL_H__

/* 
 * This file acts as a bridge between the User Library and the STM32 HAL Library.
 * It automatically selects the correct HAL header based on the target MCU family.
 */

#if defined(STM32F0)
    #include "stm32f0xx_hal.h"
#elif defined(STM32F1) || defined(STM32F103xB) || defined(STM32F103xE)
    #include "stm32f1xx_hal.h"
#elif defined(STM32F2)
    #include "stm32f2xx_hal.h"
#elif defined(STM32F3)
    #include "stm32f3xx_hal.h"
#elif defined(STM32F4) || defined(STM32F405xx) || defined(STM32F407xx) || defined(STM32F429xx) || defined(STM32F446xx)
    #include "stm32f4xx_hal.h"
#elif defined(STM32F7)
    #include "stm32f7xx_hal.h"
#elif defined(STM32H7)
    #include "stm32h7xx_hal.h"
#elif defined(STM32L0)
    #include "stm32l0xx_hal.h"
#elif defined(STM32L1)
    #include "stm32l1xx_hal.h"
#elif defined(STM32L4)
    #include "stm32l4xx_hal.h"
#elif defined(STM32G0)
    #include "stm32g0xx_hal.h"
#elif defined(STM32G4)
    #include "stm32g4xx_hal.h"
#else
    // Fallback: If no family is defined, try to include main.h which typically includes the right HAL
    #include "main.h"
#endif

#endif // __PLATFORM_HAL_H__
