#include "dht11.h"

#include "tim.h"
#include "uart.h"
#include "delay.h"

/* Internal variables to store GPIO and Pin information */
static GPIO_TypeDef *dht11_port;
static uint16_t dht11_pin;

/* Low-level GPIO control functions (HAL) */
static void DHT11_Pin_Output(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = dht11_pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(dht11_port, &GPIO_InitStruct);
}

static void DHT11_Pin_Input(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = dht11_pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(dht11_port, &GPIO_InitStruct);
}

static void DHT11_Pin_Write(uint8_t level) {
    HAL_GPIO_WritePin(dht11_port, dht11_pin, level ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static uint8_t DHT11_Pin_Read(void) {
    return HAL_GPIO_ReadPin(dht11_port, dht11_pin);
}

/* Microsecond delay using HAL (you may need to optimize for your MCU speed) */
static void DHT11_Delay_us(uint16_t us) {
    uint32_t start = __HAL_TIM_GET_COUNTER(&htim2); // You must setup a timer beforehand!
    while((__HAL_TIM_GET_COUNTER(&htim2) - start) < us);
}

/* Initialize the DHT11 hardware */
void DHT11_Init(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin) {
    dht11_port = GPIOx;
    dht11_pin = GPIO_Pin;
    DHT11_Pin_Output();
    DHT11_Pin_Write(1); // Idle high
}

/* Read a byte from DHT11 */
static uint8_t DHT11_Read_Byte(void) {
    uint8_t i, byte = 0;
    for (i = 0; i < 8; i++) {
        /* Wait for low level */
        while (DHT11_Pin_Read() == 0);
        DHT11_Delay_us(30); // After low, check high
        if (DHT11_Pin_Read() == 1)
            byte |= (1 << (7 - i));
        /* Wait for end of the bit */
        while (DHT11_Pin_Read() == 1);
    }
    return byte;
}

/* Read full data from DHT11 */
DHT11_Status DHT11_Read(DHT11_Data *data) {
    uint8_t buf[5] = {0};
    uint8_t i;

    DHT11_Pin_Output();
    DHT11_Pin_Write(0); // Start signal: low for at least 18ms
    Delay_ms(18);
    DHT11_Pin_Write(1); // Pull up for 20-40us
    DHT11_Delay_us(30);

    DHT11_Pin_Input(); // Prepare to read

    /* Wait for DHT11 response */
    uint32_t timeout = 0;
    while (DHT11_Pin_Read() == 1) {
        if (++timeout > 100) return DHT11_TIMEOUT;
        DHT11_Delay_us(1);
    }
    timeout = 0;
    while (DHT11_Pin_Read() == 0) {
        if (++timeout > 100) return DHT11_TIMEOUT;
        DHT11_Delay_us(1);
    }
    timeout = 0;
    while (DHT11_Pin_Read() == 1) {
        if (++timeout > 100) return DHT11_TIMEOUT;
        DHT11_Delay_us(1);
    }

    /* Read 5 bytes: humidity_int, humidity_dec, temp_int, temp_dec, checksum */
    for (i = 0; i < 5; i++) {
        buf[i] = DHT11_Read_Byte();
    }
    DHT11_Pin_Output();
    DHT11_Pin_Write(1); // Release pin

    /* Check data */
    uint8_t sum = buf[0] + buf[1] + buf[2] + buf[3];
    if (sum != buf[4]) return DHT11_ERROR;

    data->humidity_int = buf[0];
    data->humidity_dec = buf[1];
    data->temp_int = buf[2];
    data->temp_dec = buf[3];
    return DHT11_OK;
}
