/**
 * @file buzzer_tests.c
 * @brief Test for Buzzer Melody
 */

#include "buzzer.h"
#include "uart.h"
#include <stdio.h>

extern TIM_HandleTypeDef htim3; // Adjust to your PWM Timer

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

void Test_Buzzer_Entry(void) {
    UART_Init();
    UART_Debug_Printf("\r\n=== Buzzer/Melody Test ===\r\n");
    
    static Buzzer_Handle_t buzz;
    
    // Config: TIM3 Channel 1 (Make sure CubeMX Configured Prescaler to 1MHz!)
    Buzzer_Init(&buzz, &htim3, TIM_CHANNEL_1);
    
    UART_Debug_Printf("Playing Melody...\r\n");
    Buzzer_PlayMelody(&buzz, melody, duration, sizeof(melody)/sizeof(uint32_t));
    
    UART_Debug_Printf("Done.\r\n");
    
    // Interactive Test
    UART_Debug_Printf("Beep 1kHz for 1s...\r\n");
    Buzzer_Tone(&buzz, 1000, 1000);
    
    while(1) {
        Buzzer_Loop(&buzz); // Handles auto-stop
    }
}