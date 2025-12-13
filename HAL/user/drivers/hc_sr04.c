/**
 * @file hc_sr04.c
 * @brief HC-SR04 Ultrasonic Sensor Driver Implementation
 * @author Standard Implementation
 * @date 2024
 */

#include "drivers/hc_sr04.h"

// Helper for blocking microsecond delay using the same timer
static void HCSR04_DelayUs(TIM_HandleTypeDef *htim, uint16_t us) {
    __HAL_TIM_SET_COUNTER(htim, 0);
    while (__HAL_TIM_GET_COUNTER(htim) < us);
}

void HCSR04_Init(HCSR04_HandleTypeDef *hsensor, TIM_HandleTypeDef *htim, 
                 GPIO_TypeDef *trig_port, uint16_t trig_pin,
                 GPIO_TypeDef *echo_port, uint16_t echo_pin) 
{
    hsensor->htim = htim;
    hsensor->TrigPort = trig_port;
    hsensor->TrigPin = trig_pin;
    hsensor->EchoPort = echo_port;
    hsensor->EchoPin = echo_pin;
    hsensor->TimeoutUs = 30000; // 30ms is enough for max range (~5m)
    
    // Ensure Trigger is Low
    HAL_GPIO_WritePin(hsensor->TrigPort, hsensor->TrigPin, GPIO_PIN_RESET);
    
    // Start Timer (if not already started)
    HAL_TIM_Base_Start(hsensor->htim);
}

float HCSR04_Read(HCSR04_HandleTypeDef *hsensor) {
    uint32_t start_tick;
    uint32_t val1 = 0;
    uint32_t val2 = 0;
    uint32_t pWidth = 0;

    // 1. Send Trigger Pulse (10us)
    HAL_GPIO_WritePin(hsensor->TrigPort, hsensor->TrigPin, GPIO_PIN_SET);
    HCSR04_DelayUs(hsensor->htim, 10);
    HAL_GPIO_WritePin(hsensor->TrigPort, hsensor->TrigPin, GPIO_PIN_RESET);

    // 2. Wait for Echo High
    // We use the timer counter to implement a robust timeout loop instead of HAL_GetTick (1ms res is too coarse)
    // Assuming Timer runs at 1MHz (1us tick)
    
    __HAL_TIM_SET_COUNTER(hsensor->htim, 0);
    while (HAL_GPIO_ReadPin(hsensor->EchoPort, hsensor->EchoPin) == GPIO_PIN_RESET) {
        if (__HAL_TIM_GET_COUNTER(hsensor->htim) > hsensor->TimeoutUs) {
            return -1.0f; // Timeout waiting for Echo start
        }
    }
    
    val1 = __HAL_TIM_GET_COUNTER(hsensor->htim); // Capture Start Time (should be near 0 or small)

    // 3. Wait for Echo Low
    while (HAL_GPIO_ReadPin(hsensor->EchoPort, hsensor->EchoPin) == GPIO_PIN_SET) {
        if (__HAL_TIM_GET_COUNTER(hsensor->htim) > hsensor->TimeoutUs) {
            return -1.0f; // Timeout waiting for Echo end
        }
    }
    
    val2 = __HAL_TIM_GET_COUNTER(hsensor->htim); // Capture End Time

    // 4. Calculate Distance
    pWidth = val2 - val1;
    
    // Distance = (Time * SpeedOfSound) / 2
    // SpeedOfSound = 340m/s = 0.034 cm/us
    // Distance(cm) = pWidth(us) * 0.034 / 2 = pWidth * 0.017
    
    float distance = (float)pWidth * 0.017f;
    
    // Simple filter: invalid ranges
    if (distance > 400.0f || distance < 2.0f) return -1.0f;

    return distance;
}
