#include "delay.h"
#include "uart.h"
#include <stdio.h>

void user_main(void)
{
    UART_Debug_Printf("Initializing Delay Driver...\r\n");
    Delay_Init();
    UART_Debug_Printf("Delay Driver Initialized.\r\n");

    UART_Debug_Printf("Testing 1s delay...\r\n");
    uint32_t start_ms = millis();
    Delay_ms(1000);
    uint32_t end_ms = millis();

    char buffer[64];
    sprintf(buffer, "Waited: %lu ms (Target: 1000 ms)\r\n", end_ms - start_ms);
    UART_Debug_Printf(buffer);

    UART_Debug_Printf("Delay Test Complete.\r\n");
}
