# LED Driver Module

A generic, dependency-injection based LED driver for STM32.

## Features
*   **Decoupled**: Hardware pins are registered at runtime/init, not hardcoded.
*   **Active Level Support**: Works with both Active High and Active Low (sink) LEDs.
*   **Safe**: Checks for invalid IDs and double registration.

## Usage

### 1. Initialization
In your `user_main` or setup sequence:

```c
#include "io/led.h"

void app_main(void)
{
    // Register the Green LED (Nucleo LD2 is usually PA5, Active High)
    LED_Register(LED_1, GPIOA, GPIO_PIN_5, LED_ACTIVE_HIGH);

    // Register a Red LED connected to PB12 (Active Low)
    LED_Register(LED_2, GPIOB, GPIO_PIN_12, LED_ACTIVE_LOW);
}
```

### 2. Control
```c
LED_On(LED_1);    // Turn on
LED_Off(LED_2);   // Turn off
LED_Toggle(LED_1); // Toggle
```
