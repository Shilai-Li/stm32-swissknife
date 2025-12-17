/**
 * @file dht11.h
 * @brief DHT11 Temperature & Humidity Sensor Driver
 * 
 * =================================================================================
 *                       >>> INTEGRATION GUIDE <<<
 * =================================================================================
 * 1. CubeMX Config (System Core -> GPIO):
 *    - Select a Pin (e.g., PA1).
 *    - Mode: Output Open Drain (Preferred) or Push-Pull.
 *    - Speed: Low/Medium.
 *    - Label: (Optional) DHT11_PIN
 * 
 * 2. Note:
 *    - This driver manages pin direction (Input/Output) switching on the fly
 *      if using Push-Pull.
 *    - Requires 'delay.h' for microsecond delays.
 *    - Connect 4.7k - 10k Pull-up resistor between VCC and Data if using Open Drain.
 * =================================================================================
 */

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

#ifdef __cplusplus
}
#endif

#endif