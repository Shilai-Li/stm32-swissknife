#include "led.h"
#include <stddef.h> // for NULL

// Internal handle structure/storage
typedef struct {
    GPIO_TypeDef* Port;
    uint16_t      Pin;
    uint8_t       ActiveLevel; // 0=Low, 1=High
    uint8_t       IsRegistered;
} LED_Handle_Internal_t;

// Storage for registered LEDs
static LED_Handle_Internal_t led_handles[LED_MAX_CHANNELS] = {0};

/* ========== Public API Implementation ========== */

void LED_Register(Led_TypeDef Led, GPIO_TypeDef* Port, uint16_t Pin, Led_ActiveLevel_t ActiveLevel)
{
    if (Led >= LED_MAX_CHANNELS) return;
    
    led_handles[Led].Port = Port;
    led_handles[Led].Pin = Pin;
    led_handles[Led].ActiveLevel = (uint8_t)ActiveLevel;
    led_handles[Led].IsRegistered = 1;

    // Optional: Set initial state to OFF
    LED_Off(Led);
}

void LED_On(Led_TypeDef Led)
{
    if (Led >= LED_MAX_CHANNELS || !led_handles[Led].IsRegistered) return;

    // Determine the physical state for "ON"
    GPIO_PinState state = (led_handles[Led].ActiveLevel == LED_ACTIVE_HIGH) ? GPIO_PIN_SET : GPIO_PIN_RESET;
    HAL_GPIO_WritePin(led_handles[Led].Port, led_handles[Led].Pin, state);
}

void LED_Off(Led_TypeDef Led)
{
    if (Led >= LED_MAX_CHANNELS || !led_handles[Led].IsRegistered) return;

    // Determine the physical state for "OFF"
    GPIO_PinState state = (led_handles[Led].ActiveLevel == LED_ACTIVE_HIGH) ? GPIO_PIN_RESET : GPIO_PIN_SET;
    HAL_GPIO_WritePin(led_handles[Led].Port, led_handles[Led].Pin, state);
}

void LED_Toggle(Led_TypeDef Led)
{
    if (Led >= LED_MAX_CHANNELS || !led_handles[Led].IsRegistered) return;

    HAL_GPIO_TogglePin(led_handles[Led].Port, led_handles[Led].Pin);
}
