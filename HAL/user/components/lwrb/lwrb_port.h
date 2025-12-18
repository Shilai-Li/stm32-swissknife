/**
 * @file lwrb_port.h
 * @brief lwrb port configuration for STM32
 * 
 * This file provides platform-specific configuration for lwrb.
 * lwrb itself is self-contained and doesn't require much platform code.
 */

#ifndef __LWRB_PORT_H__
#define __LWRB_PORT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>

/* ============================================================================
 * lwrb Configuration
 * ========================================================================= */

/**
 * @brief Disable atomic operations (we use interrupt disable instead)
 * @note Set to 0 to enable built-in atomic operations if available
 */
#ifndef LWRB_DISABLE_ATOMIC
#define LWRB_DISABLE_ATOMIC  0
#endif

/**
 * @brief Custom memcpy function (optional)
 * @note Use standard memcpy by default
 */
#ifndef LWRB_MEMCPY
#define LWRB_MEMCPY  memcpy
#endif

/**
 * @brief Custom memset function (optional)
 * @note Use standard memset by default
 */
#ifndef LWRB_MEMSET
#define LWRB_MEMSET  memset
#endif

/* ============================================================================
 * Usage Example
 * ========================================================================= */

/*
 * Basic usage:
 * 
 * #include "lwrb.h"
 * 
 * // Define buffer
 * static uint8_t buffer_data[256];
 * static lwrb_t buff;
 * 
 * // Initialize
 * lwrb_init(&buff, buffer_data, sizeof(buffer_data));
 * 
 * // Write data
 * uint8_t data[] = "Hello, lwrb!";
 * lwrb_write(&buff, data, sizeof(data));
 * 
 * // Read data
 * uint8_t out[32];
 * size_t read = lwrb_read(&buff, out, sizeof(out));
 * 
 * // Check available data
 * size_t available = lwrb_get_full(&buff);
 * size_t free_space = lwrb_get_free(&buff);
 */

#ifdef __cplusplus
}
#endif

#endif /* __LWRB_PORT_H__ */
