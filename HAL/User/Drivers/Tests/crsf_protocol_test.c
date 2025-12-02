#if 0

#include <stdio.h>

#include "uart_driver.h"
#include "crsf.h"
#include "stm32f1xx_hal.h"

void User_Entry(void)
{
    // Initialize UART driver (initializes all enabled UARTs)
    UART_Init();
    
    // Initialize CRSF parser
    crsf_init();

    UART_Debug_Printf("CRSF App Started\r\n");

    uint8_t rx_byte;
    uint32_t last_print_time = 0;

    while (1)
    {
        // 1. Process incoming CRSF data from UART1
        while (UART_Read(UART_CHANNEL_1, &rx_byte))
        {
            // Using HAL_GetTick() * 1000 for microsecond approximation
            // Note: This is coarse but might suffice for basic timeout logic
            crsf_process_byte(rx_byte, HAL_GetTick() * 1000);
        }

        // 2. Periodically print channel data to UART2 (PC)
        if (HAL_GetTick() - last_print_time > 20) // 50Hz update
        {
            last_print_time = HAL_GetTick();

            if (crsf_is_connected())
            {
                // Format and send channel data
                // CRSF has 16 channels. We'll print the first 4 for brevity, or all if needed.
                // Let's print the first 8 channels to keep it readable.
                char buf[128];
                int len = snprintf(buf, sizeof(buf), 
                    "CH: %4d %4d %4d %4d %4d %4d %4d %4d\r\n",
                    crsf_get_channel(0), crsf_get_channel(1), crsf_get_channel(2), crsf_get_channel(3),
                    crsf_get_channel(4), crsf_get_channel(5), crsf_get_channel(6), crsf_get_channel(7)
                );
                
                if (len > 0)
                {
                    UART_Send(UART_CHANNEL_2, (uint8_t*)buf, len);
                }
            }
        }
    }
}

#endif
