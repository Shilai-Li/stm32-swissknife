/**
 * @file tinyframe_port.c
 * @brief TinyFrame port layer implementation for STM32 HAL UART
 * 
 * This file implements the platform-specific functions required by TinyFrame
 * to work with the STM32 UART driver.
 */

#include "tinyframe_port.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "stm32f1xx_hal.h" // Include HAL for HAL_GetTick

/* ============================================================================
 * Static TinyFrame Instance
 * ========================================================================= */

static TinyFrame tf_instance;

/* ============================================================================
 * TinyFrame Callbacks (Required by Library)
 * ========================================================================= */

/**
 * @brief TinyFrame error reporter
 * @param fmt Format string
 * @param ... Arguments
 */
void TF_Error(const char *fmt, ...)
{
    // Implementation depends on your logging system
    // For now, we print to UART debug channel if available, or do nothing
    UART_Debug_Printf("[TF Error] ");
    
    va_list args;
    va_start(args, fmt);
    char buffer[128];
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    
    UART_Debug_Printf("%s\r\n", buffer);
}


/**
 * @brief TinyFrame write callback - sends data via UART
 * @param tf TinyFrame instance
 * @param buff Data buffer to send
 * @param len Length of data
 * @note This function is called by TF_Send() to transmit frames
 */
void TF_WriteImpl(TinyFrame *tf, const uint8_t *buff, uint32_t len)
{
    (void)tf; // Unused parameter
    UART_Send(TINYFRAME_UART_CHANNEL, buff, (uint16_t)len);
}

/**
 * @brief TinyFrame millisecond timestamp callback (optional)
 * @return Current millisecond timestamp
 * @note Used for timeout handling if enabled
 */
TF_TICKS TF_GetTimestamp(void)
{
    return (TF_TICKS)HAL_GetTick();
}

/* ============================================================================
 * Port Layer Implementation
 * ========================================================================= */

/**
 * @brief Initialize TinyFrame with default configuration
 * @return Pointer to TinyFrame instance
 */
TinyFrame* TinyFrame_Init(void)
{
    // Initialize the TinyFrame instance statically
    TF_InitStatic(&tf_instance, TF_MASTER); // Use TF_SLAVE if this is a slave device
    
    // Set recommended timeouts (optional, adjust as needed)
    // tf_instance.peer_bit = TF_PEER;  // Enable if using peer-to-peer mode
    
    return &tf_instance;
}

/**
 * @brief Process incoming UART data and feed to TinyFrame
 * @param tf Pointer to TinyFrame instance
 * @note Call this regularly from your main loop
 */
void TinyFrame_Process(TinyFrame *tf)
{
    uint8_t byte;
    
    // Read all available bytes from UART and feed to TinyFrame
    while (UART_Read(TINYFRAME_UART_CHANNEL, &byte)) {
        TF_Accept(tf, &byte, 1);
    }
}
