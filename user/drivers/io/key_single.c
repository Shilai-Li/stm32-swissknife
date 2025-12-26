#include "key_single.h"

// Configuration
#define DEBOUNCE_TIME_MS      20
#define LONG_PRESS_TIME_MS    1000

typedef struct {
    GPIO_TypeDef* port;
    uint16_t      pin;
    bool          active_level;
    bool          is_registered;
    
    // Internal State
    bool          last_raw_state;
    uint32_t      state_change_time;
    KeyState_t    current_state;
    KeyEvent_t    pending_event;
    
    // Long Press logic
    bool          long_press_generated;
} KeyHandle_t;

static KeyHandle_t keys[MAX_KEYS] = {0};

/* ========== Public API Implementation ========== */

void Key_Register(uint8_t key_id, GPIO_TypeDef* port, uint16_t pin, KeyActiveLevel_t active_level)
{
    if (key_id >= MAX_KEYS) return;
    
    keys[key_id].port = port;
    keys[key_id].pin = pin;
    keys[key_id].active_level = (bool)active_level;
    keys[key_id].is_registered = true;
    
    // Initialize State
    keys[key_id].current_state = KEY_STATE_IDLE;
    keys[key_id].pending_event = KEY_EVENT_NONE;
    keys[key_id].last_raw_state = false; // Assume released initially
}

// Internal Helper: Read Normalized Raw State (True = Pressed)
static bool Key_ReadRaw(uint8_t id) {
    if (!keys[id].is_registered) return false;
    
    GPIO_PinState pin_state = HAL_GPIO_ReadPin(keys[id].port, keys[id].pin);
    bool logic_level = (pin_state == GPIO_PIN_SET);
    
    return (keys[id].active_level == logic_level);
}

void Key_Scan(void)
{
    uint32_t now = HAL_GetTick();
    
    for (int i = 0; i < MAX_KEYS; i++) {
        if (!keys[i].is_registered) continue;
        
        bool raw_pressed = Key_ReadRaw(i);
        
        // 1. Debounce Layer
        // If raw state changed, reset timer
        if (raw_pressed != keys[i].last_raw_state) {
            keys[i].state_change_time = now;
            keys[i].last_raw_state = raw_pressed;
        }
        
        // If state is stable for DEBOUNCE time
        if ((now - keys[i].state_change_time) > DEBOUNCE_TIME_MS) {
            
            // 2. Logic Layer (Stable State Machine)
            
            // Check for Rising Edge (Press)
            if (raw_pressed && keys[i].current_state == KEY_STATE_IDLE) {
                 keys[i].current_state = KEY_STATE_PRESSED;
                 keys[i].pending_event = KEY_EVENT_PRESS;
                 keys[i].long_press_generated = false;
            }
            // Check for Falling Edge (Release)
            else if (!raw_pressed && (keys[i].current_state == KEY_STATE_PRESSED || keys[i].current_state == KEY_STATE_LONG_PRESSED)) {
                
                if (keys[i].current_state == KEY_STATE_LONG_PRESSED) {
                    keys[i].pending_event = KEY_EVENT_LONG_RELEASE;
                } else {
                    keys[i].pending_event = KEY_EVENT_CLICK; // Short Click
                }
                keys[i].current_state = KEY_STATE_IDLE;
            }
            
            // Check for Long Press Hold
            if (raw_pressed && keys[i].current_state == KEY_STATE_PRESSED) {
                if ((now - keys[i].state_change_time) > LONG_PRESS_TIME_MS) {
                    keys[i].current_state = KEY_STATE_LONG_PRESSED;
                    keys[i].pending_event = KEY_EVENT_LONG_PRESS;
                    keys[i].long_press_generated = true;
                }
            }
        }
    }
}

KeyState_t Key_GetState(uint8_t key_id) {
    if (key_id >= MAX_KEYS || !keys[key_id].is_registered) return KEY_STATE_IDLE;
    return keys[key_id].current_state;
}

KeyEvent_t Key_GetEvent(uint8_t key_id) {
    if (key_id >= MAX_KEYS || !keys[key_id].is_registered) return KEY_EVENT_NONE;
    
    // Read and Clear
    KeyEvent_t evt = keys[key_id].pending_event;
    keys[key_id].pending_event = KEY_EVENT_NONE;
    return evt;
}
