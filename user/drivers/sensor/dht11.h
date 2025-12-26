/**
 * @file dht11.h
 * @brief DHT11 Temperature & Humidity Sensor Driver (Enhanced)
 */
#ifndef DHT11_H
#define DHT11_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdbool.h>

/* ============================================================
 * Type Definitions
 * ============================================================ */

/* DHT11 status return type */
typedef enum {
    DHT11_OK = 0,
    DHT11_ERROR_CHECKSUM,
    DHT11_ERROR_TIMEOUT,
    DHT11_ERROR_GPIO
} DHT11_Status;

/* Forward struct declaration */
struct DHT11_Handle_s;

/* DHT11 Data and Handle structure */
typedef struct DHT11_Handle_s {
    /* Configuration */
    GPIO_TypeDef *port;
    uint16_t pin;
    
    /* Data */
    uint8_t humidity_int;
    uint8_t humidity_dec;
    uint8_t temp_int;
    uint8_t temp_dec;

    /* Statistics (Robustness) */
    volatile uint32_t error_cnt;
    volatile uint32_t timeout_cnt;
    volatile uint32_t checksum_error_cnt;
    volatile uint32_t successful_read_cnt;
    
    /* Callback */
    void (*error_cb)(struct DHT11_Handle_s *dev); 
} DHT11_Handle_t;

/* Public API Functions */

/**
 * @brief  Initialize the DHT11 sensor handle.
 * @param  dev: Pointer to the DHT11 handle.
 * @param  port: GPIO port.
 * @param  pin: GPIO pin.
 * @retval None
 */
void DHT11_Init(DHT11_Handle_t *dev, GPIO_TypeDef *port, uint16_t pin);

void DHT11_SetErrorCallback(DHT11_Handle_t *dev, void (*cb)(DHT11_Handle_t *));
DHT11_Status DHT11_Read(DHT11_Handle_t *dev);
uint32_t DHT11_GetErrorCount(DHT11_Handle_t *dev);

#ifdef __cplusplus
}
#endif

#endif /* DHT11_H */
