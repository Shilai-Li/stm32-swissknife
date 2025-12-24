/**
 * @file ili9341_tests.c
 * @brief ILI9341 TFT LCD Driver Test Code
 */

#include "ili9341.h"
#include "uart.h"

// --- Configuration ---
// Adapt these to your actual wiring
extern SPI_HandleTypeDef hspi1;

#ifndef ILI9341_CS_PORT
#define ILI9341_CS_PORT    GPIOB
#define ILI9341_CS_PIN     GPIO_PIN_12
#endif

#ifndef ILI9341_DC_PORT
#define ILI9341_DC_PORT    GPIOB
#define ILI9341_DC_PIN     GPIO_PIN_13
#endif

#ifndef ILI9341_RST_PORT
#define ILI9341_RST_PORT   GPIOB
#define ILI9341_RST_PIN    GPIO_PIN_14
#endif

#ifndef ILI9341_BLK_PORT
#define ILI9341_BLK_PORT   GPIOB // Can be NULL
#define ILI9341_BLK_PIN    GPIO_PIN_15
#endif

ILI9341_HandleTypeDef hili;

void user_main(void)
{
    UART_Init();
    UART_Debug_Printf("\r\n--- ILI9341 LCD Test Start ---\r\n");

    // 1. Initialize
    UART_Debug_Printf("Initializing ILI9341...\r\n");
    ILI9341_Init(&hili, &hspi1, 
                ILI9341_CS_PORT, ILI9341_CS_PIN,
                ILI9341_DC_PORT, ILI9341_DC_PIN,
                ILI9341_RST_PORT, ILI9341_RST_PIN,
                ILI9341_BLK_PORT, ILI9341_BLK_PIN);
                
    UART_Debug_Printf("Display Initialized (240x320).\r\n");
    
    // 2. Color Fill Loop
    uint16_t colors[] = {ILI9341_RED, ILI9341_GREEN, ILI9341_BLUE, ILI9341_BLACK, ILI9341_WHITE};
    const char* color_names[] = {"RED", "GREEN", "BLUE", "BLACK", "WHITE"};
    
    for(int i=0; i<5; i++) {
        UART_Debug_Printf("Fill: %s\r\n", color_names[i]);
        ILI9341_FillScreen(&hili, colors[i]);
        HAL_Delay(500);
    }

    // 3. Squares
    UART_Debug_Printf("Drawing squares...\r\n");
    ILI9341_FillRect(&hili, 20, 20, 100, 100, ILI9341_YELLOW);
    ILI9341_FillRect(&hili, 140, 20, 80, 80, ILI9341_CYAN);
    ILI9341_FillRect(&hili, 20, 140, 60, 60, ILI9341_MAGENTA);

    // 4. Gradient on remaining space
    UART_Debug_Printf("Drawing gradient...\r\n");
    for(int y=220; y<300; y++) {
        for(int x=20; x<220; x++) {
             ILI9341_DrawPixel(&hili, x, y, (x*y));
        }
    }

    // 5. Rotation Test
    UART_Debug_Printf("Rotation Test...\r\n");
    ILI9341_SetRotation(&hili, 1); // Landscape
    ILI9341_FillRect(&hili, 0, 0, 50, 50, ILI9341_ORANGE); // Top-Left in landscape
    HAL_Delay(1000);
    ILI9341_SetRotation(&hili, 0); // Portrait back
    
    UART_Debug_Printf("Tests Complete. Blinking Inversion.\r\n");

    while (1) {
        ILI9341_InvertColors(&hili, 1);
        HAL_Delay(500);
        ILI9341_InvertColors(&hili, 0);
        HAL_Delay(500);
    }
}
