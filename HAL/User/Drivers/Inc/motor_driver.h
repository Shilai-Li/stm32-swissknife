#ifndef MOTOR_DRIVER_H
#define MOTOR_DRIVER_H

#include "stm32f1xx_hal.h" // 根据您的芯片型号引用对应的HAL库

// 定义电机结构体，方便管理引脚和定时器
typedef struct {
    TIM_HandleTypeDef *htim;    // PWM用的定时器句柄
    uint32_t channel;           // PWM通道
    GPIO_TypeDef *en_port;      // 启动引脚端口
    uint16_t en_pin;            // 启动引脚编号
    GPIO_TypeDef *dir_port;     // 方向引脚端口
    uint16_t dir_pin;           // 方向引脚编号
    uint32_t pwm_period;        // PWM周期，根据定时器配置
} Motor_Handle_t;

// --- 函数声明 ---

// 初始化电机
void Motor_Init(Motor_Handle_t *motor);

// 启动电机
void Motor_Start(Motor_Handle_t *motor);

// 停止电机
void Motor_Stop(Motor_Handle_t *motor);

// 设置速度 (0 - 100%)
void Motor_SetSpeed(Motor_Handle_t *motor, uint8_t duty_percent);

// 设置方向 (1:正转, 0:反转)
void Motor_SetDirection(Motor_Handle_t *motor, uint8_t direction);

#endif
