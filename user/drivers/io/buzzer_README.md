# Buzzer Driver

A generic driver for both **Active** (GPIO-based) and **Passive** (PWM-based) buzzers.

## Features
*   **Dual Mode Support**:
    *   **Passive Mode**: Uses STM32 Hardware Timer PWM to generate precise frequencies (Melodies).
    *   **Active Mode**: Uses simple GPIO Toggle for fixed-frequency buzzers.
*   **Non-Blocking Support**: `Buzzer_Loop()` handles duration timeout without `HAL_Delay`.
*   **Blocking Melody**: `Buzzer_PlayMelody()` allows easy playback of note arrays.
*   **Dependency Injection**: Hardware timer/GPIO handles are passed during initialization.

## Hardware Configuration (CubeMX)

### For Passive Buzzer (PWM)
1.  **Select a Timer** (e.g., TIM3).
2.  Set Channel to **PWM Generation CHx**.
3.  **Prescaler**: **Important!** Set Prescaler so that the counter clock is **1 MHz (1 us per tick)**.
    *   Example: If PCLK1 is 84 MHz, set Prescaler to `84-1`.
    *   This simplifies frequency calculation (ARR = 1,000,000 / Freq).
4.  **Counter Period (ARR)**: Can be default (will be changed dynamically).

### For Active Buzzer (GPIO)
1.  Configure any pin as **GPIO_Output**.

## Usage Examples

### 1. Simple Active Buzzer
```c
Buzzer_Handle_t hBuzz;

// In Setup
Buzzer_InitGPIO(&hBuzz, GPIOB, GPIO_PIN_0);

// In Loop
Buzzer_Tone(&hBuzz, 0, 100); // "0" Frequency ignored for Active, plays for 100ms
```

### 2. PWM Melody
```c
Buzzer_Handle_t hMusic;

// In Setup
// Assuming TIM3 CH1 is configured
Buzzer_Init(&hMusic, &htim3, TIM_CHANNEL_1);

// Play A4 (440Hz) for 500ms
Buzzer_Tone(&hMusic, NOTE_A4, 500);

// In Main Loop (for non-blocking stop)
while(1) {
    Buzzer_Loop(&hMusic);
}
```
