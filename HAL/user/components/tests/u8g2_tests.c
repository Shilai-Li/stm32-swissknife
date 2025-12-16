/**
 * @file u8g2_tests.c
 * @brief Test U8G2 with SSD1306 (I2C Example)
 */

#include "u8g2_port.h"
#include <stdio.h>

// Assuming u8g2 struct
static u8g2_t u8g2;

void Test_U8G2_Entry(void) {
    // 1. Init
    // If testing SPI, use U8G2_Init_SSD1306_SPI(...)
    U8G2_Init_SSD1306_I2C(&u8g2);
    
    // 2. Draw
    u8g2_ClearBuffer(&u8g2);
    
    // Font
    u8g2_SetFont(&u8g2, u8g2_font_ncenB08_tr);
    u8g2_DrawStr(&u8g2, 0, 10, "Hello U8G2!");
    
    u8g2_DrawBox(&u8g2, 10, 20, 50, 20);
    u8g2_DrawCircle(&u8g2, 100, 32, 15, U8G2_DRAW_ALL);
    
    // 3. Send
    u8g2_SendBuffer(&u8g2);
    
    while(1) {
        // Animation loop
        for(int x=0; x<128; x+=2) {
             u8g2_ClearBuffer(&u8g2);
             u8g2_DrawStr(&u8g2, 0, 10, "Hello U8G2!");
             u8g2_DrawBox(&u8g2, x, 20, 20, 20);
             u8g2_SendBuffer(&u8g2);
        }
    }
}
