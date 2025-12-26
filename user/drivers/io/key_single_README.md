# Key Driver

A generic, non-blocking, debounced Key/Button driver for STM32.

## Features
*   **Dependency Injection**: Register any GPIO at runtime using `Key_Register`.
*   **Software Debouncing**: Filters out mechanical noise (default 20ms).
*   **Rich Events**:
    *   `PRESS`: Triggered immediately after stable contact.
    *   `CLICK`: Triggered on release (if duration < LongPress).
    *   `LONG_PRESS`: Triggered if held > 1000ms.
    *   `LONG_RELEASE`: Triggered on release after a long press.
*   **Multi-Instance**: Supports up to `MAX_KEYS` (default 4) independent buttons.

## Usage

### 1. Initialization
In `user_main`:

```c
#include "io/key.h"

void app_main(void)
{
    // Register Nucleo User Button (PC13, Active Low)
    // ID: 0 (You can define enum: KEY_USER = 0)
    Key_Register(0, GPIOC, GPIO_PIN_13, KEY_ACTIVE_LOW);
}
```

### 2. Polling Loop
You must call `Key_Scan()` regularly (e.g., every 10ms or 20ms).

```c
while(1) {
    Key_Scan(); // Updates state machine
    
    // Check for Events
    KeyEvent_t evt = Key_GetEvent(0);
    
    if (evt == KEY_EVENT_CLICK) {
        printf("Button Clicked!\r\n");
    } else if (evt == KEY_EVENT_LONG_PRESS) {
        printf("Long Press!\r\n");
    }
    
    HAL_Delay(10);
}
```
