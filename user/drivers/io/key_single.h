#ifndef __KEY_DRIVER_H
#define __KEY_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdbool.h>

// Max number of keys supported
#define MAX_KEYS 4

// Key State
typedef enum {
    KEY_STATE_IDLE = 0,
    KEY_STATE_PRESSED,
    KEY_STATE_LONG_PRESSED,
    KEY_STATE_RELEASED
} KeyState_t;

// Key Event (One-shot)
typedef enum {
    KEY_EVENT_NONE = 0,
    KEY_EVENT_PRESS,
    KEY_EVENT_CLICK,
    KEY_EVENT_LONG_PRESS,
    KEY_EVENT_LONG_RELEASE
} KeyEvent_t;

// Active Level
typedef enum {
    KEY_ACTIVE_LOW = 0,  // Pin LOW = Pressed (Pull-up required)
    KEY_ACTIVE_HIGH = 1  // Pin HIGH = Pressed (Pull-down required)
} KeyActiveLevel_t;

// Register a key (ID: 0 to MAX_KEYS-1)
void Key_Register(uint8_t key_id, GPIO_TypeDef* port, uint16_t pin, KeyActiveLevel_t active_level);

// Scan all keys (Poll every 10-20ms)
void Key_Scan(void);

// Get Current State (Real-time)
KeyState_t Key_GetState(uint8_t key_id);

// Get Event (One-shot, cleared after read)
KeyEvent_t Key_GetEvent(uint8_t key_id);

#ifdef __cplusplus
}
#endif

#endif /* __KEY_DRIVER_H */
