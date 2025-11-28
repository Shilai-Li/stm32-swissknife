#include "dht11_driver.h"

#ifdef USE_STDPERIPH_DRIVER

#include "uart_driver.h"
#include "delay_driver.h"

/* Internal static variables to store GPIO and Pin */
static GPIO_TypeDef *dht11_port;
static uint16_t dht11_pin;

/* Low-level GPIO control functions (SPL) */

static void DHT11_Pin_Output(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = dht11_pin;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(dht11_port, &GPIO_InitStructure);
}

static void DHT11_Pin_Input(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = dht11_pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(dht11_port, &GPIO_InitStructure);
}

static void DHT11_Pin_Write(uint8_t value) {
    if (value)
        GPIO_SetBits(dht11_port, dht11_pin);
    else
        GPIO_ResetBits(dht11_port, dht11_pin);
}

static uint8_t DHT11_Pin_Read(void) {
    return GPIO_ReadInputDataBit(dht11_port, dht11_pin);
}

/**
 * Simple delay in microseconds (blocking)
 * (You should calibrate this on your CPU or use a hardware timer for accuracy)
 */
static void DHT11_Delay_us(uint32_t us) {
    volatile uint32_t count = us * 10; // Rough estimation for 72MHz MCU (adjust if needed)
    while (count--) { __NOP(); }
}

void DHT11_Init(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin) {
    dht11_port = GPIOx;
    dht11_pin = GPIO_Pin;
    DHT11_Pin_Output();
    DHT11_Pin_Write(1); // Set pin high (idle)
}

static uint8_t DHT11_Read_Byte(void) {
    uint8_t i, byte = 0;
    for (i = 0; i < 8; i++) {
        while (DHT11_Pin_Read() == 0);          // Wait for line to go high
        DHT11_Delay_us(30);                     // Delay ~30us
        if (DHT11_Pin_Read() == 1)              // If line still high after 30us, read as 1
            byte |= (1 << (7 - i));
        while (DHT11_Pin_Read() == 1);          // Wait for next bit
    }
    return byte;
}

DHT11_Status DHT11_Read(DHT11_Data *data) {
    uint8_t buf[5] = {0};
    uint8_t i;

    DHT11_Pin_Output();
    DHT11_Pin_Write(0);     // Start signal (pull low at least 18ms)
    for (volatile int d = 0; d < 18000; ++d) { DHT11_Delay_us(1); }
    DHT11_Pin_Write(1);
    DHT11_Delay_us(30);

    DHT11_Pin_Input();      // Switch to input for response

    // Wait for DHT11 response (sequence: low (80us), high (80us))
    uint32_t timeout = 0;
    while (DHT11_Pin_Read() == 1) { if (++timeout > 100) return DHT11_TIMEOUT;  DHT11_Delay_us(1);}
    timeout = 0;
    while (DHT11_Pin_Read() == 0) { if (++timeout > 100) return DHT11_TIMEOUT;  DHT11_Delay_us(1);}
    timeout = 0;
    while (DHT11_Pin_Read() == 1) { if (++timeout > 100) return DHT11_TIMEOUT;  DHT11_Delay_us(1);}

    /* Read 5 bytes: humidity_int, humidity_dec, temp_int, temp_dec, checksum */
    for (i = 0; i < 5; i++) {
        buf[i] = DHT11_Read_Byte();
    }
    DHT11_Pin_Output();
    DHT11_Pin_Write(1); // Release line

    /* Checksum validation */
    uint8_t sum = buf[0] + buf[1] + buf[2] + buf[3];
    if (sum != buf[4]) return DHT11_ERROR;

    data->humidity_int = buf[0];
    data->humidity_dec = buf[1];
    data->temp_int = buf[2];
    data->temp_dec = buf[3];

    return DHT11_OK;
}

void DHT11_Test(void)
{
    DHT11_Data dht_data;
    DHT11_Status status;

    UART_InitAll();
    Delay_Init();
    DHT11_Init(GPIOA, GPIO_Pin_1);

    while (1)
    {
        // Attempt to read from DHT11 sensor
        status = DHT11_Read(&dht_data);
        if (status == DHT11_OK) {
            UART_Debug_Printf("DHT11 Read OK\r\n");
            UART_Debug_Printf("Humidity: %d.%d %%\r\n", dht_data.humidity_int, dht_data.humidity_dec);
            UART_Debug_Printf("Temperature: %d.%d C\r\n", dht_data.temp_int, dht_data.temp_dec);
        } else if (status == DHT11_TIMEOUT) {
            UART_Debug_Printf("DHT11 Read Timeout\r\n");
        } else if (status == DHT11_ERROR) {
            UART_Debug_Printf("DHT11 Data Error (checksum)\r\n");
        } else {
            UART_Debug_Printf("DHT11 Unknown Error\r\n");
        }

        Delay_ms(1000);
    }
}

#endif