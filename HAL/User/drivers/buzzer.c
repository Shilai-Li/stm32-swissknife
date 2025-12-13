#include "buzzer.h"
#include "delay.h"

#include "stm32f1xx_hal.h"

// Buzzer GPIO configuration
GPIO_TypeDef* BUZZER_PORT[BUZZER_n] = {GPIOC, GPIOC};
uint16_t BUZZER_PIN[BUZZER_n] = {GPIO_PIN_13, GPIO_PIN_14};
// Define the logic level that turns the buzzer ON
GPIO_PinState BUZZER_ON_LEVEL[BUZZER_n] = {GPIO_PIN_RESET, GPIO_PIN_RESET}; // Active Low

/* ========== HAL: Public API Implementation ========== */

/*
 * @brief Initialize Buzzer GPIO pins (HAL)
 */
void BUZZER_Init(void)
{
    // Turn off initially
  for (int i = 0; i < BUZZER_n; i++)
  {
    BUZZER_Off((Buzzer_TypeDef)i);
  }
}

void BUZZER_On(Buzzer_TypeDef Buzzer)
{
  HAL_GPIO_WritePin(BUZZER_PORT[Buzzer], BUZZER_PIN[Buzzer], BUZZER_ON_LEVEL[Buzzer]);
}

void BUZZER_Off(Buzzer_TypeDef Buzzer)
{
  // Off state is the opposite of On state
  GPIO_PinState off_level = (BUZZER_ON_LEVEL[Buzzer] == GPIO_PIN_SET) ? GPIO_PIN_RESET : GPIO_PIN_SET;
  HAL_GPIO_WritePin(BUZZER_PORT[Buzzer], BUZZER_PIN[Buzzer], off_level);
}

void BUZZER_Toggle(Buzzer_TypeDef Buzzer)
{
  HAL_GPIO_TogglePin(BUZZER_PORT[Buzzer], BUZZER_PIN[Buzzer]);
}