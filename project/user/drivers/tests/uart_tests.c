#include "uart.h"

void User_Entry(void)
{
    UART_Init();

    UART_Debug_Printf("System initialized successfully.\r\n");

    uint8_t rx_data;

    while (1)
    {
        UART_Debug_Printf("Heartbeat: %lu ms\r\n", HAL_GetTick());
        HAL_Delay(1000);

        if (UART_Available(UART_DEBUG_CHANNEL))
        {
            if (UART_Read(UART_DEBUG_CHANNEL, &rx_data))
            {
                // Echo received byte back to UART_DEBUG_CHANNEL
                UART_Send(UART_DEBUG_CHANNEL, &rx_data, 1);
                UART_Debug_Printf(" [Echoed: %c]\r\n", rx_data);
            }
        }
    }
}
