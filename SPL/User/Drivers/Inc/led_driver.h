#ifndef __LED_DRIVER_H__
#define __LED_DRIVER_H__

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
    
/* ============================================================
 * Type Definitions
 * ============================================================ */
/* LED activation level */
typedef enum {
    LED_ACTIVE_HIGH,  /* High level turns LED ON */
    LED_ACTIVE_LOW    /* Low level turns LED ON */
} LED_Active_Level;

/* HAL LED object structure */
typedef struct {
    GPIO_TypeDef *port;           /* GPIO port */
    uint16_t pin;                 /* GPIO pin (HAL format) */
    bool is_on;                   /* LED current state */
    LED_Active_Level level;       /* Activation level */
} LED;

/* ============================================================
 * Public API Functions
 * ============================================================ */
void LED_Init(void);
void LED_On(uint8_t id);
void LED_Off(uint8_t id);
void LED_Toggle(uint8_t id);
void LED_SetState(uint8_t id, bool state);
void LED_SetBrightness_SPL(uint8_t brightness, uint16_t period);

void LED_Test(void);

#endif

#ifdef __cplusplus
}
#endif

#endif /* __LED_DRIVER_H__ */		