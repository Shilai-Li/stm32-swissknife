#include "pid.h"

void PID_Reset(PIDController *pid) {
    pid->error_prev = 0.0f;
    pid->output_prev = 0.0f;
    pid->integral_prev = 0.0f;
    pid->timestamp_prev = 0;
}

// Initialize PID controller
void PID_Init(PIDController* pid, float p, float i, float d, float limit, float ramp) {
    pid->P = p;
    pid->I = i;
    pid->D = d;
    pid->limit = limit;
    pid->output_ramp = ramp;

    pid->error_prev = 0.0f;
    pid->output_prev = 0.0f;
    pid->integral_prev = 0.0f;
    pid->timestamp_prev = 0; // Time function needs to be adapted to your platform
}

// Main PID calculation function
float PID_Compute(PIDController* pid, float error) {
    // Use fixed sampling period of 10ms (0.01s), since PID control runs in 10ms timer interrupt
    float Ts = 0.01f;

    // Proportional term
    float proportional = pid->P * error;

    // Integral term with anti-windup
    // Only accumulate integral when output is not saturated, or when integral direction is opposite to error direction
    float integral = pid->integral_prev + pid->I * error * Ts;
    // Simple integral limiting
    if (integral > pid->limit) integral = pid->limit;
    else if (integral < -pid->limit) integral = -pid->limit;

    // Derivative term (error derivative, can also use measurement derivative to reduce impact)
    float derivative = (error - pid->error_prev) / Ts;

    // Total output
    float output = proportional + integral + pid->D * derivative;

    // Output limiting
    if (output > pid->limit) output = pid->limit;
    else if (output < -pid->limit) output = -pid->limit;

    // Output rate limit (Ramp) - Optional, protects motor and power supply
    if (pid->output_ramp > 0) {
        float output_rate = (output - pid->output_prev) / Ts;
        if (output_rate > pid->output_ramp)
            output = pid->output_prev + pid->output_ramp * Ts;
        else if (output_rate < -pid->output_ramp)
            output = pid->output_prev - pid->output_ramp * Ts;
    }

    // Save state for next iteration
    pid->integral_prev = integral;
    pid->output_prev = output;
    pid->error_prev = error;

    return output;
}