/**
 * @file watchdog.c
 * @brief Watchdog Driver Implementation (Supports IWDG and WWDG)
 */

#include "watchdog.h"

/* ============================================================================
 * IWDG Implementation (Independent Watchdog)
 * ========================================================================= */
#if defined(HAL_IWDG_MODULE_ENABLED)

static IWDG_HandleTypeDef internal_hiwdg;
static IWDG_HandleTypeDef *phiwdg = &internal_hiwdg;

#define LSI_FREQ 40000UL 

void Watchdog_Register(IWDG_HandleTypeDef *hiwdg) {
    if (hiwdg) {
        phiwdg = hiwdg;
    }
}

bool Watchdog_Init(uint32_t timeout_ms) {
    uint32_t prescalers[] = {4, 8, 16, 32, 64, 128, 256};
    uint32_t reg_vals[] = {
        IWDG_PRESCALER_4, IWDG_PRESCALER_8, IWDG_PRESCALER_16, 
        IWDG_PRESCALER_32, IWDG_PRESCALER_64, IWDG_PRESCALER_128, IWDG_PRESCALER_256
    };
    
    uint32_t selected_reg = 0;
    uint32_t reload = 0;
    bool found = false;

    for (int i = 0; i < 7; i++) {
        uint32_t max_ms = (prescalers[i] * 4095UL * 1000UL) / LSI_FREQ;
        if (timeout_ms <= max_ms) {
            selected_reg = reg_vals[i];
            reload = (timeout_ms * LSI_FREQ) / (prescalers[i] * 1000UL);
            if (reload > 4095) reload = 4095;
            if (reload < 1) reload = 1;
            found = true;
            break;
        }
    }

    if (!found) return false;

    phiwdg->Instance = IWDG;
    phiwdg->Init.Prescaler = selected_reg;
    phiwdg->Init.Reload = reload;
    
    return (HAL_IWDG_Init(phiwdg) == HAL_OK);
}

void Watchdog_Feed(void) {
    if (phiwdg) {
        HAL_IWDG_Refresh(phiwdg);
    }
}

bool Watchdog_WasResetByDog(void) {
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST) != RESET) {
        __HAL_RCC_CLEAR_RESET_FLAGS();
        return true;
    }
    return false;
}

/* ============================================================================
 * WWDG Implementation (Window Watchdog)
 * ========================================================================= */
#elif defined(HAL_WWDG_MODULE_ENABLED)

static WWDG_HandleTypeDef *phwwdg = NULL;

void Watchdog_Register(WWDG_HandleTypeDef *hwwdg) {
    phwwdg = hwwdg;
}

bool Watchdog_Init(uint32_t timeout_ms) {
    // WWDG is typically initialized by CubeMX with specific timing
    // Just verify handle is registered
    if (!phwwdg) return false;
    
    // WWDG already initialized by HAL, just start it
    return (HAL_WWDG_Init(phwwdg) == HAL_OK);
}

void Watchdog_Feed(void) {
    if (phwwdg) {
        // WWDG refresh - must be called within the window
        HAL_WWDG_Refresh(phwwdg);
    }
}

bool Watchdog_WasResetByDog(void) {
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_WWDGRST) != RESET) {
        __HAL_RCC_CLEAR_RESET_FLAGS();
        return true;
    }
    return false;
}

/* ============================================================================
 * Stub Implementation (No Watchdog Enabled)
 * ========================================================================= */
#else

bool Watchdog_Init(uint32_t timeout_ms) { (void)timeout_ms; return false; }
void Watchdog_Feed(void) {}
bool Watchdog_WasResetByDog(void) { return false; }

#endif
