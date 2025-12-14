/**
 * @file ds18b20.h
 * @brief DS18B20 OneWire Temperature Sensor Driver
 * 
 * =================================================================================
 *                       >>> INTEGRATION GUIDE <<<
 * =================================================================================
 * 1. CubeMX Config (GPIO):
 *    - Select a Pin (e.g. PB1).
 *    - Mode: Output Open Drain (Strongly Recommended).
 *    - Pull-up: None (External 4.7k Pull-up Resistor required).
 *    - Speed: Medium/High.
 * 
 * 2. Dependencies:
 *    - Requires 'delay.h' for Delay_us().
 * 
 * 3. Wiring:
 *    - DS18B20 DQ pin -> GPIO (with 4.7k to 3.3V).
 * =================================================================================
 */

#ifndef DS18B20_H
#define DS18B20_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"
#include <stdbool.h>

/**
 * @brief DS18B20 Handle
 */
typedef struct {
    GPIO_TypeDef *port;
    uint16_t pin;
    float last_temp;
    bool error;
} DS18B20_Handle_t;

/**
 * @brief Initialize the DS18B20 handle (Setups GPIO if needed, but usually GPIO Init is done by CubeMX or manually)
 * @note  This driver assumes you have initialized the 'delay' driver.
 */
void DS18B20_Init(DS18B20_Handle_t *handle, GPIO_TypeDef *port, uint16_t pin);

/**
 * @brief Start Temperature Conversion (Non-blocking usually, but this function just sends command)
 * @details After calling this, wait 750ms for 12-bit conversion.
 */
void DS18B20_StartConversion(DS18B20_Handle_t *handle);

/**
 * @brief Read Temperature
 * @return Temperature in Celsius. Returns -999.0f on error.
 */
float DS18B20_ReadTemp(DS18B20_Handle_t *handle);

/**
 * @brief Sync Wrapper: Start -> Delay(750ms) -> Read
 */
float DS18B20_ReadTempBlocked(DS18B20_Handle_t *handle);

#ifdef __cplusplus
}
#endif

#endif // DS18B20_H
