#include "uart.h"

#include "main.h"
#include "usart.h"

#define CH_DEBUG 0

void user_main(void)
{
    // 注入：将 CH_DEBUG 绑定到 huart2 硬件上
    UART_Register(CH_DEBUG, &huart2);
    // 之后就可以正常使用了
    UART_SendString(CH_DEBUG, "UART Initialized via Injection!\r\n");

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
