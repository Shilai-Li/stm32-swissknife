/**
 * @file encoder_motor.c
 * @brief Motor Encoder Driver Implementation
 * @author Standard Implementation
 * @date 2024
 */

#include "encoder_motor.h"

void Encoder_Motor_Init(Encoder_Motor_HandleTypeDef *hmotor, TIM_HandleTypeDef *htim, uint16_t cpr) {
    hmotor->htim = htim;
    hmotor->CPR = cpr;
    hmotor->Inverted = 0;
    
    hmotor->CountPrev = 0;
    hmotor->TotalCount = 0;
    hmotor->LastCount = 0;
    hmotor->LastTime = HAL_GetTick();
    hmotor->SpeedRPM = 0.0f;
    
    // Start Hardware
    HAL_TIM_Encoder_Start(htim, TIM_CHANNEL_ALL);
    __HAL_TIM_SET_COUNTER(htim, 0);
}

void Encoder_Motor_Update(Encoder_Motor_HandleTypeDef *hmotor) {
    int16_t current_cnt = (int16_t)__HAL_TIM_GET_COUNTER(hmotor->htim);
    
    // Calculate Delta (Handles 16-bit wrap-around correctly)
    int16_t delta = current_cnt - hmotor->CountPrev;
    hmotor->CountPrev = current_cnt;
    
    if (hmotor->Inverted) delta = -delta;
    
    hmotor->TotalCount += delta;
}

float Encoder_Motor_GetSpeed(Encoder_Motor_HandleTypeDef *hmotor) {
    // 1. Force an update to get latest count
    Encoder_Motor_Update(hmotor);
    
    // 2. Calculate time delta
    uint32_t now = HAL_GetTick();
    uint32_t dt_ms = now - hmotor->LastTime;
    
    // Avoid division by zero or too frequent calls (noise)
    if (dt_ms == 0) return hmotor->SpeedRPM;
    
    // 3. Calculate count delta
    int64_t count_diff = hmotor->TotalCount - hmotor->LastCount;
    
    // Update State
    hmotor->LastTime = now;
    hmotor->LastCount = hmotor->TotalCount;
    
    // 4. Compute RPM
    // Speed (RPM) = (DeltaCounts / CPR) / (DeltaTime_ms / 60000.0)
    //             = (DeltaCounts * 60000) / (CPR * DeltaTime_ms)
    
    float rpm = (float)(count_diff * 60000) / (float)(hmotor->CPR * dt_ms);
    
    // Simple infinite impulse response (IIR) filter (Low Pass) to smooth speed?
    // Let's implement a weak filter: New = 0.7*New + 0.3*Old
    // hmotor->SpeedRPM = 0.7f * rpm + 0.3f * hmotor->SpeedRPM;
    
    // For raw driver, return raw calculate to allow external PID filter
    hmotor->SpeedRPM = rpm;
    
    return rpm;
}

int64_t Encoder_Motor_GetCount(Encoder_Motor_HandleTypeDef *hmotor) {
    Encoder_Motor_Update(hmotor);
    return hmotor->TotalCount;
}

void Encoder_Motor_Reset(Encoder_Motor_HandleTypeDef *hmotor) {
    __HAL_TIM_SET_COUNTER(hmotor->htim, 0);
    hmotor->CountPrev = 0;
    hmotor->TotalCount = 0;
    hmotor->LastCount = 0;
}
