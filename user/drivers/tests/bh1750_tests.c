#include "main.h"
#include "drivers/sensor/bh1750.h"
#include "drivers/interface/soft_i2c.h"
#include "drivers/communication/uart.h"
#include "drivers/system/delay.h"
#include <stdio.h>

// I2C Pin Configuration (Adjust for your board!)
#define SCL_PORT    GPIOB
#define SCL_PIN     GPIO_PIN_6
#define SDA_PORT    GPIOB
#define SDA_PIN     GPIO_PIN_7

Soft_I2C_HandleTypeDef hi2c;
BH1750_Handle_t hLight;

void user_main(void)
{
    // Initialize System Drivers
    UART_Init();
    Delay_Init();
    
    UART_Debug_Printf("\r\n=== BH1750 Light Sensor Test ===\r\n");

    // 1. Initialize Software I2C
    UART_Debug_Printf("- Init Soft I2C on PB6/PB7...\r\n");
    Soft_I2C_Init(&hi2c, SCL_PORT, SCL_PIN, SDA_PORT, SDA_PIN);
    
    // 2. Initialize BH1750 Driver
    // Address Pin State: 0 (Low) -> Address 0x23 (7-bit)
    BH1750_Init(&hLight, &hi2c, 0); 
    
    // 3. Power On and Start Continuous Config
    UART_Debug_Printf("- Starting Sensor...\r\n");
    BH1750_Start(&hLight);
    
    UART_Debug_Printf("Initialization Complete. Loop Starting.\r\n");

    while (1)
    {
        float lux = BH1750_ReadLux(&hLight);
        
        if (lux >= 0.0f) {
            // Print Lux value (Integer workaround for float if %f not supported)
            UART_Debug_Printf("Lux: %d.%02d\r\n", (int)lux, (int)((lux - (int)lux)*100));
        } else {
            UART_Debug_Printf("Error Reading Sensor! (Check ADDR, SCL, SDA wiring)\r\n");
            
            // Re-init workaround attempt?
            // BH1750_Start(&hLight);
        }
        
        Delay_ms(500);
    }
}
