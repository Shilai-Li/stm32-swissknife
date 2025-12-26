#ifndef __LED_DRIVER_H__
#define __LED_DRIVER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

// Number of LEDs supported (Logical Channels)
#define LED_MAX_CHANNELS 8

// Logical LED IDs (App uses these)
typedef enum
{
  LED_1 = 0,
  LED_2,
  LED_3,
  LED_4,
  // Add more if needed
} Led_TypeDef;

// Active Level (Is current sinking or sourcing?)
typedef enum {
    LED_ACTIVE_LOW  = 0, // Pin LOW = LED ON (Common)
    LED_ACTIVE_HIGH = 1  // Pin HIGH = LED ON
} Led_ActiveLevel_t;

/**
 * @brief Register a hardware GPIO to a logical LED ID
 * @param Led: Logical ID (LED_1, LED_2, etc.)
 * @param Port: STM32 GPIO Port (GPIOA, GPIOB, etc.)
 * @param Pin: STM32 GPIO Pin (GPIO_PIN_0, etc.)
 * @param ActiveLevel: LED_ACTIVE_LOW or LED_ACTIVE_HIGH
 */
void LED_Register(Led_TypeDef Led, GPIO_TypeDef* Port, uint16_t Pin, Led_ActiveLevel_t ActiveLevel);

// Basic Control
void LED_On(Led_TypeDef Led);
void LED_Off(Led_TypeDef Led);
void LED_Toggle(Led_TypeDef Led);

#ifdef __cplusplus
}
#endif

#endif /* __LED_DRIVER_H__ */
