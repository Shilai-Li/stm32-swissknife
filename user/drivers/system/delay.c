/**
 * @file delay.c
 * @brief DWT-based Delay Driver Implementation
 */

#include "delay.h"

/* 
 * DWT (Data Watchpoint and Trace) is a debug unit inside Cortex-M3/M4/M7.
 * It provides a cycle counter (CYCCNT) that increments at CPU clock speed.
 * This allows for very precise microsecond delays without using a hardware Timer.
 */

static uint32_t cpu_freq_mhz = 0;

void Delay_Init(void)
{
    /* 1. Get CPU Frequency in MHz */
    cpu_freq_mhz = HAL_RCC_GetHCLKFreq() / 1000000;

    /* 2. Enable TRCENA in DEMCR (Debug Exception and Monitor Control Register) 
     *    This enables the DWT unit.
     */
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

    /* 3. Reset Cycle Counter */
    DWT->CYCCNT = 0;

    /* 4. Enable Cycle Counter in DWT_CTRL */
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

uint32_t micros(void)
{
    if (cpu_freq_mhz == 0) Delay_Init(); // Auto-init if forgotten
    return DWT->CYCCNT / cpu_freq_mhz;
}

uint32_t millis(void)
{
    return HAL_GetTick(); 
}

void Delay_us(uint32_t us)
{
    if (cpu_freq_mhz == 0) Delay_Init();
    
    uint32_t start_cycles = DWT->CYCCNT;
    uint32_t target_cycles = us * cpu_freq_mhz;

    // Handle overflow automatically by using unsigned arithmetic subtraction
    while ((DWT->CYCCNT - start_cycles) < target_cycles)
    {
        __NOP(); 
    }
}

void Delay_ms(uint32_t ms)
{
    HAL_Delay(ms);
}
