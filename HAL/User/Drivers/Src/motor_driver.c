#include "motor_driver.h"

// 初始化：其实HAL_Init已经做完了硬件初始化，这里主要是启动PWM
void Motor_Init(Motor_Handle_t *motor) {
    // 默认停止
    HAL_GPIO_WritePin(motor->en_port, motor->en_pin, GPIO_PIN_RESET);
    // 默认正转
    HAL_GPIO_WritePin(motor->dir_port, motor->dir_pin, GPIO_PIN_SET); // 假设高电平是正转
    // 启动PWM基准信号 (TIM1 PWM)
    HAL_TIM_PWM_Start(motor->htim, motor->channel);
    
    motor->total_count = 0;
    motor->last_counter = 0;
}

void Motor_Encoder_Init(Motor_Handle_t *motor) {
    // 启动硬件编码器模式 (TIM2)
    HAL_TIM_Encoder_Start(motor->htim_enc, TIM_CHANNEL_ALL);
}

int32_t Motor_GetEncoderCount(Motor_Handle_t *motor) {
    // 读取当前定时器值
    uint16_t current_counter = __HAL_TIM_GET_COUNTER(motor->htim_enc);
    
    // 计算差值 (注意是 int16_t，利用溢出特性自动处理回绕)
    int16_t diff = (int16_t)(current_counter - motor->last_counter);
    
    // 更新总计数
    motor->total_count += diff;
    motor->last_counter = current_counter;
    
    return motor->total_count;
}

void Motor_ResetEncoderCount(Motor_Handle_t *motor, int32_t value) {
    __HAL_TIM_SET_COUNTER(motor->htim_enc, 0); // 硬件计数器清零
    motor->last_counter = 0;
    motor->total_count = value; // 逻辑计数器设为目标值
}

void Motor_Start(Motor_Handle_t *motor) {
    // 拉高绿色线，启动
    HAL_GPIO_WritePin(motor->en_port, motor->en_pin, GPIO_PIN_SET);
}

void Motor_Stop(Motor_Handle_t *motor) {
    // 拉低绿色线，停止
    HAL_GPIO_WritePin(motor->en_port, motor->en_pin, GPIO_PIN_RESET);
}

void Motor_SetSpeed(Motor_Handle_t *motor, uint8_t duty_percent) {
    // 限制范围 0-100
    if (duty_percent > 100) duty_percent = 100;

    // 假设ARR是99，这里直接设置 CCR = duty_percent
    // 如果ARR不是99，这里需要按比例计算: (ARR+1) * duty_percent / 100
    // 通用的计算公式
    uint32_t compare_value = (motor->pwm_period + 1) * duty_percent / 100;
    __HAL_TIM_SET_COMPARE(motor->htim, motor->channel, compare_value);
}

void Motor_SetDirection(Motor_Handle_t *motor, uint8_t direction) {
    if (direction == 1) {
        // 正转：恢复为高电平
        HAL_GPIO_WritePin(motor->dir_port, motor->dir_pin, GPIO_PIN_SET);
    } else {
        // 反转：恢复为低电平
        HAL_GPIO_WritePin(motor->dir_port, motor->dir_pin, GPIO_PIN_RESET);
    }
}
