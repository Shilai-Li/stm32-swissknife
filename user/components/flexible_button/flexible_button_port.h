#ifndef __FLEXIBLE_BUTTON_PORT_H__
#define __FLEXIBLE_BUTTON_PORT_H__

#include <stdint.h>
#include "stm32f1xx_hal.h" 

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the FlexibleButton library and hardware
 * 
 */
void FlexibleButton_Init(void);

/**
 * @brief Main loop process for FlexibleButton
 * @note This function should be called periodically (e.g., every 5ms-20ms) inside the main loop or a timer interrupt
 */
void FlexibleButton_Scan(void);

/**
 * @brief Example callback function for button events
 * 
 * @param btn Pointer to the flexible button instance
 */
void btn_event_callback(void *btn);

#ifdef __cplusplus
}
#endif

#endif // __FLEXIBLE_BUTTON_PORT_H__
