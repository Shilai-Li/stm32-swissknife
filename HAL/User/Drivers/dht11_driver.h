#ifndef __DHT11_DRIVER_H__
#define __DHT11_DRIVER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

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
    uint8_t humidity_dec;    /* Decimal part of humidity (always 0 for DHT11) */
    uint8_t temp_int;        /* Integer part of temperature */
    uint8_t temp_dec;        /* Decimal part of temperature (always 0 for DHT11) */
} DHT11_Data;

/* ============================================================
 * Public API Functions
 * ============================================================ */

/**
 * @brief  Initialize the DHT11 sensor (configure GPIO).
 * @param  GPIOx: GPIO port where DHT11 is connected.
 * @param  GPIO_Pin: GPIO pin number (HAL format).
 * @retval None
 */
void DHT11_Init(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
DHT11_Status DHT11_Read(DHT11_Data *data);
void DHT11_Test(void);

#ifdef __cplusplus
}
#endif

#endif