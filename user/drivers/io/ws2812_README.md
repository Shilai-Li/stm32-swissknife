# WS2812B / SK6812 LED Driver

A PWM + DMA based driver for WS2812B Addressable LEDs (NeoPixel).

## Features
*   **Non-Blocking**: Uses hardware DMA to send data, freeing up the CPU.
*   **Efficient**: Dynamic waveform generation based on Timer Period (ARR).
*   **Portable**: Supports any timer and any number of LEDs (configured via `WS2812_MAX_LEDS`).

## Hardware Configuration (CubeMX)

1.  **Timer Config** (e.g., TIM2):
    *   **Channel X**: PWM Generation CHx.
    *   **Prescaler (PSC)**: 0 (for fast 72MHz+ clocks).
    *   **Counter Period (ARR)**: Set to generate **800 kHz** (1.25 us periode).
        *   Formula: `TimerFreq / 800000 - 1`.
        *   Example (72MHz): `72000000 / 800000 - 1` = **89**.
    
2.  **DMA Settings** (Timer -> DMA Settings):
    *   Add Request for `TIMx_CHx` (or `TIMx_UP` depending on board).
    *   **Direction**: Memory To Peripheral.
    *   **Mode**: **Normal** (Direct Mode). **DO NOT use Circular Mode!**
    *   **Data Width**: **Half Word (16 bit)** for both Memory and Peripheral.
    *   **Priority**: High or Very High.

3.  **Interrupts**:
    *   Enable Timer Global Interrupt (needed for DMA transfer complete callback).

## Integration Guide

### 1. Define Handle globally
**CRITICAL**: The `WS2812_HandleTypeDef` contains a large DMA buffer (hundreds of bytes). **Do NOT** define it on the stack (inside a function). Define it as a global static variable.

```c
// main.c
// Increase this in ws2812.h if needed
#define WS2812_MAX_LEDS 64 

WS2812_HandleTypeDef hws; // <--- GLOBAL
```

### 2. DMA Callback Hook
You must forward the Timer PWM Pulse Finished Interrupt to the driver.

In `main.c` (or `stm32f4xx_it.c`):

```c
void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
    if (htim == &htim2) { // Replace with your Timer
        WS2812_DmaCallback(&hws);
    }
}
```

### 3. Usage

```c
// In Setup
WS2812_Init(&hws, &htim2, TIM_CHANNEL_1, 8); // 8 LEDs

// Set Colors
WS2812_Fill(&hws, 0, 0, 0); // Clear
WS2812_SetPixelColor(&hws, 0, 255, 0, 0); // LED 0 Red
WS2812_SetPixelColor(&hws, 1, 0, 255, 0); // LED 1 Green

// Send to Strip
WS2812_Show(&hws);
```
