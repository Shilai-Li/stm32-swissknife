#ifndef __UART_DRIVER_H__
#define __UART_DRIVER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#if defined(STM32F0)
    #include "stm32f0xx_hal.h"
#elif defined(STM32F1) || defined(STM32F103xB) || defined(STM32F103xE)
    #include "stm32f1xx_hal.h"
#elif defined(STM32F2)
    #include "stm32f2xx_hal.h"
#elif defined(STM32F3)
    #include "stm32f3xx_hal.h"
#elif defined(STM32F4) || defined(STM32F405xx) || defined(STM32F407xx) || defined(STM32F429xx) || defined(STM32F446xx)
    #include "stm32f4xx_hal.h"
#elif defined(STM32F7)
    #include "stm32f7xx_hal.h"
#elif defined(STM32H7)
    #include "stm32h7xx_hal.h"
#elif defined(STM32L0)
    #include "stm32l0xx_hal.h"
#elif defined(STM32L1)
    #include "stm32l1xx_hal.h"
#elif defined(STM32L4)
    #include "stm32l4xx_hal.h"
#elif defined(STM32G0)
    #include "stm32g0xx_hal.h"
#elif defined(STM32G4)
    #include "stm32g4xx_hal.h"
#else
    #include "main.h"
#endif

// Logic channel ID, managed by user application
typedef uint8_t UART_Channel;

#define UART_CHANNEL_MAX 3 // Reduced to save RAM (was 8)

/* ============================================================================
 * UART Configuration
 * ========================================================================= */
/* Buffer sizes are now dynamic, configured in UART_Register */

// UART_DEBUG_CHANNEL should be defined by user if they use UART_Debug_Printf, 
// usually as a macro in main.h or here if hardcoded. Defaulting to 0.
#ifndef UART_DEBUG_CHANNEL
#define UART_DEBUG_CHANNEL 0
#endif


typedef struct {
    uint8_t *buf;
    uint16_t size;
    uint8_t *dma_buf;      // Pointer to DMA buffer
    uint16_t dma_size;     // Size of DMA buffer
    volatile uint16_t head;
    volatile uint16_t tail;
    volatile uint32_t overrun_cnt;  // RX Software Buffer Overflow
    volatile uint32_t tx_dropped;   // TX Buffer Overflow (Send failed)
    volatile uint32_t error_cnt;    // Total Hardware Errors
    volatile uint32_t pe_error_cnt; // Parity Errors
    volatile uint32_t ne_error_cnt; // Noise Errors
    volatile uint32_t fe_error_cnt; // Frame Errors
    volatile uint32_t ore_error_cnt; // Overrun Errors
    volatile uint32_t dma_error_cnt; // DMA Transfer Errors
    volatile uint8_t error_flag;    // Error flag for recovery in main loop
} UART_RingBuf;

typedef struct {
    uint8_t *buf;
    uint16_t size;
    volatile uint16_t head;
    volatile uint16_t tail;
    volatile uint8_t busy;
    uint16_t inflight_len;
} UART_TxRingBuf;

/* ============================================================================
 * UART Public API
 * ========================================================================= */

// Callback type for RX Data Available
typedef void (*UART_RxCallback)(UART_Channel channel);

void UART_Register(UART_Channel channel, UART_HandleTypeDef *huart, 
                   uint8_t *rx_dma_buf, uint16_t rx_dma_size,
                   uint8_t *rx_ring_buf, uint16_t rx_ring_size,
                   uint8_t *tx_ring_buf, uint16_t tx_ring_size);
void UART_SetRxCallback(UART_Channel channel, UART_RxCallback cb);
bool UART_Send(UART_Channel channel, const uint8_t *data, uint16_t len);
void UART_SendString(UART_Channel channel, const char *str);
uint16_t UART_Available(UART_Channel channel);
bool UART_Read(UART_Channel channel, uint8_t *out);
bool UART_Receive(UART_Channel channel, uint8_t *out, uint32_t timeout_ms);
void UART_Flush(UART_Channel channel);
bool UART_IsTxBusy(UART_Channel channel);
uint32_t UART_GetRxOverrunCount(UART_Channel channel);
void UART_Poll(void);
uint32_t UART_GetTxDropCount(UART_Channel channel);
uint32_t UART_GetErrorCount(UART_Channel channel);
uint32_t UART_GetPEErrorCount(UART_Channel channel);
uint32_t UART_GetNEErrorCount(UART_Channel channel);
uint32_t UART_GetFEErrorCount(UART_Channel channel);
uint32_t UART_GetOREErrorCount(UART_Channel channel);
uint32_t UART_GetDMAErrorCount(UART_Channel channel);
void UART_Debug_Printf(const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif
