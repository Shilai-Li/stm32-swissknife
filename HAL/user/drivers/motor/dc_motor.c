#include "dc_motor.h"

// Initialize motor - Start PWM signal, hardware initialization is done by HAL_Init
void Motor_Init(Motor_Handle_t *motor) {
    // Default stop
    HAL_GPIO_WritePin(motor->en_port, motor->en_pin, GPIO_PIN_RESET);
    // Default to forward direction
    HAL_GPIO_WritePin(motor->dir_port, motor->dir_pin, GPIO_PIN_SET); // Assume high level is forward
    
    // CRITICAL: Set PWM duty cycle to 0 BEFORE starting PWM
    __HAL_TIM_SET_COMPARE(motor->htim, motor->channel, 0);
    
    // Start PWM signal
    HAL_TIM_PWM_Start(motor->htim, motor->channel);
    
    // CRITICAL FIX: For advanced timers (TIM1/TIM8), enable Main Output Enable (MOE)
    // Without MOE enabled, the PWM output pin will be in high-impedance state
    // which can cause unpredictable behavior with motor drivers!
    if (motor->htim->Instance == TIM1
#ifdef TIM8
        || motor->htim->Instance == TIM8
#endif
    ) {
        __HAL_TIM_MOE_ENABLE(motor->htim);
    }
    
    motor->total_count = 0;
    motor->last_counter = 0;
}

void Motor_Encoder_Init(Motor_Handle_t *motor) {
    // Start hardware encoder mode
    HAL_TIM_Encoder_Start(motor->htim_enc, TIM_CHANNEL_ALL);
}

int32_t Motor_GetEncoderCount(Motor_Handle_t *motor) {
    // Read current timer value
    uint16_t current_counter = __HAL_TIM_GET_COUNTER(motor->htim_enc);
    
    // Calculate difference (note: int16_t, uses overflow to handle wrap-around)
    int16_t diff = (int16_t)(current_counter - motor->last_counter);
    
    // Update total count
    motor->total_count += diff;
    motor->last_counter = current_counter;
    
    return motor->total_count;
}

void Motor_ResetEncoderCount(Motor_Handle_t *motor, int32_t value) {
    __HAL_TIM_SET_COUNTER(motor->htim_enc, 0); // Clear hardware counter
    motor->last_counter = 0;
    motor->total_count = value; // Set logical counter to target value
}

void Motor_Start(Motor_Handle_t *motor) {
    // Enable motor
    HAL_GPIO_WritePin(motor->en_port, motor->en_pin, GPIO_PIN_SET);
}

void Motor_Stop(Motor_Handle_t *motor) {
    // First, set PWM to 0 to ensure no PWM output
    __HAL_TIM_SET_COMPARE(motor->htim, motor->channel, 0);
    
    // Disable motor via enable pin
    HAL_GPIO_WritePin(motor->en_port, motor->en_pin, GPIO_PIN_RESET);
}

void Motor_SetSpeed(Motor_Handle_t *motor, uint8_t duty_percent) {
    // Limit range 0-100
    if (duty_percent > 100) duty_percent = 100;

    // If ARR is 99, set CCR = duty_percent directly
    // If ARR is not 99, calculate proportionally: (ARR+1) * duty_percent / 100
    // General calculation formula
    uint32_t compare_value = (motor->pwm_period + 1) * duty_percent / 100;
    __HAL_TIM_SET_COMPARE(motor->htim, motor->channel, compare_value);
}

void Motor_SetDirection(Motor_Handle_t *motor, uint8_t direction) {
    if (direction == 1) {
        // Forward: set to high level
        HAL_GPIO_WritePin(motor->dir_port, motor->dir_pin, GPIO_PIN_SET);
    } else {
        // Reverse: set to low level
        HAL_GPIO_WritePin(motor->dir_port, motor->dir_pin, GPIO_PIN_RESET);
    }
}