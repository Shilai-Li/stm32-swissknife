#include "dht11.h"

#include "uart.h"
#include "delay.h"

void user_main(void)
{
    DHT11_Data dht_data;
    DHT11_Status status;

    UART_Init();
    Delay_Init();
    DHT11_Init(GPIOA, GPIO_PIN_1);

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
