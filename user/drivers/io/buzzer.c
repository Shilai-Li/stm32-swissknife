/**
 * @file buzzer.c
 * @brief Enhanced Buzzer Driver Implementation
 */

#include "buzzer.h"

void Buzzer_Init(Buzzer_Handle_t *h, TIM_HandleTypeDef *htim, uint32_t channel) {
    h->htim = htim;
    h->channel = channel;
    h->gpio_port = NULL;
    h->is_playing = false;
    
    // Ensure PWM stopped
    if (h->htim) {
        HAL_TIM_PWM_Stop(h->htim, h->channel);
    }
}

void Buzzer_InitGPIO(Buzzer_Handle_t *h, GPIO_TypeDef *port, uint16_t pin) {
    h->htim = NULL;
    h->gpio_port = port;
    h->gpio_pin = pin;
    h->is_playing = false;
    
    HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
}

// Helper to set PWM Frequency
static void SetPWM(Buzzer_Handle_t *h, uint32_t freq) {
    if (!h->htim || freq == 0) return;
    
    // We need to calculate ARR based on Timer Clock
    // Assumption: Timer Prescaler is set to 1MHz tick (1us) or similar in CubeMX?
    // Usually easier: Let's assume Timer Clock is PCLK (e.g., 72MHz or 84MHz)
    // To make this generic, we ideally need `HAL_RCC_GetPCLK1Freq()`.
    // But modifying Prescaler on the fly is tricky if other channels used.
    // 
    // Standard approach: Prescaler fixed (e.g. to 1MHz -> 1us tick), change ARR.
    // 1MHz Tick -> ARR = 1000000 / freq - 1.
    
    /* 
       IMPORTANT: For this driver to work generically, PLEASE config CubeMX Timer 
       Prescaler such that the counter clock is 1 MHz (1 us precision).
       Example: 72MHz -> PSC = 71.
    */
    
    uint32_t period = 1000000 / freq;
    if (period < 2) period = 2; // Safety
    
    __HAL_TIM_SET_AUTORELOAD(h->htim, period - 1);
    __HAL_TIM_SET_COMPARE(h->htim, h->channel, period / 2); // 50% Duty Cycle
    
    // Force Update to apply ARR change immediately
    // HAL_TIM_GenerateEvent(h->htim, TIM_EVENTSOURCE_UPDATE); 
    // ^ Be careful, this might generate interrupt if enabled.
    
    // Direct register access is often safer for glitch-free update if Preload enabled.
    // Assuming Preload Enable in CubeMX.
}

void Buzzer_Tone(Buzzer_Handle_t *h, uint32_t frequency, uint32_t duration_ms) {
    if (frequency == 0) {
        Buzzer_Stop(h);
        return;
    }

    if (h->htim) {
        // Passive: Set PWM
        SetPWM(h, frequency);
        HAL_TIM_PWM_Start(h->htim, h->channel);
    } else if (h->gpio_port) {
        // Active: Just ON
        HAL_GPIO_WritePin(h->gpio_port, h->gpio_pin, GPIO_PIN_SET);
    }
    
    h->is_playing = true;
    if (duration_ms > 0) {
        h->stop_time = HAL_GetTick() + duration_ms;
    } else {
        h->stop_time = 0; // Infinite
    }
}

void Buzzer_Stop(Buzzer_Handle_t *h) {
    if (h->htim) {
        HAL_TIM_PWM_Stop(h->htim, h->channel);
    } else if (h->gpio_port) {
        HAL_GPIO_WritePin(h->gpio_port, h->gpio_pin, GPIO_PIN_RESET);
    }
    h->is_playing = false;
}

void Buzzer_Loop(Buzzer_Handle_t *h) {
    if (h->is_playing && h->stop_time > 0) {
        if (HAL_GetTick() >= h->stop_time) {
            Buzzer_Stop(h);
        }
    }
}

void Buzzer_PlayMelody(Buzzer_Handle_t *h, const uint32_t *melody, const uint32_t *durations, uint32_t length) {
    for (uint32_t i = 0; i < length; i++) {
        uint32_t note = melody[i];
        uint32_t duration = durations[i];
        
        if (note == 0) {
            Buzzer_Stop(h);
        } else {
            Buzzer_Tone(h, note, 0); // 0 = infinite (we control timing here)
        }
        
        HAL_Delay(duration);
        
        // Slight gap between notes to distinguish them
        Buzzer_Stop(h);
        HAL_Delay(20); 
    }
    Buzzer_Stop(h);
}
