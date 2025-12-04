#include "pid.h"
#include <math.h> // for fabsf

void PID_Init(PID_Controller_t *pid, float Kp, float Ki, float Kd, float outputLimit, float integralLimit, float deadZone) {
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;
    pid->outputLimit = outputLimit;
    pid->integralLimit = integralLimit;
    pid->deadZone = deadZone;
    PID_Reset(pid);
}

void PID_Reset(PID_Controller_t *pid) {
    pid->prevError = 0.0f;
    pid->integral = 0.0f;
}

float PID_Compute(PID_Controller_t *pid, float target, float current, float dt) {
    float error = target - current;
    
    // 死区控制
    if (error > 0 && error < pid->deadZone) error = 0;
    if (error < 0 && error > -pid->deadZone) error = 0;
    
    // 如果在死区内，直接返回0并清空积分（可选，防止积分累积）
    if (error == 0.0f) {
        pid->integral = 0.0f;
        pid->prevError = 0.0f;
        return 0.0f;
    }

    // 积分项
    pid->integral += error * dt;
    
    // 积分抗饱和
    if (pid->integral > pid->integralLimit) pid->integral = pid->integralLimit;
    if (pid->integral < -pid->integralLimit) pid->integral = -pid->integralLimit;

    // 微分项
    float derivative = 0.0f;
    if (dt > 0.0001f) { // 防止除以零
        derivative = (error - pid->prevError) / dt;
    }
    pid->prevError = error;

    // 计算输出
    float output = (pid->Kp * error) + (pid->Ki * pid->integral) + (pid->Kd * derivative);

    // 输出限幅
    if (output > pid->outputLimit) output = pid->outputLimit;
    if (output < -pid->outputLimit) output = -pid->outputLimit;

    return output;
}
