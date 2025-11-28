#include "delay_driver.h"

#if USE_DELAY_TIM

#include "tim.h"
#include "uart_driver.h"
#include <stdio.h>

/* Global variable to accumulate microseconds on timer overflow (uint64_t for robustness) */
static volatile uint64_t _micros_acc = 0;

/**
 * @brief  Initialize microsecond timer using the selected TIM instance.
 *         This function calculates the prescaler to ensure 1us tick.
 */
void Delay_Init(void)
{
    uint32_t timer_clock_freq = 0;
    RCC_ClkInitTypeDef RCC_ClkInitStruct;
    uint32_t pFLatency;

    /* Get clock configuration */
    HAL_RCC_GetClockConfig(&RCC_ClkInitStruct, &pFLatency);

    /* Determine Timer Clock Frequency */
    if (DELAY_TIM_INSTANCE == TIM1)
    {
        /* TIM1 is on APB2 */
        timer_clock_freq = HAL_RCC_GetPCLK2Freq();
        if (RCC_ClkInitStruct.APB2CLKDivider != RCC_HCLK_DIV1)
        {
            timer_clock_freq *= 2;
        }
    }
    else
    {
        /* TIM2, TIM3, TIM4 are on APB1 */
        timer_clock_freq = HAL_RCC_GetPCLK1Freq();
        if (RCC_ClkInitStruct.APB1CLKDivider != RCC_HCLK_DIV1)
        {
            timer_clock_freq *= 2;
        }
    }

    /* Calculate Prescaler for 1MHz (1us tick) */
    /* Prescaler = (TimerClock / 1MHz) - 1 */
    uint32_t prescaler = (timer_clock_freq / 1000000) - 1;

    /* Ensure TIM is stopped/reset before starting */
    __HAL_TIM_SET_COUNTER(&DELAY_TIM_HANDLE, 0);
    
    /* Update Prescaler */
    __HAL_TIM_SET_PRESCALER(&DELAY_TIM_HANDLE, prescaler);
    
    /* Start timer in interrupt mode */
    HAL_TIM_Base_Start_IT(&DELAY_TIM_HANDLE);
}

/**
 * @brief This callback is called automatically by HAL on timer overflow (update event).
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == DELAY_TIM_INSTANCE)
    {
        _micros_acc += 65536UL;  // 16-bit period = 65536 microseconds each overflow (1us per tick)
    }
}

/**
 * @brief Get current time in microseconds (rolls over every ~584,942 years)
 */
uint32_t micros(void)
{
    uint64_t base;
    uint16_t cnt;
    __disable_irq();  // Prevent race
    base = _micros_acc;
    cnt = __HAL_TIM_GET_COUNTER(&DELAY_TIM_HANDLE);
    __enable_irq();
    return (uint32_t)(base + cnt);
}

/**
 * @brief Get current time in milliseconds
 */
uint32_t millis(void)
{
    return micros() / 1000;
}

/**
 * @brief Busy-wait delay for a number of microseconds
 */
void Delay_us(uint32_t us)
{
    uint32_t start = micros();
    while ((micros() - start) < us)
    {
        // Busy wait
    }
}

/**
 * @brief Busy-wait delay for a number of milliseconds
 */
void Delay_ms(uint32_t ms)
{
    while (ms--)
    {
        Delay_us(1000);
    }
}

void Delay_Test(void)
{
    UART_Debug_Printf("Initializing Delay Driver...\r\n");
    Delay_Init();
    UART_Debug_Printf("Delay Driver Initialized.\r\n");

    UART_Debug_Printf("Testing 1s delay...\r\n");
    uint32_t start_ms = millis();
    Delay_ms(1000);
    uint32_t end_ms = millis();
    
    char buffer[64];
    sprintf(buffer, "Waited: %lu ms (Target: 1000 ms)\r\n", end_ms - start_ms);
    UART_Debug_Printf(buffer);
    
    UART_Debug_Printf("Delay Test Complete.\r\n");
}

#endif