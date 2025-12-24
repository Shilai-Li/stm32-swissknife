/**
 * @file xpt2046_tests.c
 * @brief XPT2046 Touch Driver Test
 *
 * CubeMX:
 * - SPI1 or SPI2 (Standard SPI Mode 0 or similar).
 * - GPIO Output: Touch_CS (e.g. PB12)
 * - GPIO Input: Touch_IRQ (e.g. PB1) -> Pull Up recommended logic, usually internal pullup or external. XPT2046 IRQ is Open Drain low.
 */

#include "xpt2046.h"
#include "uart.h"

// --- Config ---
extern SPI_HandleTypeDef hspi2; // Use SPI2 for Touch (often shared with LCD or dedicated)

#ifndef TOUCH_CS_PORT
#define TOUCH_CS_PORT GPIOB
#define TOUCH_CS_PIN  GPIO_PIN_12
#endif

#ifndef TOUCH_IRQ_PORT
#define TOUCH_IRQ_PORT GPIOB
#define TOUCH_IRQ_PIN  GPIO_PIN_1
#endif


XPT2046_HandleTypeDef htouch;

void user_main(void)
{
    UART_Init();
    UART_Debug_Printf("\r\n--- XPT2046 Touch Test ---\r\n");

    // 1. Init
    UART_Debug_Printf("Init XPT2046 (SPI2, CS=PB12, IRQ=PB1)...\r\n");
    XPT2046_Init(&htouch, &hspi2, 
                 TOUCH_CS_PORT, TOUCH_CS_PIN,
                 TOUCH_IRQ_PORT, TOUCH_IRQ_PIN);
    
    // Optional: LCD Rotation logic
    // XPT2046_SetRotation(&htouch, 1); // Landscape
    
    UART_Debug_Printf("Touch the screen! (Read Loop)\r\n");
    
    while(1) {
        if(XPT2046_IsTouched(&htouch)) {
            uint16_t x, y;
            if(XPT2046_GetCoordinates(&htouch, &x, &y)) {
                UART_Debug_Printf("Touch: X=%d, Y=%d\r\n", x, y);
                // Simple drawing debounce or wait
                HAL_Delay(50); 
            }
        }
        HAL_Delay(10);
    }
}
