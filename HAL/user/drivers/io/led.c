#include "led.h"

#include "stm32f1xx_hal.h"

// LED GPIO configuration
GPIO_TypeDef* GPIO_PORT[LED_n] = {GPIOC, GPIOC};
uint16_t GPIO_PIN[LED_n] = {GPIO_PIN_13, GPIO_PIN_14};
// Define the logic level that turns the buzzer ON
GPIO_PinState LED_ON_LEVEL[LED_n] = {GPIO_PIN_RESET, GPIO_PIN_RESET}; // Active Low

/* ========== HAL: Public API Implementation ========== */

/*
 * @brief Initialize LED GPIO pins (HAL)
 */
void LED_Init(void)
{
  // Turn off initially
  for (int i = 0; i < LED_n; i++)
  {
    LED_Off((Led_TypeDef)i);
  }
}

void LED_On(Led_TypeDef Led)
{
  HAL_GPIO_WritePin(GPIO_PORT[Led], GPIO_PIN[Led], LED_ON_LEVEL[Led]);
}

void LED_Off(Led_TypeDef Led)
{
  // Off state is the opposite of On state
  GPIO_PinState off_level = (LED_ON_LEVEL[Led] == GPIO_PIN_SET) ? GPIO_PIN_RESET : GPIO_PIN_SET;
  HAL_GPIO_WritePin(GPIO_PORT[Led], GPIO_PIN[Led], off_level);
}

void LED_Toggle(Led_TypeDef Led)
{
  HAL_GPIO_TogglePin(GPIO_PORT[Led], GPIO_PIN[Led]);
}