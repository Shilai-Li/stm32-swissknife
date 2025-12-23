/**
 * @file tinyframe_port.h
 * @brief TinyFrame port layer for STM32 HAL UART driver
 * 
 * This file provides the platform-specific configuration and
 * helper functions for using TinyFrame with the STM32 UART driver.
 */

#ifndef __TINYFRAME_PORT_H__
#define __TINYFRAME_PORT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "tinyframe.h"
#include "uart.h"

/* ============================================================================
 * Configuration
 * ========================================================================= */

/**
 * @brief UART channel to use for TinyFrame communication
 * @note Make sure this UART is enabled in uart.h (e.g., #define USE_UART2)
 */
#ifndef TINYFRAME_UART_CHANNEL
#define TINYFRAME_UART_CHANNEL  UART_CHANNEL_2
#endif

/**
 * @brief Maximum RX payload size (bytes)
 * @note Adjust based on your application needs and available RAM
 */
#ifndef TF_MAX_PAYLOAD_RX
#define TF_MAX_PAYLOAD_RX  256
#endif

/**
 * @brief Maximum TX payload size (bytes)
 */
#ifndef TF_MAX_PAYLOAD_TX
#define TF_MAX_PAYLOAD_TX  256
#endif

/**
 * @brief Enable CRC checksum for frames
 * @note 0 = No checksum, 8 = CRC8, 16 = CRC16, 32 = CRC32
 */
#ifndef TF_CKSUM_TYPE
#define TF_CKSUM_TYPE  TF_CKSUM_CRC16
#endif

/* ============================================================================
 * Port Layer API
 * ========================================================================= */

/**
 * @brief Initialize TinyFrame with port-specific configuration
 * @return Pointer to initialized TinyFrame instance
 * @note This function allocates static memory for TinyFrame
 */
TinyFrame* TinyFrame_Init(void);

/**
 * @brief Process incoming data from UART
 * @param tf Pointer to TinyFrame instance
 * @note Call this function periodically in your main loop or from a timer
 */
void TinyFrame_Process(TinyFrame *tf);

#ifdef __cplusplus
}
#endif

#endif /* __TINYFRAME_PORT_H__ */
