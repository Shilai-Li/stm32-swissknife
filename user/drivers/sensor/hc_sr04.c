#include "hc_sr04.h"

#include <stdbool.h>

#include "delay.h"

static void HCSR04_HandleError(HCSR04_HandleTypeDef *dev, bool timeout) {
    if (dev) {
        dev->error_cnt++;
        if (timeout) dev->timeout_cnt++;
        if (dev->error_cb) {
            dev->error_cb(dev);
        }
    }
}

void HCSR04_SetErrorCallback(HCSR04_HandleTypeDef *hsensor, HCSR04_ErrorCallback cb) {
    if (hsensor) {
        hsensor->error_cb = cb;
    }
}

void HCSR04_Init(HCSR04_HandleTypeDef *hsensor, 
                 GPIO_TypeDef *trig_port, uint16_t trig_pin,
                 GPIO_TypeDef *echo_port, uint16_t echo_pin) 
{
    if (!hsensor) return;

    hsensor->TrigPort = trig_port;
    hsensor->TrigPin = trig_pin;
    hsensor->EchoPort = echo_port;
    hsensor->EchoPin = echo_pin;
    hsensor->TimeoutUs = 30000; // 30ms default
    
    hsensor->error_cnt = 0;
    hsensor->success_cnt = 0;
    hsensor->timeout_cnt = 0;
    hsensor->error_cb = NULL;
    
    // Ensure Trigger is Low
    HAL_GPIO_WritePin(hsensor->TrigPort, hsensor->TrigPin, GPIO_PIN_RESET);
}

float HCSR04_Read(HCSR04_HandleTypeDef *hsensor) {
    if (!hsensor) return -1.0f;
    
    uint32_t start_tick;
    uint32_t echo_start = 0;
    uint32_t echo_end = 0;
    uint32_t pWidth = 0;

    // 1. Send Trigger Pulse (10us) with strict timing
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    HAL_GPIO_WritePin(hsensor->TrigPort, hsensor->TrigPin, GPIO_PIN_SET);
    Delay_us(10);
    HAL_GPIO_WritePin(hsensor->TrigPort, hsensor->TrigPin, GPIO_PIN_RESET);
    __set_PRIMASK(primask);

    // 2. Wait for Echo High
    // Capture start time for timeout
    start_tick = micros();
    
    while (HAL_GPIO_ReadPin(hsensor->EchoPort, hsensor->EchoPin) == GPIO_PIN_RESET) {
        if ((micros() - start_tick) > hsensor->TimeoutUs) {
            HCSR04_HandleError(hsensor, true);
            return -1.0f; // Timeout waiting for Echo start
        }
    }
    
    echo_start = micros();

    // 3. Wait for Echo Low
    while (HAL_GPIO_ReadPin(hsensor->EchoPort, hsensor->EchoPin) == GPIO_PIN_SET) {
        if ((micros() - echo_start) > hsensor->TimeoutUs) {
             HCSR04_HandleError(hsensor, true);
            return -1.0f; // Timeout waiting for Echo end
        }
    }
    
    echo_end = micros();

    // 4. Calculate Distance
    if (echo_end >= echo_start) {
        pWidth = echo_end - echo_start;
    } else {
        // Handle micros() wrap-around (very rare but possible, 32-bit wrap is ~70mins)
        pWidth = (0xFFFFFFFF - echo_start) + echo_end + 1;
    }
    
    // Distance = (Time * SpeedOfSound) / 2
    // SpeedOfSound = 340m/s = 0.034 cm/us
    // Distance(cm) = pWidth(us) * 0.017
    
    float distance = (float)pWidth * 0.017f;
    
    // Simple filter: invalid ranges
    if (distance > 400.0f || distance < 2.0f) {
        HCSR04_HandleError(hsensor, false);
        return -1.0f;
    }

    hsensor->success_cnt++;
    return distance;
}
