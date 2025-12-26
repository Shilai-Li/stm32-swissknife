/**
 * @file ili9488_tests.c
 * @brief ILI9488 TFT LCD Driver Test Code
 */

#include "ili9488.h"
#include "uart.h"

// --- Configuration ---
extern SPI_HandleTypeDef hspi1;

#ifndef ILI9488_CS_PORT
#define ILI9488_CS_PORT    GPIOB
#define ILI9488_CS_PIN     GPIO_PIN_12
#endif

#ifndef ILI9488_DC_PORT
#define ILI9488_DC_PORT    GPIOB
#define ILI9488_DC_PIN     GPIO_PIN_13
#endif

#ifndef ILI9488_RST_PORT
#define ILI9488_RST_PORT   GPIOB
#define ILI9488_RST_PIN    GPIO_PIN_14
#endif

#ifndef ILI9488_BLK_PORT
#define ILI9488_BLK_PORT   GPIOB 
#define ILI9488_BLK_PIN    GPIO_PIN_15
#endif

ILI9488_HandleTypeDef hili_huge;

void app_main(void)
{
    UART_Init();
    UART_Debug_Printf("\r\n--- ILI9488 LCD Test Start ---\r\n");

    // 1. Initialize
    UART_Debug_Printf("Initializing ILI9488 (320x480)...\r\n");
    ILI9488_Init(&hili_huge, &hspi1, 
                ILI9488_CS_PORT, ILI9488_CS_PIN,
                ILI9488_DC_PORT, ILI9488_DC_PIN,
                ILI9488_RST_PORT, ILI9488_RST_PIN,
                ILI9488_BLK_PORT, ILI9488_BLK_PIN);
                
    UART_Debug_Printf("Display Initialized.\r\n");
    
    // 2. Full screen fills
    UART_Debug_Printf("Test: Red\r\n");
    ILI9488_FillScreen(&hili_huge, ILI9488_RED);
    HAL_Delay(500);

    UART_Debug_Printf("Test: Green\r\n");
    ILI9488_FillScreen(&hili_huge, ILI9488_GREEN);
    HAL_Delay(500);
    
    UART_Debug_Printf("Test: Blue\r\n");
    ILI9488_FillScreen(&hili_huge, ILI9488_BLUE);
    HAL_Delay(500);

    // 3. Rectangles
    UART_Debug_Printf("Test: Rectangles and Rotation\r\n");
    
    ILI9488_FillScreen(&hili_huge, ILI9488_BLACK);
    
    ILI9488_FillRect(&hili_huge, 10, 10, 100, 100, ILI9488_ORANGE);
    ILI9488_FillRect(&hili_huge, 120, 10, 100, 100, ILI9488_CYAN);
    ILI9488_FillRect(&hili_huge, 10, 120, 100, 100, ILI9488_PURPLE);
    
    // 4. Gradient (Testing pixel drawing performance)
    UART_Debug_Printf("Test: Pixel Gradient\r\n");
    for(int y=300; y<400; y++) {
        for(int x=10; x<110; x++) {
             uint16_t color = (x << 2) | (y << 5); 
             ILI9488_DrawPixel(&hili_huge, x, y, color);
        }
    }
    
    // 5. Rotation
    ILI9488_SetRotation(&hili_huge, 1); // Landscape 480x320
    UART_Debug_Printf("Rotated to Landscape.\r\n");
    ILI9488_FillRect(&hili_huge, 0, 0, 50, 50, ILI9488_WHITE); // Should be top left

    UART_Debug_Printf("Tests Complete.\r\n");

    while (1) {
        ILI9488_InvertColors(&hili_huge, 1);
        HAL_Delay(500);
        ILI9488_InvertColors(&hili_huge, 0);
        HAL_Delay(500);
    }
}
