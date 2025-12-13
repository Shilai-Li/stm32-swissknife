/**
 * @file hc_sr04.h
 * @brief HC-SR04 Ultrasonic Sensor Driver Header File
 * @author Standard Implementation
 * @date 2024
 */

#ifndef __HC_SR04_H
#define __HC_SR04_H

#include "main.h"

#ifndef __STM32F1xx_HAL_TIM_H
#include "stm32f1xx_hal.h"
#endif

typedef struct {
    TIM_HandleTypeDef *htim;       // Timer handle for microsecond delay/measurement
    GPIO_TypeDef      *TrigPort;   // Trigger Pin Port
    uint16_t           TrigPin;    // Trigger Pin
    GPIO_TypeDef      *EchoPort;   // Echo Pin Port
    uint16_t           EchoPin;    // Echo Pin
    uint32_t           TimeoutUs;  // Timeout in microseconds (e.g. 25000 for ~4m)
} HCSR04_HandleTypeDef;

/* Function Prototypes */

/**
 * @brief Initialize HC-SR04 Driver
 * @param hsensor Handle
 * @param htim Timer Handle (Must be configured to tick at 1MHz / 1us resolution preferred)
 *             If timer is not 1MHz, user must adjust reading calculation or prescaler.
 * @param trig_port Trigger GPIO Port
 * @param trig_pin Trigger GPIO Pin
 * @param echo_port Echo GPIO Port
 * @param echo_pin Echo GPIO Pin
 */
void HCSR04_Init(HCSR04_HandleTypeDef *hsensor, TIM_HandleTypeDef *htim, 
                 GPIO_TypeDef *trig_port, uint16_t trig_pin,
                 GPIO_TypeDef *echo_port, uint16_t echo_pin);

/**
 * @brief Read distance in centimeters
 * @return Distance in cm, or -1.0f on Error/Timeout
 */
float HCSR04_Read(HCSR04_HandleTypeDef *hsensor);

#endif // __HC_SR04_H
