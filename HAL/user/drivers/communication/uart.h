#ifndef __UART_DRIVER_H__
#define __UART_DRIVER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

// #define USE_UART1
#define USE_UART2
// #define USE_UART3
// #define USE_UART4
// #define USE_UART5
// #define USE_UART6
// #define USE_UART7
// #define USE_UART8

typedef enum {
#if defined USE_UART1
    UART_CHANNEL_1,
#endif
#if defined USE_UART2
    UART_CHANNEL_2,
#endif
#if defined USE_UART3
    UART_CHANNEL_3,
#endif
#if defined USE_UART4
    UART_CHANNEL_4,
#endif
#if defined USE_UART5
    UART_CHANNEL_5,
#endif
#if defined USE_UART6
    UART_CHANNEL_6,
#endif
#if defined USE_UART7
    UART_CHANNEL_7,
#endif
#if defined USE_UART8
    UART_CHANNEL_8,
#endif
    UART_CHANNEL_MAX
} UART_Channel;

/* ============================================================================
 * UART Configuration
 * ========================================================================= */
#ifndef UART_RX_BUF_SIZE
#define UART_RX_BUF_SIZE  2048
#endif

#ifndef UART_TX_BUF_SIZE
#define UART_TX_BUF_SIZE  2048
#endif

#ifndef UART_DEBUG_CHANNEL
#define UART_DEBUG_CHANNEL UART_CHANNEL_2
#endif

typedef struct {
    uint8_t buf[UART_RX_BUF_SIZE];
    volatile uint16_t head;
    volatile uint16_t tail;
    uint8_t rx_byte;
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

/* ============================================================================
 * UART Public API
 * ========================================================================= */
void UART_Init(void);
void UART_Send(UART_Channel channel, const uint8_t *data, uint16_t len);
void UART_SendString(UART_Channel channel, const char *str);
uint16_t UART_Available(UART_Channel channel);
bool UART_Read(UART_Channel channel, uint8_t *out);
bool UART_Receive(UART_Channel channel, uint8_t *out, uint32_t timeout_ms);
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
