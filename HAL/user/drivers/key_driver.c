/**
 * @file    key_driver.c
 * @brief   Key Driver Module Implementation
 * @details Implementation of key scanning, debouncing, and event detection for STM32
 * @author  STM32 HAL Developer
 * @date    2024
 */

#include "key_driver.h"

#include "uart_driver.h"
#include "delay_driver.h"

/* ========================================= PRIVATE VARIABLES ========================================= */

/**
 * @brief Global key driver configuration
 */
static KeyConfig_t key_config = {
    .debounce_time_ms = KEY_DEFAULT_DEBOUNCE_TIME_MS,
    .long_press_time_ms = KEY_DEFAULT_LONG_PRESS_TIME_MS,
    .auto_clear_events = true
};

/**
 * @brief Array to store all single key configurations
 */
static KeySingle_t single_keys[MAX_SINGLE_KEYS] = {
    { 
        .port = GPIOB, 
        .pin = GPIO_PIN_11, 
        .active_level = false, 
        .state = KEY_STATE_IDLE,
        .event = KEY_EVENT_NONE,
        .raw_state = false,
        .last_raw_state = false,
        .press_time = 0,
        .debounce_time = 0,
        .long_press_time = 0
    }
};

#if USE_MATRIX_KEY
/**
 * @brief Example 4x4 matrix keypad configuration (Matrix A)
 */
static GPIO_TypeDef *row_ports_A[] = {GPIOB, GPIOB, GPIOB, GPIOB};
static uint16_t row_pins_A[]       = {GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_2, GPIO_PIN_3};
static GPIO_TypeDef *col_ports_A[] = {GPIOC, GPIOC, GPIOC, GPIOC};
static uint16_t col_pins_A[]       = {GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_2, GPIO_PIN_3};

/**
 * @brief Array to store all matrix keypad configurations
 */
static KeyMatrix_t matrix_keys[MAX_MATRIX_KEYS] = {
    { 
        .row_ports = row_ports_A, 
        .row_pins = row_pins_A, 
        .col_ports = col_ports_A, 
        .col_pins = col_pins_A, 
        .rows = 4, 
        .cols = 4,
        .key_states = NULL,
        .key_events = NULL
    } // 4x4 matrix
};
#endif

/* ========================================= PRIVATE FUNCTION PROTOTYPES ========================================= */

/**
 * @brief Enable GPIO clock automatically based on port
 * @param port GPIO port to enable clock for
 */
static void Key_EnableClock(GPIO_TypeDef *port);

/**
 * @brief Update key state machine for single keys
 * @param key Pointer to key structure
 */
static void Key_UpdateStateMachine(KeySingle_t *key);

/* ========================================= PRIVATE FUNCTIONS ========================================= */

static void Key_EnableClock(GPIO_TypeDef *port)
{
    if (port == GPIOA) __HAL_RCC_GPIOA_CLK_ENABLE();
    else if (port == GPIOB) __HAL_RCC_GPIOB_CLK_ENABLE();
    else if (port == GPIOC) __HAL_RCC_GPIOC_CLK_ENABLE();
    else if (port == GPIOD) __HAL_RCC_GPIOD_CLK_ENABLE();
    else if (port == GPIOE) __HAL_RCC_GPIOE_CLK_ENABLE();
    // Add other ports as needed for your specific STM32 series
}

static void Key_UpdateStateMachine(KeySingle_t *key)
{
    uint32_t current_time = HAL_GetTick();
    
    // State machine for key events
    switch (key->state) {
        case KEY_STATE_IDLE:
            if (key->raw_state) {
                // Key press detected, start debounce
                key->debounce_time = current_time;
                key->state = KEY_STATE_PRESSED;
            }
            break;
            
        case KEY_STATE_PRESSED:
            if (key->raw_state) {
                // Check if debounce time has elapsed
                if ((current_time - key->debounce_time) >= key_config.debounce_time_ms) {
                    // Valid key press, set event
                    key->event = KEY_EVENT_PRESS;
                    key->press_time = current_time;
                    key->long_press_time = current_time;
                    
                    // Check if it's a long press
                    if ((current_time - key->long_press_time) >= key_config.long_press_time_ms) {
                        key->state = KEY_STATE_LONG_PRESSED;
                    }
                }
            } else {
                // Key released before debounce
                key->state = KEY_STATE_IDLE;
            }
            break;
            
        case KEY_STATE_LONG_PRESSED:
            if (key->raw_state) {
                // Still pressing, check if long press event needs to be set
                if (key->event != KEY_EVENT_LONG_PRESS) {
                    key->event = KEY_EVENT_LONG_PRESS;
                }
            } else {
                // Key released from long press
                key->event = KEY_EVENT_LONG_RELEASE;
                key->state = KEY_STATE_RELEASED;
            }
            break;
            
        case KEY_STATE_RELEASED:
            // Check for debounce on release
            if (!key->raw_state) {
                if ((current_time - key->debounce_time) >= key_config.debounce_time_ms) {
                    // If it wasn't a long press, generate click event
                    if (key->event != KEY_EVENT_LONG_RELEASE) {
                        key->event = KEY_EVENT_CLICK;
                    }
                    key->state = KEY_STATE_IDLE;
                }
            } else {
                // Key pressed again before debounce complete
                key->debounce_time = current_time;
            }
            break;
            
        default:
            // Reset to idle state if in unknown state
            key->state = KEY_STATE_IDLE;
            key->event = KEY_EVENT_NONE;
            break;
    }
}

/* ========================================= PUBLIC FUNCTIONS ========================================= */

/**
 * @brief Get default key configuration
 * @return Default key configuration structure
 */
KeyConfig_t Key_GetDefaultConfig(void)
{
    KeyConfig_t default_config = {
        .debounce_time_ms = KEY_DEFAULT_DEBOUNCE_TIME_MS,
        .long_press_time_ms = KEY_DEFAULT_LONG_PRESS_TIME_MS,
        .auto_clear_events = true
    };
    return default_config;
}

/**
 * @brief Initialize key driver with default configuration
 * @details Configures GPIO pins and initializes key structures with default settings
 */
void Key_Init(void)
{
    Key_InitWithConfig(NULL);
}

/**
 * @brief Initialize key driver with custom configuration
 * @details Configures GPIO pins and initializes key structures with custom settings
 * @param config Pointer to configuration structure (NULL to use default)
 */
void Key_InitWithConfig(KeyConfig_t *config)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* Initialize independent keys */
#if USE_SINGLE_KEY
    for (uint8_t i = 0; i < MAX_SINGLE_KEYS; i++) {
        // Enable clock for the GPIO port
        Key_EnableClock(single_keys[i].port);

        // Configure GPIO pin
        GPIO_InitStruct.Pin = single_keys[i].pin;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        
        // Configure pull resistor based on active level
        if (single_keys[i].active_level) {
            GPIO_InitStruct.Pull = GPIO_PULLDOWN;  // Active high -> Pull down
        } else {
            GPIO_InitStruct.Pull = GPIO_PULLUP;    // Active low -> Pull up
        }
        
        // Initialize GPIO
        HAL_GPIO_Init(single_keys[i].port, &GPIO_InitStruct);
        
        // Initialize state variables
        single_keys[i].state = KEY_STATE_IDLE;
        single_keys[i].event = KEY_EVENT_NONE;
        single_keys[i].raw_state = false;
        single_keys[i].last_raw_state = false;
        single_keys[i].press_time = 0;
        single_keys[i].debounce_time = 0;
        single_keys[i].long_press_time = 0;
        
        // Update configuration if provided
        if (config != NULL) {
            key_config = *config;
        }
    }
#endif
    
    // If configuration provided, update global config
    if (config != NULL) {
        key_config = *config;
    }

    /* Initialize matrix keypads */
#if USE_MATRIX_KEY
    for (uint8_t k = 0; k < MAX_MATRIX_KEYS; k++) {
        // Initialize row pins (output)
        for (uint8_t r = 0; r < matrix_keys[k].rows; r++) {
            Key_EnableClock(matrix_keys[k].row_ports[r]);

            GPIO_InitStruct.Pin = matrix_keys[k].row_pins[r];
            GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
            GPIO_InitStruct.Pull = GPIO_NOPULL;
            HAL_GPIO_Init(matrix_keys[k].row_ports[r], &GPIO_InitStruct);

            // Set initial state to low
            HAL_GPIO_WritePin(matrix_keys[k].row_ports[r], matrix_keys[k].row_pins[r], GPIO_PIN_RESET);
        }

        // Initialize column pins (input)
        for (uint8_t c = 0; c < matrix_keys[k].cols; c++) {
            Key_EnableClock(matrix_keys[k].col_ports[c]);

            GPIO_InitStruct.Pin = matrix_keys[k].col_pins[c];
            GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
            GPIO_InitStruct.Pull = GPIO_PULLDOWN;  // Pull down for active high keys
            HAL_GPIO_Init(matrix_keys[k].col_ports[c], &GPIO_InitStruct);
        }
    }
#endif
}

void Key_Scan(void)
{
    uint32_t current_time = HAL_GetTick();
    
    /* Scan independent keys */
#if USE_SINGLE_KEY
    for (uint8_t i = 0; i < MAX_SINGLE_KEYS; i++) {
        // Read raw state from GPIO
        bool gpio_state = (HAL_GPIO_ReadPin(single_keys[i].port, single_keys[i].pin) == GPIO_PIN_SET);
        
        // Update raw state based on active level
        single_keys[i].raw_state = (single_keys[i].active_level ? gpio_state : !gpio_state);
        
        // Check for state change to start debounce timing
        if (single_keys[i].raw_state != single_keys[i].last_raw_state) {
            single_keys[i].debounce_time = current_time;
        }
        
        // Only update state machine if debounce time has passed
            if ((current_time - single_keys[i].debounce_time) >= key_config.debounce_time_ms) {
            Key_UpdateStateMachine(&single_keys[i]);
        }
        
        // Save current state for next scan
        single_keys[i].last_raw_state = single_keys[i].raw_state;
    }
#endif

    /* Scan matrix keypads */
#if USE_MATRIX_KEY
    for (uint8_t m = 0; m < MAX_MATRIX_KEYS; m++) {
        for (uint8_t r = 0; r < matrix_keys[m].rows; r++) {
            // Drive one row high
            HAL_GPIO_WritePin(matrix_keys[m].row_ports[r], matrix_keys[m].row_pins[r], GPIO_PIN_SET);
            
            // Small delay for signal stabilization
            // Note: For better timing precision, use a small delay function instead of HAL_Delay
            for (volatile uint8_t delay = 0; delay < 10; delay++);

            for (uint8_t c = 0; c < matrix_keys[m].cols; c++) {
                // Read column state
                bool col_state = (HAL_GPIO_ReadPin(matrix_keys[m].col_ports[c], matrix_keys[m].col_pins[c]) == GPIO_PIN_SET);
                
                // TODO: Implement matrix key state machine similar to single keys
                // This would require 2D arrays for state tracking
            }

            // Reset row to low
            HAL_GPIO_WritePin(matrix_keys[m].row_ports[r], matrix_keys[m].row_pins[r], GPIO_PIN_RESET);
        }
    }
#endif
}

KeyState_t Key_GetSingleState(uint8_t key_id)
{
#if USE_SINGLE_KEY
    if (key_id < MAX_SINGLE_KEYS) {
        return single_keys[key_id].state;
    }
#endif
    return KEY_STATE_IDLE; // Return idle state for invalid key_id
}

KeyEvent_t Key_GetSingleEvent(uint8_t key_id)
{
#if USE_SINGLE_KEY
    if (key_id < MAX_SINGLE_KEYS) {
        // Get current event
        KeyEvent_t current_event = single_keys[key_id].event;
        
        // Clear event after reading if configured
        if (key_config.auto_clear_events && current_event != KEY_EVENT_NONE) {
            single_keys[key_id].event = KEY_EVENT_NONE;
        }
        
        return current_event;
    }
#endif
    return KEY_EVENT_NONE; // Return no event for invalid key_id
}

bool Key_IsSinglePressed(uint8_t key_id)
{
    KeyState_t state = Key_GetSingleState(key_id);
    return (state == KEY_STATE_PRESSED || state == KEY_STATE_LONG_PRESSED);
}

KeyState_t Key_GetMatrixState(uint8_t matrix_id, uint8_t row, uint8_t col)
{
#if USE_MATRIX_KEY
    if (matrix_id < MAX_MATRIX_KEYS && 
        row < matrix_keys[matrix_id].rows && 
        col < matrix_keys[matrix_id].cols) {
        // TODO: Implement matrix key state retrieval
        // This would require state tracking arrays for each matrix key
        return KEY_STATE_IDLE; // Placeholder
    }
#endif
    return KEY_STATE_IDLE; // Return idle state for invalid parameters
}

KeyEvent_t Key_GetMatrixEvent(uint8_t matrix_id, uint8_t row, uint8_t col)
{
#if USE_MATRIX_KEY
    if (matrix_id < MAX_MATRIX_KEYS && 
        row < matrix_keys[matrix_id].rows && 
        col < matrix_keys[matrix_id].cols) {
        // TODO: Implement matrix key event retrieval with configuration support
        // This would require event tracking arrays for each matrix key
        return KEY_EVENT_NONE; // Placeholder
    }
#endif
    return KEY_EVENT_NONE; // Return no event for invalid parameters
}

/**
 * @brief Update key driver configuration at runtime
 * @param config Pointer to configuration structure with new values
 */
void Key_UpdateConfig(KeyConfig_t *config)
{
    if (config != NULL) {
        key_config = *config;
    }
}

void Key_Test(void)
{
    // Initialize required peripherals
    UART_Init();
    Key_Init();

    UART_Debug_Printf("Key driver initialized. Starting test loop...\r\n");
    
    uint32_t loop_count = 0;
    KeyState_t last_key_state = KEY_STATE_IDLE;

    while (1)
    {
        // Scan keys
        Key_Scan();
        
        // Get current key state and event
        KeyState_t current_key_state = Key_GetSingleState(0);
        KeyEvent_t current_key_event = Key_GetSingleEvent(0);
        GPIO_PinState raw = HAL_GPIO_ReadPin(single_keys[0].port, single_keys[0].pin);
        
        // Print when key state changes or event occurs OR every 50 loops (heartbeat)
        if (current_key_state != last_key_state || 
            current_key_event != KEY_EVENT_NONE || 
            loop_count % 50 == 0) {
            
            // Print loop counter and raw state
            UART_Debug_Printf("[%lu] Raw: %d State: %d", loop_count, raw, current_key_state);
            
            // Print event if any
            if (current_key_event != KEY_EVENT_NONE) {
                UART_Debug_Printf(" Event: %d", current_key_event);
            }
            
            UART_Debug_Printf("\r\n");
            last_key_state = current_key_state;
        }
        
        loop_count++;
        HAL_Delay(20);  // 50Hz scan rate
    }
}

void single_key_test(void)
{
    // Initialize required modules
    UART_Init();
    Key_Init();

    UART_Debug_Printf("Single Key Test Started\r\n");
    UART_Debug_Printf("Press PA7 to test...\r\n");

    uint8_t last_key_state = 0;
    uint32_t count = 0;

    while (1)
    {
        // Scan all keys
        Key_Scan();

        // Get key state (0 = released, 1 = pressed)
        uint8_t current_key_state = Key_GetSingleState(0);

        // Print when state changes
        if (current_key_state != last_key_state)
        {
            if (current_key_state)
            {
                UART_Debug_Printf("[%lu] Key Pressed!\r\n", count);
            }
            else
            {
                UART_Debug_Printf("[%lu] Key Released!\r\n", count);
            }
            last_key_state = current_key_state;
        }

        count++;
        HAL_Delay(20);  // 20ms scan interval
    }
}