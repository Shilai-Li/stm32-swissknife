/**
 * @file ws2812_tests.c
 * @brief WS2812 Driver Test
 */

#include "io/ws2812.h"
#include "uart.h"
#include "usb_cdc.h"
#include "tim.h" // For htimx

#define CH_DEBUG 2

// GLOBAL Handle
WS2812_HandleTypeDef hws;

// External Timer Handle (Depends on CubeMX Config)
// Assuming TIM2 CH1 for Nucleo/F446
extern TIM_HandleTypeDef htim2; 
// extern TIM_HandleTypeDef htim1; // F103 common

void user_main(void)
{
    UART_Register(CH_DEBUG, &huart2);
    // UART_Init(); 

    UART_SendString(CH_DEBUG, "\r\n--- WS2812 Test Start ---\r\n");
    UART_SendString(CH_DEBUG, "Warning: Ensure Timer PWM+DMA is Configured in CubeMX!\r\n");
    UART_SendString(CH_DEBUG, "Warning: Ensure HAL_TIM_PWM_PulseFinishedCallback calls WS2812_DmaCallback!\r\n");

    // Initialize (8 LEDs on TIM2 Channel 1)
    // Adjust TIM and Channel according to your hardware wiring!
    WS2812_Init(&hws, &htim2, TIM_CHANNEL_1, 8);
    
    UART_SendString(CH_DEBUG, "Running Rainbow Loop...\r\n");
    
    uint8_t offset = 0;
    while (1) {
        if (!hws.Busy) {
             for (int i=0; i<8; i++) {
                 // Simple color wheel logic
                 int pos = (i * 30 + offset) % 255;
                 uint8_t r, g, b;
                 
                 if(pos < 85) {
                     r = pos * 3; g = 255 - pos * 3; b = 0;
                 } else if(pos < 170) {
                     pos -= 85; r = 255 - pos * 3; g = 0; b = pos * 3;
                 } else {
                     pos -= 170; r = 0; g = pos * 3; b = 255 - pos * 3;
                 }
                 
                 // Dimming (10% brightness)
                 WS2812_SetPixelColor(&hws, i, r/10, g/10, b/10);
             }
             
             WS2812_Show(&hws);
             offset++;
        }
        
        HAL_Delay(50);
    }
}

// Optional Hook for testing if not in main.c
// Note: This might conflict if main.c also defines it without weak linkage.
/*
void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM2) {
        WS2812_DmaCallback(&hws);
    }
}
*/
