#ifndef PID_H
#define PID_H

#include <stdint.h>

typedef struct {
    float P;        // Proportional gain
    float I;        // Integral gain
    float D;        // Derivative gain
    float output_ramp; // Output rate limit (optional, prevents current surge)
    float limit;    // Output limit (e.g., maximum PWM value)

    float error_prev;   // Previous error
    float output_prev;  // Previous output
    float integral_prev;// Previous integral value
    unsigned long timestamp_prev; // Previous calculation timestamp (us or ms)
} PIDController;

// Initialize PID controller
void PID_Init(PIDController* pid, float p, float i, float d, float limit, float ramp);

// Main PID calculation function
float PID_Compute(PIDController* pid, float error);

// Reset PID controller
void PID_Reset(PIDController* pid);

#endif // PID_H
