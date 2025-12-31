/**
 * @file hc_sr04.h
 * @brief HC-SR04 Ultrasonic Sensor Driver Header File
 */

#ifndef __HC_SR04_H
#define __HC_SR04_H

#include "main.h"

/* Forward Declaration */
typedef struct HCSR04_Handle_s HCSR04_HandleTypeDef;
typedef void (*HCSR04_ErrorCallback)(HCSR04_HandleTypeDef *dev);

struct HCSR04_Handle_s {
    GPIO_TypeDef      *TrigPort;
    uint16_t           TrigPin;
    GPIO_TypeDef      *EchoPort;
    uint16_t           EchoPin;
    uint32_t           TimeoutUs;
    
    /* Stats */
    volatile uint32_t error_cnt;
    volatile uint32_t success_cnt;
    volatile uint32_t timeout_cnt;
    
    /* Callback */
    HCSR04_ErrorCallback error_cb;
};

void HCSR04_SetErrorCallback(HCSR04_HandleTypeDef *hsensor, HCSR04_ErrorCallback cb);

/* Function Prototypes */

/**
 * @brief Initialize HC-SR04 Driver
 * @param hsensor Handle
 * @param trig_port Trigger GPIO Port
 * @param trig_pin Trigger GPIO Pin
 * @param echo_port Echo GPIO Port
 * @param echo_pin Echo GPIO Pin
 */
void HCSR04_Init(HCSR04_HandleTypeDef *hsensor, 
                 GPIO_TypeDef *trig_port, uint16_t trig_pin,
                 GPIO_TypeDef *echo_port, uint16_t echo_pin);

/**
 * @brief Read distance in centimeters
 * @return Distance in cm, or -1.0f on Error/Timeout
 */
float HCSR04_Read(HCSR04_HandleTypeDef *hsensor);

#endif // __HC_SR04_H
