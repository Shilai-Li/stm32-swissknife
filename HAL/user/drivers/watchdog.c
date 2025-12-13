/**
 * @file watchdog.c
 * @brief Independent Watchdog (IWDG) Driver Implementation
 */

#include "watchdog.h"

#include "watchdog.h"

#ifdef HAL_IWDG_MODULE_ENABLED

/* Global Handle */
static IWDG_HandleTypeDef hiwdg;

/* LSI Frequency is typically 40kHz (range 30-60kHz) on STM32F1 */
#define LSI_FREQ 40000UL 

bool Watchdog_Init(uint32_t timeout_ms) {
    // IWDG runs on LSI.
    // Timeout = (Prescaler * Reload) / LSI_Freq
    // Reload max is 4095 (12-bit)
    
    // We want to find smallest Prescaler that fits the timeout_ms
    // Choices: 4, 8, 16, 32, 64, 128, 256
    
    uint32_t prescalers[] = {4, 8, 16, 32, 64, 128, 256};
    uint32_t reg_vals[] = {
        IWDG_PRESCALER_4, IWDG_PRESCALER_8, IWDG_PRESCALER_16, 
        IWDG_PRESCALER_32, IWDG_PRESCALER_64, IWDG_PRESCALER_128, IWDG_PRESCALER_256
    };
    
    uint32_t selected_pre = 0;
    uint32_t selected_reg = 0;
    uint32_t reload = 0;
    bool found = false;

    for (int i = 0; i < 7; i++) {
        // Calculate max timeout for this prescaler
        // Max Time = (Prescaler * 4095) / 40000 * 1000
        uint32_t max_ms = (prescalers[i] * 4095UL * 1000UL) / LSI_FREQ;
        
        if (timeout_ms <= max_ms) {
            selected_pre = prescalers[i];
            selected_reg = reg_vals[i];
            found = true;
            break;
        }
    }

    if (!found) {
        return false; // Timeout too long (> ~26 sec)
    }

    // specific reload value
    // Reload = (Timeout_ms * LSI) / (Prescaler * 1000)
    reload = (timeout_ms * LSI_FREQ) / (selected_pre * 1000UL);
    if (reload > 4095) reload = 4095;
    if (reload < 1) reload = 1; // Safety

    // Initialize HAL
    hiwdg.Instance = IWDG;
    hiwdg.Init.Prescaler = selected_reg;
    hiwdg.Init.Reload = reload;
    
    if (HAL_IWDG_Init(&hiwdg) != HAL_OK) {
        return false;
    }

    return true;
}

void Watchdog_Feed(void) {
    HAL_IWDG_Refresh(&hiwdg);
}

bool Watchdog_WasResetByDog(void) {
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST) != RESET) {
        // Clear reset flags to detect next time correctly
        __HAL_RCC_CLEAR_RESET_FLAGS();
        return true;
    }
    return false;
}

#else

// Dummy Stubs if module not enabled (to avoid link errors)
bool Watchdog_Init(uint32_t timeout_ms) { return false; }
void Watchdog_Feed(void) {}
bool Watchdog_WasResetByDog(void) { return false; }

#endif

