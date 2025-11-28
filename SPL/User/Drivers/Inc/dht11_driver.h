#ifndef __DHT11_DRIVER_H__
#define __DHT11_DRIVER_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef USE_STDPERIPH_DRIVER

#include <stdint.h>
#include <stdbool.h>

#define STM32F1
//#define STM32F4

#ifdef STM32F1
#include "stm32f10x.h"
#endif

/* ============================================================
 * Type Definitions
 * ============================================================ */

/* DHT11 status return type */
typedef enum {
    DHT11_OK = 0,
    DHT11_ERROR,
    DHT11_TIMEOUT
} DHT11_Status;

/* DHT11 data structure */
typedef struct {
    uint8_t humidity_int;    /* Integer part of humidity */
    uint8_t humidity_dec;    /* Decimal part of humidity */
    uint8_t temp_int;        /* Integer part of temperature */
    uint8_t temp_dec;        /* Decimal part of temperature */
} DHT11_Data;

/* ============================================================
 * Public API Functions
 * ============================================================ */

/**
 * @brief  Initialize the DHT11 sensor (configure GPIO).
 * @param  GPIOx: GPIO port where DHT11 is connected.
 * @param  GPIO_Pin: GPIO pin number (SPL format).
 */
void DHT11_Init(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);

/**
 * @brief  Read data once from the DHT11 sensor.
 * @param  data: Pointer to DHT11_Data structure to save the results.
 * @retval DHT11_Status: Operation status.
 */
DHT11_Status DHT11_Read(DHT11_Data *data);

void DHT11_Test(void);

#endif

#ifdef __cplusplus
}
#endif

#endif /* __DHT11_DRIVER_H__ */
