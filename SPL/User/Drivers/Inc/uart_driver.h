#ifndef __UART_DRIVER_H__
#define __UART_DRIVER_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef USE_STDPERIPH_DRIVER

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "stm32f10x.h"

#define SYS_TICK_FREQ_HZ  1000U
#define DEBUG_ENABLE       1

#define USE_UART1   1
#define USE_UART2   0
#define USE_UART3   0
#define USE_UART4   0
#define USE_UART5   0
#define USE_UART6   0
#define USE_UART7   0
#define USE_UART8   0

#define UART_DEFAULT_BAUD 115200

typedef enum {
#if USE_UART1
    UART_CHANNEL_1,
#endif
#if USE_UART2
    UART_CHANNEL_2,
#endif
#if USE_UART3
    UART_CHANNEL_3,
#endif
#if USE_UART4
    UART_CHANNEL_4,
#endif
#if USE_UART5
    UART_CHANNEL_5,
#endif
#if USE_UART6
    UART_CHANNEL_6,
#endif
#if USE_UART7
    UART_CHANNEL_7,
#endif
#if USE_UART8
    UART_CHANNEL_8,
#endif
    UART_CHANNEL_MAX
} UART_Channel;

/* ============================================================================
 * UART Configuration
 * ========================================================================= */
#ifndef UART_RX_BUF_SIZE
#define UART_RX_BUF_SIZE  256
#endif

#ifndef UART_DEBUG_ENABLE
#define UART_DEBUG_ENABLE 1
#endif

#ifndef UART_DEBUG_CHANNEL
#define UART_DEBUG_CHANNEL UART_CHANNEL_1
#endif

typedef struct {
    uint8_t buf[UART_RX_BUF_SIZE];
    volatile uint16_t head;
    volatile uint16_t tail;
    uint8_t rx_byte;
} UART_RingBuf;

/* ============================================================================
 * UART Public API
 * ========================================================================= */
void UART_InitAll(void);
void UART_Send(UART_Channel channel, const uint8_t *data, uint16_t len);
void UART_SendString(UART_Channel channel, const char *str);
uint16_t UART_Available(UART_Channel channel);
bool UART_Read(UART_Channel channel, uint8_t *out);
bool UART_Receive(UART_Channel channel, uint8_t *out, uint32_t timeout_ms);
void UART_Debug_Printf(const char *fmt, ...);
void UART_Test(void);
 
#endif

#ifdef __cplusplus
}
#endif

#endif /* __MODULE_UART_H__ */