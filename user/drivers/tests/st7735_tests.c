/**
 * @file st7735_tests.c
 * @brief ST7735 TFT LCD Driver Test Code
 */

#include "st7735.h"
#include "uart.h"

// --- Configuration ---
extern SPI_HandleTypeDef hspi1;

#ifndef ST7735_CS_PORT
#define ST7735_CS_PORT    GPIOB
#define ST7735_CS_PIN     GPIO_PIN_12
#endif

#ifndef ST7735_DC_PORT
#define ST7735_DC_PORT    GPIOB
#define ST7735_DC_PIN     GPIO_PIN_13
#endif

#ifndef ST7735_RST_PORT
#define ST7735_RST_PORT   GPIOB
#define ST7735_RST_PIN    GPIO_PIN_14
#endif

#ifndef ST7735_BLK_PORT
#define ST7735_BLK_PORT   GPIOB 
#define ST7735_BLK_PIN    GPIO_PIN_15
#endif

ST7735_HandleTypeDef hst7735;

void user_main(void)
{
    UART_Init();
    UART_Debug_Printf("\r\n--- ST7735 LCD Test Start ---\r\n");

    // 1. Initialize
    UART_Debug_Printf("Initializing ST7735...\r\n");
    ST7735_Init(&hst7735, &hspi1, 
                ST7735_CS_PORT, ST7735_CS_PIN,
                ST7735_DC_PORT, ST7735_DC_PIN,
                ST7735_RST_PORT, ST7735_RST_PIN,
                ST7735_BLK_PORT, ST7735_BLK_PIN);
    
    // Optional: Configure for specific panel if not default 1.8"
    // For 0.96" IPS (80x160), use:
    // ST7735_SetType(&hst7735, 26, 1, 80, 160);
    
    UART_Debug_Printf("Display Initialized (Default 128x160).\r\n");
    
    // 2. Color Fill
    UART_Debug_Printf("Test: Blue Shield\r\n");
    ST7735_FillScreen(&hst7735, ST7735_BLUE);
    HAL_Delay(500);

    UART_Debug_Printf("Test: Red Shield\r\n");
    ST7735_FillScreen(&hst7735, ST7735_RED);
    HAL_Delay(500);
    
    UART_Debug_Printf("Test: Green Shield\r\n");
    ST7735_FillScreen(&hst7735, ST7735_GREEN);
    HAL_Delay(500);
    
    ST7735_FillScreen(&hst7735, ST7735_BLACK);

    // 3. Rectangles
    UART_Debug_Printf("Test: Rectangles\r\n");
    ST7735_FillRect(&hst7735, 10, 10, 50, 50, ST7735_YELLOW);
    ST7735_FillRect(&hst7735, 70, 10, 30, 30, ST7735_MAGENTA);

    // 4. Pixel Gradient
    UART_Debug_Printf("Test: Gradient\r\n");
    for(int y=70; y<120; y++) {
        for(int x=10; x<110; x++) {
            ST7735_DrawPixel(&hst7735, x, y, (x*2) | (y<<6));
        }
    }

    UART_Debug_Printf("Tests Complete.\r\n");

    while (1) {
        ST7735_InvertColors(&hst7735, 1);
        HAL_Delay(1000);
        ST7735_InvertColors(&hst7735, 0);
        HAL_Delay(1000);
    }
}
