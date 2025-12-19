#include "flexible_button.h"
#include "flexible_button_port.h"
#include "uart.h" // For debug printing

// Define the number of buttons
#define USER_BUTTON_COUNT 1

// Static array to store button objects
static flexible_button_t user_buttons[USER_BUTTON_COUNT];

// Hardware configuration for buttons
// Modify this to match your actual hardware
typedef struct {
    GPIO_TypeDef* port;
    uint16_t pin;
    uint8_t active_level; // 0 for low active, 1 for high active
} ButtonHardware_t;

static const ButtonHardware_t button_hw[USER_BUTTON_COUNT] = {
    {GPIOB, GPIO_PIN_11, 0} // Example: User Button on PB11, Active Low
};

/**
 * @brief Read button state function required by FlexibleButton
 * 
 * @param button Pointer to the button structure
 * @return uint8_t 1 if active, 0 if inactive
 */
static uint8_t common_btn_read(void *arg)
{
    flexible_button_t *btn = (flexible_button_t *)arg;
    uint32_t btn_id = btn->id;
    
    if (btn_id >= USER_BUTTON_COUNT) return 0;

    // Read hardware logic level
    GPIO_PinState pin_state = HAL_GPIO_ReadPin(button_hw[btn_id].port, button_hw[btn_id].pin);
    
    // Convert to logical active state
    // If active low (0), and pin is RESET (0), then button is active (return 1)
    if (button_hw[btn_id].active_level == 0) {
        return (pin_state == GPIO_PIN_RESET) ? 1 : 0;
    } else {
        return (pin_state == GPIO_PIN_SET) ? 1 : 0;
    }
}

/**
 * @brief Default event callback
 */
void btn_event_callback(void *arg)
{
    flexible_button_t *btn = (flexible_button_t *)arg;
    
    // Print event for debugging
    // UART_Debug_Printf("Button ID: %d, Event: ", btn->id);
    // Note: Assuming UART_Debug_Printf is available from uart.h
    
    switch (btn->event)
    {
        case FLEX_BTN_PRESS_DOWN:
            UART_Debug_Printf("Button [%d] Press Down\r\n", btn->id);
            break;
        case FLEX_BTN_PRESS_CLICK:
            UART_Debug_Printf("Button [%d] Click\r\n", btn->id);
            break;
        case FLEX_BTN_PRESS_DOUBLE_CLICK:
            UART_Debug_Printf("Button [%d] Double Click\r\n", btn->id);
            break;
        case FLEX_BTN_PRESS_REPEAT_CLICK:
            UART_Debug_Printf("Button [%d] Repeat Click\r\n", btn->id);
            break;
        case FLEX_BTN_PRESS_SHORT_START:
            UART_Debug_Printf("Button [%d] Short Start\r\n", btn->id);
            break;
        case FLEX_BTN_PRESS_SHORT_UP:
            UART_Debug_Printf("Button [%d] Short Up\r\n", btn->id);
            break;
        case FLEX_BTN_PRESS_LONG_START:
            UART_Debug_Printf("Button [%d] Long Start\r\n", btn->id);
            break;
        case FLEX_BTN_PRESS_LONG_HOLD:
            // This happens repeatedly during long press
            // UART_Debug_Printf("Button [%d] Long Hold\r\n", btn->id);
            break;
        case FLEX_BTN_PRESS_LONG_UP:
            UART_Debug_Printf("Button [%d] Long Up\r\n", btn->id);
            break;
        case FLEX_BTN_PRESS_MAX:
             break;
        case FLEX_BTN_PRESS_NONE:
             break;
    }
}

void FlexibleButton_Init(void)
{
    // Initialize GPIOs
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // Enable clocks (Add more if needed for other ports)
    __HAL_RCC_GPIOB_CLK_ENABLE();
    
    for (int i = 0; i < USER_BUTTON_COUNT; i++) {
        GPIO_InitStruct.Pin = button_hw[i].pin;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        // Logic: if active low -> pull up, if active high -> pull down
        GPIO_InitStruct.Pull = (button_hw[i].active_level == 0) ? GPIO_PULLUP : GPIO_PULLDOWN;
        HAL_GPIO_Init(button_hw[i].port, &GPIO_InitStruct);
        
        // Initialize FlexibleButton structure
        user_buttons[i].id = i;
        user_buttons[i].usr_button_read = common_btn_read;
        user_buttons[i].cb = btn_event_callback; // Common callback
        user_buttons[i].pressed_logic_level = 1; // Logic level 1 represents "Active" in our read function
        user_buttons[i].short_press_start_tick = FLEX_BTN_SCAN_INT_MS; // default
        user_buttons[i].long_press_start_tick = 1000 / FLEX_BTN_SCAN_INT_MS; // 1s for long press
        user_buttons[i].long_hold_start_tick = 1000 / FLEX_BTN_SCAN_INT_MS;  // 1s for long hold
        
        flexible_button_register(&user_buttons[i]);
    }
}

void FlexibleButton_Scan(void)
{
    // Important: FlexibleButton relies on scanning repeatedly
    flexible_button_scan();
}
