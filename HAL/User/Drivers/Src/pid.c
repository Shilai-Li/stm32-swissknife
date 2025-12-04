#include "pid.h"
#include <math.h> // for fabsf

void PID_Init(PID_Controller_t *pid, float Kp, float Ki, float Kd, float outputLimit, float integralLimit, float deadZone) {
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;
    pid->outputLimit = outputLimit;
    pid->integralLimit = integralLimit;
    pid->deadZone = deadZone;
    pid->lpfBeta = 0.1f; // 默认强滤波 (0.1)
    PID_Reset(pid);
}

void PID_Reset(PID_Controller_t *pid) {
    pid->prevError = 0.0f;
    pid->prevMeasurement = 0.0f;
    pid->integral = 0.0f;
    pid->dTerm = 0.0f;
}

float PID_Compute(PID_Controller_t *pid, float target, float current, float dt) {
    float error = target - current;
    
    // 误差死区控制（避免在目标附近震荡）
    if (error > 0 && error < pid->deadZone) error = 0;
    if (error < 0 && error > -pid->deadZone) error = 0;
    
    // 如果在死区内，直接返回0并清空积分
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

    // 微分项 (Derivative on Measurement)
    // d(Error)/dt = d(Target - Measurement)/dt
    // 如果 Target 不变，则 = -d(Measurement)/dt
    // 这样可以避免改变 Target 时产生的微分冲击 (Derivative Kick)
    float rawDerivative = 0.0f;
    if (dt > 0.0001f) {
        rawDerivative = -(current - pid->prevMeasurement) / dt;
    }
    pid->prevMeasurement = current;
    pid->prevError = error; // 仍然记录Error供参考

    // 微分项低通滤波 (Low Pass Filter)
    // dTerm = (1-beta)*old + beta*new
    pid->dTerm = (1.0f - pid->lpfBeta) * pid->dTerm + pid->lpfBeta * rawDerivative;

    // 计算输出
    float output = (pid->Kp * error) + (pid->Ki * pid->integral) + (pid->Kd * pid->dTerm);

    // 输出限幅
    if (output > pid->outputLimit) output = pid->outputLimit;
    if (output < -pid->outputLimit) output = -pid->outputLimit;

    // 【新增】摩擦力死区补偿
    // 如果输出太小，无法克服静摩擦力，直接归零避免抖动
    #define MIN_OUTPUT_THRESHOLD 0.0f  // 暂时禁用摩擦力补偿，排除干扰
    
    if (output > 0 && output < MIN_OUTPUT_THRESHOLD) {
        output = MIN_OUTPUT_THRESHOLD;  // 补偿静摩擦力
    } else if (output < 0 && output > -MIN_OUTPUT_THRESHOLD) {
        output = -MIN_OUTPUT_THRESHOLD;
    }
    // 或者使用摩擦力前馈补偿（更激进）
    // if (output > 0 && output < MIN_OUTPUT_THRESHOLD) {
    //     output = MIN_OUTPUT_THRESHOLD;  // 保证最小驱动力
    // } else if (output < 0 && output > -MIN_OUTPUT_THRESHOLD) {
    //     output = -MIN_OUTPUT_THRESHOLD;
    // }

    return output;
}
