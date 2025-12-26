/**
 * @file buzzer_tests.c
 * @brief Test for Buzzer Melody
 */

#include "io/buzzer.h"
#include "usart.h"
#include "uart.h"
#include "usb_cdc.h" // Add for build consistency
#include <stdio.h>

#define CH_DEBUG 2

// Super Mario Theme Intro (Simplified)
static const uint32_t melody[] = {
    NOTE_E5, NOTE_E5, 0, NOTE_E5, 
    0, NOTE_C5, NOTE_E5, 0,
    NOTE_G5, 0, 0,
    NOTE_G4
};

static const uint32_t duration[] = {
    100, 100, 100, 100,
    100, 100, 100, 100,
    100, 300, 300,
    100
};

void user_main(void) {
    // Hardware Setup
    UART_Register(CH_DEBUG, &huart2);
    // UART_Init(); // Deprecated, use Register pattern if possible or rely on CubeMX Init

    UART_SendString(CH_DEBUG, "\r\n=== Buzzer/Melody Test ===\r\n");
    
    static Buzzer_Handle_t buzz;
    
    // Config: Active Buzzer on PA4 (Nucleo D2) or PC15 (Blue Pill)
    // For Passive PWM test, change to Buzzer_Init(&buzz, &htim3, TIM_CHANNEL_1);
    Buzzer_InitGPIO(&buzz, GPIOA, GPIO_PIN_4);
    
    UART_SendString(CH_DEBUG, "Playing Melody (Simulated on Active Buzzer)...\r\n");
    
    // Note: PlayMelody uses HAL_Delay (Blocking)
    Buzzer_PlayMelody(&buzz, melody, duration, sizeof(melody)/sizeof(uint32_t));
    
    UART_SendString(CH_DEBUG, "Done.\r\n");
    
    // Interactive Test
    UART_SendString(CH_DEBUG, "Beep 1kHz for 1s...\r\n");
    
    // Start a tone (Active buzzer will just turn ON, frequency ignored)
    Buzzer_Tone(&buzz, 1000, 1000);
    
    while(1) {
        Buzzer_Loop(&buzz); // Handles auto-stop based on HAL_Tick
    }
}
