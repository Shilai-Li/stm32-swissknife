#ifndef __BUZZER_DRIVER_H__
#define __BUZZER_DRIVER_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef USE_STDPERIPH_DRIVER

#include <stdint.h>
#include <stdbool.h>

    /* MCU series selection */
#define STM32F1
    //#define STM32F4

    /* Buzzer count */
#define BUZZER_MAX_COUNT   1

#ifdef STM32F1
#include "stm32f10x.h"
#endif

/* ============================================================
 * Type Definitions
 * ============================================================ */

/* Buzzer activation level */
typedef enum {
		BUZZER_ACTIVE_HIGH,   /* High level turns buzzer ON */
		BUZZER_ACTIVE_LOW     /* Low level turns buzzer ON */
} BUZZER_Active_Level;

/* Buzzer object structure */
typedef struct {
		GPIO_TypeDef *port;         /* GPIO port */
		uint16_t pin;               /* GPIO pin */
		bool is_on;                 /* Current buzzer state */
		BUZZER_Active_Level level;  /* Activation level */
} BUZZER;


/* ============================================================
 * Public API Functions
 * ============================================================ */
void BUZZER_Init(void);
void BUZZER_On(uint8_t id);
void BUZZER_Off(uint8_t id);
void BUZZER_Toggle(uint8_t id);
void BUZZER_SetState(uint8_t id, bool state);
void BUZZER_Test(void);

		
#endif

#ifdef __cplusplus
}
#endif

#endif /* __BUZZER_DRIVER_H__ */
