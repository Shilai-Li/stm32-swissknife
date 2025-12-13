/**
 * @file encoder_knob.c
 * @brief EC11 Rotary Encoder Knob Driver Implementation
 * @author Standard Implementation
 * @date 2024
 */

#include "drivers/encoder_knob.h"
#include <stdlib.h> // for abs

void Encoder_Knob_Init(Encoder_Knob_HandleTypeDef *hknob, TIM_HandleTypeDef *htim) {
    hknob->htim = htim;
    hknob->Inverted = 0;
    hknob->UseVelocity = 0;
    
    hknob->CountRaw = 0;
    hknob->CountPrev = 0;
    hknob->Position = 0;
    hknob->LastTick = HAL_GetTick();
    
    // Start Hardware Encoder
    HAL_TIM_Encoder_Start(htim, TIM_CHANNEL_ALL);
    
    // Reset Counter to center (0x7FFF) or 0? 
    // Best practice: Keep at 0, handle overflow mathematically if needed, 
    // but for knobs, we don't usually overflow standard int16 range quickly.
    // However, to be safe, sometimes we set CNT to mid-range to avoid underflow at 0 immediately.
    // Here we stick to 0 for simplicity, relying on standard roll-over logic.
    __HAL_TIM_SET_COUNTER(htim, 0);
}

int16_t Encoder_Knob_Update(Encoder_Knob_HandleTypeDef *hknob) {
    int16_t delta = 0;
    
    // Read hardware counter using volatile access or macro
    int16_t current_cnt = (int16_t)__HAL_TIM_GET_COUNTER(hknob->htim);
    
    // Calculate hardware delta
    // This logic handles 16-bit timer overflow automatically:
    // e.g. Current=1, Prev=65535(-1) -> 1 - (-1) = 2. 
    // e.g. Current=65535, Prev=0 -> -1 - 0 = -1.
    int16_t hw_delta = current_cnt - hknob->CountPrev;
    hknob->CountPrev = current_cnt;
    
    // EC11 usually generates 2 or 4 counts per physical click (detent) depending on mode (X2 vs X4).
    // User might want to divide this? We return raw counts here, user can divide.
    // Actually, usually we divide by 2 or 4 here to match physical clicks.
    // Let's assume standard X4 mode, usually 4 counts per click for a standard full cycle.
    // But to keep it "Driver" level, raw is safer unless parameterized.
    // Let's return raw delta, let UI handle division (e.g. if(pos % 4 == 0)).
    
    if (hknob->Inverted) hw_delta = -hw_delta;
    
    if (hw_delta != 0) {
        // --- Velocity Acceleration Logic ---
        int16_t step_size = 1;
        
        if (hknob->UseVelocity) {
            uint32_t now = HAL_GetTick();
            uint32_t dt = now - hknob->LastTick;
            hknob->LastTick = now;
            
            // Heuristic: If faster than 50ms per click, accelerate
            // Very simple non-linear map
            if (dt < 20) step_size = 10;
            else if (dt < 50) step_size = 5;
            else if (dt < 100) step_size = 2;
        } else {
            hknob->LastTick = HAL_GetTick(); // Keep tick updated
        }
        
        delta = (hw_delta > 0) ? step_size : -step_size;
        
        // If hw_delta > 1 (missed polling), we might want to amplify?
        // Simplified: just apply direction * step_size * abs(hw_delta)
        // Wait, if we missed 10 counts, we should move 10 potential steps.
        // Let's refine:
        delta = hw_delta; // Base delta
        
        if (hknob->UseVelocity && abs(hw_delta) == 1) { // Only accelerate single continuous movements usually
             // If we really moved fast, hw_delta might be > 1 already.
             // Let's just apply multiplier to the magnitude.
             delta = (hw_delta > 0) ? step_size : -step_size;
        }
        
        hknob->Position += delta;
    }
    
    return delta;
}

int32_t Encoder_Knob_GetPosition(Encoder_Knob_HandleTypeDef *hknob) {
    return hknob->Position;
}

void Encoder_Knob_Reset(Encoder_Knob_HandleTypeDef *hknob) {
    hknob->Position = 0;
    // Don't reset HW counter, just offset logic
}
