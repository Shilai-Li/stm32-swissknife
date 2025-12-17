/**
 * @file servo_motor.c
 * @brief RC Servo Motor Driver Implementation
 * @author Standard Implementation
 * @date 2024
 */

#include "servo_motor.h"

// Helper to convert us to timer ticks
// We need to know the Timer Clock Frequency to convert US to CCR value accurately.
// However, passing PCLK freq is annoying. 
// A robust way: 
// 1. Assume Timer Prescaler is set to 1MHz (1us tick). Then CCR = us. This is BEST PRACTICE for Servo.
// 2. Or, we assume ARR = 20000 (for 20ms) -> 1 tick = 1us.
// We will assume 1 tick = 1 microsecond. This puts burden on CubeMX config but simplifies driver speed.

void Servo_Motor_Init(Servo_Motor_HandleTypeDef *hservo, TIM_HandleTypeDef *htim, uint32_t channel) {
    hservo->htim = htim;
    hservo->Channel = channel;
    
    // Default standard limits
    hservo->MinPulse = 500;
    hservo->MaxPulse = 2500;
    hservo->MaxAngle = 180.0f;
    hservo->CurrentAngle = 0.0f;
    
    // Start PWM signal
    HAL_TIM_PWM_Start(hservo->htim, hservo->Channel);
    
    // Initial Position (0 or Middle?)
    // Let's go to 0
    Servo_Motor_SetAngle(hservo, 0.0f);
}

void Servo_Motor_SetLimits(Servo_Motor_HandleTypeDef *hservo, uint16_t min_us, uint16_t max_us, float max_angle) {
    hservo->MinPulse = min_us;
    hservo->MaxPulse = max_us;
    hservo->MaxAngle = max_angle;
}

void Servo_Motor_WriteMicroseconds(Servo_Motor_HandleTypeDef *hservo, uint16_t pulse_us) {
    // Clamp values
    if (pulse_us < hservo->MinPulse) pulse_us = hservo->MinPulse;
    if (pulse_us > hservo->MaxPulse) pulse_us = hservo->MaxPulse;
    
    // Write to Register
    // Assuming Prescaler set for 1us ticks.
    __HAL_TIM_SET_COMPARE(hservo->htim, hservo->Channel, pulse_us);
}

void Servo_Motor_SetAngle(Servo_Motor_HandleTypeDef *hservo, float angle) {
    if (angle < 0.0f) angle = 0.0f;
    if (angle > hservo->MaxAngle) angle = hservo->MaxAngle;
    
    hservo->CurrentAngle = angle;
    
    // Map Angle to Pulse Width
    // Pulse = Min + (Angle / MaxAngle) * (Max - Min)
    float ratio = angle / hservo->MaxAngle;
    uint16_t pulse = hservo->MinPulse + (uint16_t)(ratio * (hservo->MaxPulse - hservo->MinPulse));
    
    Servo_Motor_WriteMicroseconds(hservo, pulse);
}
