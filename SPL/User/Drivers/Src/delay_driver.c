#include "delay_driver.h"

#ifdef USE_STDPERIPH_DRIVER

#ifdef USE_SYSTICK_DELAY

static volatile uint32_t delay_ms_counter = 0;

/* ============================================================
 * Initialize SysTick for 1ms interrupt tick
 * ============================================================ */
void Delay_Init(void)
{
    /* Configure SysTick to generate interrupt every 1ms */
    SysTick_Config(SystemCoreClock / 1000);
}

/* ============================================================
 * SysTick Interrupt Handler (1ms)
 * ============================================================ */
void SysTick_Handler(void)
{
    delay_ms_counter++;
}

/* ============================================================
 * Millisecond delay (uses the 1ms tick counter)
 * ============================================================ */
void Delay_ms(uint32_t ms)
{
    uint32_t start = delay_ms_counter;

    while ((delay_ms_counter - start) < ms)
    {
        /* Wait for ms to pass */
    }
}

/* ============================================================
 * Microsecond delay (safe version)
 * Uses temporary SysTick settings and restores previous state
 * ============================================================ */
void Delay_us(uint32_t us)
{
    uint32_t ticks = (SystemCoreClock / 1000000) * us;

    /* Backup SysTick registers */
    uint32_t backup_LOAD = SysTick->LOAD;
    uint32_t backup_VAL  = SysTick->VAL;
    uint32_t backup_CTRL = SysTick->CTRL;

    /* Configure SysTick for microsecond delay */
    SysTick->LOAD = ticks;
    SysTick->VAL  = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;

    /* Wait until COUNTFLAG is set */
    while (!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk))
    {
        /* Busy wait */
    }

    /* Restore original SysTick configuration */
    SysTick->CTRL = backup_CTRL;
    SysTick->LOAD = backup_LOAD;
    SysTick->VAL  = backup_VAL;
}

#endif

#ifdef USE_TIM2_DELAY

/* TIM2 clock is configured to run at 1MHz (1 tick = 1us) */

static volatile uint64_t _micros_acc = 0;

void Delay_Init(void)
{
    /* Enable TIM2 clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    /* TIM2 configuration */
    TIM_TimeBaseInitTypeDef tim;
    tim.TIM_Prescaler = (SystemCoreClock / 1000000) - 1;  /* 1MHz tick */
    tim.TIM_CounterMode = TIM_CounterMode_Up;
    tim.TIM_Period = 0xFFFF;  /* Max period */
    tim.TIM_ClockDivision = TIM_CKD_DIV1;
    tim.TIM_RepetitionCounter = 0;

    TIM_TimeBaseInit(TIM2, &tim);
    TIM_Cmd(TIM2, ENABLE);
}

void Delay_NVIC_Init(void)
{
    NVIC_InitTypeDef nvic;
    nvic.NVIC_IRQChannel = TIM2_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 1;
    nvic.NVIC_IRQChannelSubPriority = 1;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic);

    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
}

uint32_t micros(void)
{
    uint64_t base;
    uint16_t counter;

    __disable_irq();
    base = _micros_acc;           // accumulated us
    counter = TIM_GetCounter(TIM2);
    __enable_irq();

    return (uint32_t)(base + counter);
}

uint32_t millis(void)
{
    return micros() / 1000;
}

/* ============================================================
 * Delay in microseconds (busy wait)
 * ============================================================ */
void Delay_us(uint32_t us)
{
    uint32_t start = TIM_GetCounter(TIM2);
    while ((uint32_t)(TIM_GetCounter(TIM2) - start) < us)
    {
        /* busy wait */
    }
}

/* ============================================================
 * Delay in milliseconds
 * ============================================================ */
void Delay_ms(uint32_t ms)
{
    while (ms--)
    {
        Delay_us(1000);
    }
}

void TIM2_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
    {
        _micros_acc += 65536ULL;   // timer overflow → add 65536 us
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    }
}

#endif

#endif