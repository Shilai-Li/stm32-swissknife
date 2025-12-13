/**
 * @file st7789_tests.c
 * @brief ST7789 TFT LCD Driver Test Code
 * 
 * CubeMX Configuration Guide:
 * 1. Connectivity -> SPI1 (or other SPI)
 *    - Mode: Transmit Only Master (or Full-Duplex Master)
 *    - Data Size: 8 bits
 *    - Prescaler: As low as possible for high speed (e.g. 2 or 4) -> 36MHz or 18MHz usually fine for ST7789
 *    - CPOL: High, CPHA: 2 Edge (Mode 3) usually works best, or Mode 0. Try Mode 3.
 *    - NSS: Disable
 * 
 * 2. GPIOs
 *    - CS (Chip Select) -> Output
 *    - DC (Data/Command) -> Output
 *    - RES (Reset) -> Output
 *    - BLK (Backlight) -> Output (optional)
 * 
 * 3. Wiring
 *    - VCC -> 3.3V
 *    - GND -> GND
 *    - CS  -> CS Pin
 *    - RES -> RES Pin
 *    - DC  -> DC Pin
 *    - SDA -> MOSI
 *    - SCL -> SCK
 *    - BLK -> BLK Pin
 */

#include "drivers/st7789.h"
#include "drivers/uart.h"

// --- Configuration ---
extern SPI_HandleTypeDef hspi1;

#ifndef ST7789_CS_PORT
#define ST7789_CS_PORT    GPIOB
#define ST7789_CS_PIN     GPIO_PIN_12
#endif

#ifndef ST7789_DC_PORT
#define ST7789_DC_PORT    GPIOB
#define ST7789_DC_PIN     GPIO_PIN_13
#endif

#ifndef ST7789_RST_PORT
#define ST7789_RST_PORT   GPIOB
#define ST7789_RST_PIN    GPIO_PIN_14
#endif

#ifndef ST7789_BLK_PORT
#define ST7789_BLK_PORT   GPIOB // Can be NULL
#define ST7789_BLK_PIN    GPIO_PIN_15
#endif

ST7789_HandleTypeDef hlcd;

void User_Entry(void)
{
    UART_Init();
    UART_Debug_Printf("\r\n--- ST7789 LCD Test Start ---\r\n");

    // 1. Initialize
    UART_Debug_Printf("Initializing Display...\r\n");
    ST7789_Init(&hlcd, &hspi1, 
                ST7789_CS_PORT, ST7789_CS_PIN,
                ST7789_DC_PORT, ST7789_DC_PIN,
                ST7789_RST_PORT, ST7789_RST_PIN,
                ST7789_BLK_PORT, ST7789_BLK_PIN);
                
    UART_Debug_Printf("Display Initialized. Turning on Backlight.\r\n");
    
    // 2. Color Fill Test
    UART_Debug_Printf("Test: Red Screen\r\n");
    ST7789_FillScreen(&hlcd, ST7789_RED);
    HAL_Delay(500);
    
    UART_Debug_Printf("Test: Green Screen\r\n");
    ST7789_FillScreen(&hlcd, ST7789_GREEN);
    HAL_Delay(500);
    
    UART_Debug_Printf("Test: Blue Screen\r\n");
    ST7789_FillScreen(&hlcd, ST7789_BLUE);
    HAL_Delay(500);
    
    UART_Debug_Printf("Test: Black Screen\r\n");
    ST7789_FillScreen(&hlcd, ST7789_BLACK);
    HAL_Delay(500);

    // 3. Rectangles Test
    UART_Debug_Printf("Test: Rectangles\r\n");
    ST7789_FillRect(&hlcd, 10, 10, 50, 50, ST7789_YELLOW);
    ST7789_FillRect(&hlcd, 70, 10, 50, 50, ST7789_CYAN);
    ST7789_FillRect(&hlcd, 10, 70, 50, 50, ST7789_MAGENTA);
    ST7789_FillRect(&hlcd, 70, 70, 50, 50, ST7789_WHITE);
    
    // 4. Pixel Test (Gradient)
    UART_Debug_Printf("Test: Gradient\r\n");
    for(int y=130; y<200; y++) {
        for(int x=10; x<200; x++) {
            // Simple gradient logic
            uint16_t color = (x << 11) | (y << 5); 
            ST7789_DrawPixel(&hlcd, x, y, color);
        }
    }
    
    UART_Debug_Printf("Tests Complete. Looping colors.\r\n");

    while (1) {
        ST7789_InvertColors(&hlcd, 1);
        HAL_Delay(1000);
        ST7789_InvertColors(&hlcd, 0);
        HAL_Delay(1000);
    }
}
