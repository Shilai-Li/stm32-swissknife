/**
 * @file ds18b20.h
 * @brief DS18B20 OneWire Temperature Sensor Driver
 */

#ifndef DS18B20_H
#define DS18B20_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdbool.h>

/**
 * @brief DS18B20 Handle
 */
/* Forward declaration */
typedef struct DS18B20_Handle_s DS18B20_Handle_t;
typedef void (*DS18B20_ErrorCallback)(DS18B20_Handle_t *dev);

struct DS18B20_Handle_s {
    GPIO_TypeDef *port;
    uint16_t pin;
    /* TIM handle removed: uses DWT Delay_us() */
    
    float last_temp;
    
    /* Stats */
    volatile uint32_t error_cnt;
    volatile uint32_t success_cnt;
    volatile uint32_t crc_error_cnt;
    
    /* Callback */
    DS18B20_ErrorCallback error_cb;
};

/**
 * @brief Initialize the DS18B20 handle
 */
void DS18B20_Init(DS18B20_Handle_t *handle, GPIO_TypeDef *port, uint16_t pin);

/**
 * @brief Start Temperature Conversion
 */
void DS18B20_StartConversion(DS18B20_Handle_t *handle);

/**
 * @brief Read Temperature
 */
float DS18B20_ReadTemp(DS18B20_Handle_t *handle);

/**
 * @brief Set Error Callback
 */
void DS18B20_SetErrorCallback(DS18B20_Handle_t *handle, DS18B20_ErrorCallback cb);

/**
 * @brief Sync Wrapper: Start -> Delay(750ms) -> Read
 */
float DS18B20_ReadTempBlocked(DS18B20_Handle_t *handle);

#ifdef __cplusplus
}
#endif

#endif // DS18B20_H
