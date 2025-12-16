/**
 * @file stepper_motor.c
 * @brief Open Loop Stepper Motor Driver Implementation
 * @author Standard Implementation (Based on AccelStepper Logic)
 * @date 2024
 */

#include "drivers/stepper_motor.h"
#include <math.h>
#include <stdlib.h>

// Helper to get microsecond timestamp handling wrap-around logic roughly
// We assume timer is 16-bit or 32-bit.
// The driver logic handles interval calculation using unsigned sub.
static inline unsigned long Stepper_Micros(Stepper_HandleTypeDef *h) {
    return __HAL_TIM_GET_COUNTER(h->htim);
}

void Stepper_Init(Stepper_HandleTypeDef *hstepper, 
                  GPIO_TypeDef *step_port, uint16_t step_pin,
                  GPIO_TypeDef *dir_port, uint16_t dir_pin,
                  GPIO_TypeDef *en_port, uint16_t en_pin,
                  TIM_HandleTypeDef *htim)
{
    hstepper->StepPort = step_port; hstepper->StepPin = step_pin;
    hstepper->DirPort = dir_port;   hstepper->DirPin = dir_pin;
    hstepper->EnPort = en_port;     hstepper->EnPin = en_pin;
    hstepper->htim = htim;
    
    // Default Config
    hstepper->MaxSpeed = 800.0f;
    hstepper->Acceleration = 400.0f;
    hstepper->MinPulseWidth = 2.0f; // 2us
    hstepper->EnPolarity = 0; // Active Low
    
    hstepper->CurrentPos = 0;
    hstepper->TargetPos = 0;
    hstepper->Speed = 0.0f;
    hstepper->n = 0;
    
    // Init GPIO output states
    HAL_GPIO_WritePin(hstepper->StepPort, hstepper->StepPin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(hstepper->DirPort, hstepper->DirPin, GPIO_PIN_RESET);
    // Disable by default? Or leave user to enable.
    // Let's Disable (Release Torque) initially.
    Stepper_Enable(hstepper, 0); 

    // Start Timer
    HAL_TIM_Base_Start(htim);
}

void Stepper_SetSpeedConfig(Stepper_HandleTypeDef *hstepper, float max_speed, float acceleration) {
    if (max_speed < 0.0f) max_speed = -max_speed;
    if (acceleration < 0.0f) acceleration = -acceleration;
    
    hstepper->MaxSpeed = max_speed;
    hstepper->Acceleration = acceleration;
}

void Stepper_Enable(Stepper_HandleTypeDef *hstepper, uint8_t enable) {
    if (hstepper->EnPort == NULL) return;
    
    GPIO_PinState pin_state;
    if (hstepper->EnPolarity == 0) { // Active Low (Low = Enable)
        pin_state = enable ? GPIO_PIN_RESET : GPIO_PIN_SET;
    } else { // Active High
        pin_state = enable ? GPIO_PIN_SET : GPIO_PIN_RESET;
    }
    HAL_GPIO_WritePin(hstepper->EnPort, hstepper->EnPin, pin_state);
}

// Compute new step interval based on acceleration physics
static void Stepper_ComputeNewSpeed(Stepper_HandleTypeDef *h) {
    long distance_to_go = h->TargetPos - h->CurrentPos;
    
    long steps_to_stop = (long)((h->Speed * h->Speed) / (2.0f * h->Acceleration)); 
    
    if (distance_to_go == 0 && steps_to_stop <= 1) {
        // We are there (or almost)
        h->StepInterval = 0;
        h->Speed = 0.0f;
        h->n = 0;
        return;
    }
    
    if (distance_to_go > 0) {
        // Need to move forward
        if (h->n > 0) {
            // Currently accelerating or cruising positive
            if ((h->n >= steps_to_stop) || (h->Speed == h->MaxSpeed)) {
                // Need to decelerate or cruise
            }
        } else if (h->n < 0) {
            // Currently moving backward, need to stop first
            h->n = -steps_to_stop;
        }
    } else if (distance_to_go < 0) {
        // Need to move backward
         if (h->n > 0) {
            // Currently moving forward, need to stop
             h->n = steps_to_stop;
         } else if (h->n < 0) {
             // Currently moving backward
         }
    }
    
    // --- Determine state (Accel/Decel/Const) ---
    // n is the step number since start of acceleration.
    
    if (distance_to_go == 0) {
        // Decelerate to stop
        h->Speed = 0.0f; 
        h->n = 0;
        h->StepInterval = 0;
        return;
    }
    
    // Calculate Speed and Period
    // Algorithm: c0 = sqrt(2/a) * 1e6
    // cn = c(n-1) - (2*c(n-1)) / (4n + 1)
    
    if (h->n == 0) {
        // First step from stop
        h->c0 = sqrtf(2.0f / h->Acceleration) * 1000000.0f; // us
        h->cn = h->c0;
        h->cmin = 1000000.0f / h->MaxSpeed; // Min delay (Max speed)
        
        // Direction
        if (distance_to_go > 0) h->Speed = 1.0f; // Only sign matters for initial calc
        else h->Speed = -1.0f;
        
        h->n = 1; // Begin accel
    } else {
        // Subsequent step
        // Calculate new cn from old cn
        // c_new = c_old - (2 * c_old) / (4 * n + 1)
        float new_cn = h->cn - ((2.0f * h->cn) / (4.0f * (float)labs(h->n) + 1.0f));
        h->cn = new_cn;
         
        // Clamp to Max Speed
        if (h->cn < h->cmin) {
            h->cn = h->cmin; 
             // We are at max speed, don't increment n (keep it const to stay at max speed profile??)
             // Actually AccelStepper trick ensures n doesn't grow infinitely when cruising
        } 
    }
    
    h->StepInterval = (unsigned long)h->cn;
    // Speed update for direction
    if (distance_to_go > 0) h->Speed = (1e6f / h->cn);
    else h->Speed = -(1e6f / h->cn);
    
    // Deceleration Detection
    // If we are close to target, we should invert n logic or such?
    // AccelStepper Logic simplifies this: 
    // It compares steps_to_stop vs distance_to_go. If distance <= steps_to_stop, we must decel.
    
    if ((distance_to_go > 0 && distance_to_go <= steps_to_stop) || 
        (distance_to_go < 0 && distance_to_go >= -steps_to_stop)) 
    {
        // Decelerating
        h->n = -labs(h->n); // Negative n implies deceleration phase in math
        // Recalc cn for decel? 
        // Logic: if n is negative, the Taylor series formula works in reverse?
        // Actually: c_new = c_old - (2 * c_old) / (4 * n + 1). If n is negative, delay grows!
    } else {
        // Accelerating or Cruising
        // Check if we hit max speed (already handled by cmin clamp)
    }
}


void Stepper_MoveTo(Stepper_HandleTypeDef *hstepper, long absolute_pos) {
    if (hstepper->TargetPos != absolute_pos) {
        hstepper->TargetPos = absolute_pos;
        // Recalculate immediate needs?
        // If stopped, n=0. If running, curve adjusts.
        if (hstepper->n == 0) Stepper_ComputeNewSpeed(hstepper);
    }
}

void Stepper_Move(Stepper_HandleTypeDef *hstepper, long relative_steps) {
    Stepper_MoveTo(hstepper, hstepper->CurrentPos + relative_steps);
}

uint8_t Stepper_Run(Stepper_HandleTypeDef *hstepper) {
    // 1. Check if we need to step
    if (hstepper->CurrentPos == hstepper->TargetPos && hstepper->Speed == 0.0f) {
        return 0; // Stopped
    }
    
    // if n=0 but target!=current, we need to kickstart
    if (hstepper->IsRunning == 0 && hstepper->TargetPos != hstepper->CurrentPos) {
        Stepper_ComputeNewSpeed(hstepper);
        hstepper->LastStepTime = Stepper_Micros(hstepper);
        hstepper->IsRunning = 1;
    }

    unsigned long time = Stepper_Micros(hstepper);
    // Handle timer wrap around (16-bit Timer wrap at 65535us = 65ms). 
    // Unsigned subtraction (time - last) works correctly across one wrap.
    // However, if StepInterval > 65ms, logic fails. 
    // Constraint: Max step delay < Timer Period. 
    // At 1MHz, 16-bit is 65ms. If speed < 15 steps/sec, we fail.
    // Solution: User should use 32-bit timer or faster min speed. Or we handle overflow count.
    // For now, assume intervals are reasonable (<50ms).
    
    if ((time - hstepper->LastStepTime) >= hstepper->StepInterval) {
        // Time to Step!
        if (hstepper->CurrentPos == hstepper->TargetPos) {
             // Correction: Might settle final microstep
             hstepper->IsRunning = 0;
             hstepper->Speed = 0.0f;
             hstepper->n = 0;
             return 0; 
        }
        
        // DIR
        if (hstepper->TargetPos > hstepper->CurrentPos) {
            HAL_GPIO_WritePin(hstepper->DirPort, hstepper->DirPin, GPIO_PIN_SET); // CW?
            hstepper->CurrentPos++;
        } else {
            HAL_GPIO_WritePin(hstepper->DirPort, hstepper->DirPin, GPIO_PIN_RESET); // CCW?
            hstepper->CurrentPos--;
        }
        
        // STEP Pulse
        HAL_GPIO_WritePin(hstepper->StepPort, hstepper->StepPin, GPIO_PIN_SET);
        // Small delay for setup time (1-2us)
        // Since we are running loop, assume minimal overhead is enough or blocking tiny delay
        // A dummy instruction loop
        for(volatile int i=0; i<8; i++); 
        
        HAL_GPIO_WritePin(hstepper->StepPort, hstepper->StepPin, GPIO_PIN_RESET);
        
        hstepper->LastStepTime = time;
        
        // Recalculate for next step
        Stepper_ComputeNewSpeed(hstepper);
    }
    
    return 1; // Still running
}

void Stepper_RunToPosition(Stepper_HandleTypeDef *hs) {
    while (Stepper_Run(hs));
}

void Stepper_SetHome(Stepper_HandleTypeDef *hs) {
    hs->CurrentPos = 0;
    hs->TargetPos = 0;
}
