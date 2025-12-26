#include "dht11.h"
#include "delay.h"

static void DHT11_HandleError(DHT11_Handle_t *dev) {
    if (dev) {
        dev->error_cnt++;
        if (dev->error_cb) {
            dev->error_cb(dev);
        }
    }
}

void DHT11_SetErrorCallback(DHT11_Handle_t *dev, void (*cb)(DHT11_Handle_t *)) {
    if (dev) dev->error_cb = cb;
}

static void DHT11_Pin_Output(DHT11_Handle_t *dev) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = dev->pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(dev->port, &GPIO_InitStruct);
}

static void DHT11_Pin_Input(DHT11_Handle_t *dev) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = dev->pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(dev->port, &GPIO_InitStruct);
}

static void DHT11_Pin_Write(DHT11_Handle_t *dev, uint8_t level) {
    HAL_GPIO_WritePin(dev->port, dev->pin, level ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static uint8_t DHT11_Pin_Read(DHT11_Handle_t *dev) {
    return (uint8_t)HAL_GPIO_ReadPin(dev->port, dev->pin);
}

static uint8_t DHT11_Read_Byte(DHT11_Handle_t *dev) {
    uint8_t i, byte = 0;
    for (i = 0; i < 8; i++) {
        /* Wait for pin to go high (start of bit) */
        uint32_t timeout = 0;
        while (DHT11_Pin_Read(dev) == 0) {
            if (++timeout > 1000) return 0; // simple guard
        }
        
        // Wait 30us to determine 0 or 1
        Delay_us(35); 
        
        if (DHT11_Pin_Read(dev) == 1) {
            byte |= (1 << (7 - i));
        }
        
        /* Wait for pin to go low (end of bit) */
        while (DHT11_Pin_Read(dev) == 1) {
             if (++timeout > 1000) break; 
        }
    }
    return byte;
}

/* ============================================================================
 * Public API Functions
 * ========================================================================= */

void DHT11_Init(DHT11_Handle_t *dev, GPIO_TypeDef *port, uint16_t pin) {
    if (!dev) return;

    dev->port = port;
    dev->pin = pin;
    
    dev->humidity_int = 0;
    dev->humidity_dec = 0;
    dev->temp_int = 0;
    dev->temp_dec = 0;
    
    dev->error_cnt = 0;
    dev->timeout_cnt = 0;
    dev->checksum_error_cnt = 0;
    dev->successful_read_cnt = 0;
    dev->error_cb = NULL;

    DHT11_Pin_Output(dev);
    DHT11_Pin_Write(dev, 1); // Idle high
}

DHT11_Status DHT11_Read(DHT11_Handle_t *dev) {
    if (!dev) return DHT11_ERROR_GPIO;

    uint8_t buf[5] = {0};
    uint8_t i;

    // Critical Section
    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    DHT11_Pin_Output(dev);
    DHT11_Pin_Write(dev, 0); // Start signal: low for at least 18ms
    
    // Enable IRQ for the long delay (18ms)
    __set_PRIMASK(primask); 
    HAL_Delay(18); // Use HAL_Delay for ms
    __disable_irq();
    
    DHT11_Pin_Write(dev, 1); // Pull up
    Delay_us(30);
    
    DHT11_Pin_Input(dev); // Prepare to read

    /* Check Response: Low for ~80us then High for ~80us */
    uint32_t timeout = 0;
    
    // Wait for Low (Response start)
    while (DHT11_Pin_Read(dev) == 1) {
        if (++timeout > 500) {
             __set_PRIMASK(primask);
             dev->timeout_cnt++;
             DHT11_HandleError(dev);
             return DHT11_ERROR_TIMEOUT;
        }
        Delay_us(1);
    }
    
    timeout = 0;
    // Wait for High (Response Low -> High)
    while (DHT11_Pin_Read(dev) == 0) {
        if (++timeout > 500) {
             __set_PRIMASK(primask);
             dev->timeout_cnt++;
             DHT11_HandleError(dev);
             return DHT11_ERROR_TIMEOUT;
        }
        Delay_us(1);
    }
    
    timeout = 0;
    // Wait for Low (Response High -> Start of transmission)
    while (DHT11_Pin_Read(dev) == 1) {
        if (++timeout > 500) {
             __set_PRIMASK(primask);
             dev->timeout_cnt++;
             DHT11_HandleError(dev);
             return DHT11_ERROR_TIMEOUT;
        }
        Delay_us(1);
    }

    /* Read 5 bytes */
    for (i = 0; i < 5; i++) {
        buf[i] = DHT11_Read_Byte(dev);
    }

    __set_PRIMASK(primask);

    DHT11_Pin_Output(dev);
    DHT11_Pin_Write(dev, 1);

    /* Check Checksum */
    uint8_t sum = buf[0] + buf[1] + buf[2] + buf[3];
    if (sum != buf[4]) {
        dev->checksum_error_cnt++;
        DHT11_HandleError(dev);
        return DHT11_ERROR_CHECKSUM;
    }

    dev->humidity_int = buf[0];
    dev->humidity_dec = buf[1];
    dev->temp_int = buf[2];
    dev->temp_dec = buf[3];
    
    dev->successful_read_cnt++;
    return DHT11_OK;
}

