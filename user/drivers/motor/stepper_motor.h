/**
 * @file stepper_motor.h
 * @brief Open Loop Stepper Motor Driver with Trapezoidal Acceleration
 * @author Standard Implementation
 * @date 2024
 */

#ifndef __STEPPER_MOTOR_H
#define __STEPPER_MOTOR_H

#include "main.h"

#ifndef __STM32F1xx_HAL_TIM_H
#include "stm32f1xx_hal.h"
#endif

typedef struct {
    // Hardware Config
    GPIO_TypeDef *StepPort;
    uint16_t      StepPin;
    GPIO_TypeDef *DirPort;
    uint16_t      DirPin;
    GPIO_TypeDef *EnPort;   // Optional (can be NULL)
    uint16_t      EnPin;
    uint8_t       EnPolarity; // 0=Active Low (Low=Enable), 1=Active High
    
    TIM_HandleTypeDef *htim; // Timer for microsecond timestamping (Prescaler set to 1us/tick)

    // Motion Settings
    float    MaxSpeed;       // Steps per second
    float    Acceleration;   // Steps per second^2
    float    MinPulseWidth;  // Minimum pulse width in microseconds (default 2)

    // State (Private-ish)
    long     CurrentPos;     // Current Position in steps
    long     TargetPos;      // Target Position in steps
    
    float    Speed;          // Current Speed (+/-)
    unsigned long StepInterval; // Delay between steps in us
    unsigned long LastStepTime; // Time of last step (timer counter)
    
    long     n;              // Step counter for acceleration calc
    float    c0;             // Initial step delay
    float    cn;             // Current step delay
    float    cmin;           // Min step delay (at max speed)
    
    uint8_t  IsRunning;
} Stepper_HandleTypeDef;

/* Function Prototypes */

/**
 * @brief Initialize Stepper Driver
 * @param hstepper Handle
 * @param step_port STEP Port
 * @param step_pin  STEP Pin
 * @param dir_port  DIR Port
 * @param dir_pin   DIR Pin
 * @param en_port   ENABLE Port (NULL if not used)
 * @param en_pin    ENABLE Pin
 * @param htim      Timer Handle (Must run at 1MHz / 1us precision)
 */
void Stepper_Init(Stepper_HandleTypeDef *hstepper, 
                  GPIO_TypeDef *step_port, uint16_t step_pin,
                  GPIO_TypeDef *dir_port, uint16_t dir_pin,
                  GPIO_TypeDef *en_port, uint16_t en_pin,
                  TIM_HandleTypeDef *htim);

/**
 * @brief Set Max Speed and Acceleration
 * @param max_speed Steps per second (e.g. 1000.0)
 * @param acceleration Steps per second^2 (e.g. 500.0)
 */
void Stepper_SetSpeedConfig(Stepper_HandleTypeDef *hstepper, float max_speed, float acceleration);

/**
 * @brief Set Enable Pin State
 * @param enable 1=Enable Motor (Torque), 0=Disable (Free)
 */
void Stepper_Enable(Stepper_HandleTypeDef *hstepper, uint8_t enable);

/**
 * @brief Set Target Position (Absolute)
 * @param absolute_pos Target position in steps
 */
void Stepper_MoveTo(Stepper_HandleTypeDef *hstepper, long absolute_pos);

/**
 * @brief Move Relative (Incremental)
 * @param relative_steps Steps to move (+/-)
 */
void Stepper_Move(Stepper_HandleTypeDef *hstepper, long relative_steps);

/**
 * @brief Non-Blocking Run Function.
 *        MUST be called frequently in main loop (faster than max stepping speed).
 * @return 1 if motor is still running to target, 0 if arrived
 */
uint8_t Stepper_Run(Stepper_HandleTypeDef *hstepper);

/**
 * @brief Blocking Run Function.
 *        Returns only when target is reached.
 */
void Stepper_RunToPosition(Stepper_HandleTypeDef *hstepper);

/**
 * @brief Set current position as 0 (Home)
 */
void Stepper_SetHome(Stepper_HandleTypeDef *hstepper);

#endif // __STEPPER_H
