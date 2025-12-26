#include "usb_cdc.h"
#include "main.h"

void user_main(void)
{
    // 1. Initialize USB Buffer
    USB_CDC_Init();
    
    // Note: USB setup itself happens in main.c (MX_USB_DEVICE_Init) which is called before user_main.
    // However, USB enumeration takes time (host needs to install drivers).
    // We wait a bit or just start pushing. If not connected, packets drop.
    
    HAL_Delay(2000); // Wait for Host to enumerate

    // 2. Test SendString
    USB_CDC_SendString("\r\n===================================\r\n");
    USB_CDC_SendString("      USB CDC Driver Test Suite    \r\n");
    USB_CDC_SendString("===================================\r\n");
    USB_CDC_Printf("Cmds: [s]SendBursts [f]Flush [e]Echo \r\n");

    uint32_t last_tick = 0;

    while (1)
    {
        // No Poll() needed for USB CDC (it's interrupt driven from USB stack)

        // 4. Heartbeat
        if (HAL_GetTick() - last_tick > 1000) {
            last_tick = HAL_GetTick();
            USB_CDC_Printf("[Tick] System Alive: %lu ms\r\n", last_tick);
        }

        // 5. Check Reception
        if (USB_CDC_Available())
        {
            uint8_t cmd;
            if (USB_CDC_Read(&cmd)) 
            {
                switch (cmd) 
                {
                    case 's': // Test Burst Send
                        USB_CDC_SendString("[Test] Sending Burst 1...\r\n");
                        USB_CDC_SendString("[Test] Sending Burst 2...\r\n");
                        USB_CDC_SendString("[Test] Sending Burst 3...\r\n");
                        break;
                    
                    case 'f': // Test Flush
                        USB_CDC_Printf("[Test] Flushing Rx Buffer... \r\n");
                        USB_CDC_Flush();
                        break;

                    case '\r':
                    case '\n':
                        // Ignore newlines
                        break;

                    default: // Echo back
                        USB_CDC_Printf("Echo: %c (0x%02X)\r\n", cmd, cmd);
                        break;
                }
            }
        }
    }
}
