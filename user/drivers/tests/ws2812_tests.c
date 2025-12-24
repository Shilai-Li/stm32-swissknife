/**
 * @file ws2812_tests.c
 * @brief WS2812 Driver Test Code
 *
 * CubeMX Config:
 * 1. TIM4 (or other) -> Channel 1 PWM Generation.
 *    - Prescaler: 0
 *    - Counter Period (ARR): 89 (Assuming 72MHz Clock creates 800kHz). 
 *                            (72000000 / 800000) - 1 = 89.
 *    - Pulse: 0
 * 2. DMA Settings -> TIM4_CH1 (or TIM4_UP?)
 *    - Direction: Memory To Peripheral
 *    - Priority: High
 *    - Mode: Normal (Not Circular!)
 *    - Data Width: Half Word (16 bit) -> Half Word.
 */

#include "ws2812.h"
#include "uart.h"

// --- Configuration ---
extern TIM_HandleTypeDef htim4; // Assume TIM4 CH1

WS2812_HandleTypeDef hws;

// We need to link the HAL/ISR callback to driver
// Since we don't want to modify main.c, we assume polling or user links calls manually.
// For test loop, we can just rely on non-blocking behavior or add a hacky hook if needed.
// However, WS2812_Show needs HAL_TIM_PWM_PulseFinishedCallback to be called to stop DMA.
// In a clean architecture, User adds this to stm32f1xx_it.c or main.c.
// Here we will mock it: The HAL interrupt handler calls the callback. 
// If main.c doesn't call us, Busy flag will never clear.
// WARN: User must hook `WS2812_DmaCallback(&hws)` in `HAL_TIM_PWM_PulseFinishedCallback`.

void user_main(void)
{
    UART_Init();
    UART_Debug_Printf("\r\n--- WS2812 Test Start ---\r\n");

    // 1. Initialize (8 LEDs)
    UART_Debug_Printf("Init WS2812 on TIM4 CH1...\r\n");
    WS2812_Init(&hws, &htim4, TIM_CHANNEL_1, 8);
    
    // 2. Rainbow Cycle
    UART_Debug_Printf("Running Rainbow Loop...\r\n");
    
    uint8_t offset = 0;
    while (1) {
        if (!hws.Busy) {
             for (int i=0; i<8; i++) {
                 // Simple color wheel logic
                 int pos = (i * 30 + offset) % 255;
                 uint8_t r, g, b;
                 
                 if(pos < 85) {
                     r = pos * 3;
                     g = 255 - pos * 3;
                     b = 0;
                 } else if(pos < 170) {
                     pos -= 85;
                     r = 255 - pos * 3;
                     g = 0;
                     b = pos * 3;
                 } else {
                     pos -= 170;
                     r = 0;
                     g = pos * 3;
                     b = 255 - pos * 3;
                 }
                 
                 // Dimming for test safety (don't draw too much amps)
                 WS2812_SetPixelColor(&hws, i, r/10, g/10, b/10);
             }
             
             WS2812_Show(&hws);
             offset++;
        }
        
        HAL_Delay(50);
        
        // Polling Hack for Test Environment if Interrupts aren't hooked:
        // In real hardware, ISR must fire.
        // For test without ISR hook, this will hang on busy after 1st frame.
        // We assume user hooks it up.
    }
}

// Optional: If main.c uses strong/weak linking, we can define the callback here?
// void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim) {
//     if (htim == &htim4) {
//         WS2812_DmaCallback(&hws);
//     }
// }
