#ifndef PID_H
#define PID_H

#include <stdint.h>

typedef struct {
    float Kp;
    float Ki;
    float Kd;
    float prevError;
    float prevMeasurement; // 上一次的测量值 (用于微分)
    float integral;
    float dTerm;         // 滤波后的微分项
    float lpfBeta;       // 微分项低通滤波系数 (0-1, 1=不滤波)
    float outputLimit;   // 输出限幅 (绝对值)
    float integralLimit; // 积分限幅 (绝对值)
    float deadZone;      // 死区 (误差绝对值小于此值时输出为0)
} PID_Controller_t;

/**
 * @brief 初始化 PID 控制器
 * @param pid PID 对象指针
 * @param Kp 比例系数
 * @param Ki 积分系数
 * @param Kd 微分系数
 * @param outputLimit 输出限幅 (如 100.0)
 * @param integralLimit 积分限幅 (通常小于输出限幅)
 * @param deadZone 死区范围
 */
void PID_Init(PID_Controller_t *pid, float Kp, float Ki, float Kd, float outputLimit, float integralLimit, float deadZone);

/**
 * @brief 重置 PID 状态 (清除积分和历史误差)
 * @param pid PID 对象指针
 */
void PID_Reset(PID_Controller_t *pid);

/**
 * @brief 计算 PID 输出
 * @param pid PID 对象指针
 * @param target 目标值
 * @param current 当前反馈值
 * @param dt 采样周期 (秒)
 * @return 控制输出量
 */
float PID_Compute(PID_Controller_t *pid, float target, float current, float dt);

#endif // PID_H
