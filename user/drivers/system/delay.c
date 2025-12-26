#include "delay.h"
#include "main.h" 

/* 
 * DWT (Data Watchpoint and Trace) is a debug unit inside Cortex-M3/M4/M7.
 * It provides a cycle counter (CYCCNT) that increments at CPU clock speed.
 * This allows for very precise microsecond delays without using a hardware Timer.
 */

static uint32_t cpu_freq_mhz = 0;

/**
 * @brief  Initialize DWT for microsecond delay
 */
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

/**
 * @brief Get current time in microseconds (from DWT cycle counter)
 * @note  This counter overflows every (2^32 / CPU_Freq) seconds.
 *        At 72MHz, it overflows every ~59 seconds.
 *        At 168MHz, it overflows every ~25 seconds.
 */
uint32_t micros(void)
{
    if (cpu_freq_mhz == 0) Delay_Init(); // Auto-init if forgotten
    return DWT->CYCCNT / cpu_freq_mhz;
}

/**
 * @brief Get current time in milliseconds
 */
uint32_t millis(void)
{
    // Use HAL_GetTick if available (SysTick based), otherwise use DWT
    return HAL_GetTick(); 
}

/**
 * @brief Busy-wait delay for a number of microseconds
 */
void Delay_us(uint32_t us)
{
    if (cpu_freq_mhz == 0) Delay_Init();
    uint32_t start_cycles = DWT->CYCCNT;
    uint32_t target_cycles = us * cpu_freq_mhz;

    // Handle overflow automatically by using unsigned arithmetic subtraction
    while ((DWT->CYCCNT - start_cycles) < target_cycles)
    {
        // Busy wait
        __NOP(); 
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

/**
 * @brief Legacy/Compatibility function for Timer callback (Not needed for DWT)
 */
void Delay_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    // Unused in DWT mode
    (void)htim;
}
