#include "uart.h"
#include "usart.h"

#include <string.h>
#include <stdarg.h>
#include <stdio.h>

static UART_RingBuf uart_rbuf[UART_CHANNEL_MAX];

typedef struct {
    uint8_t buf[UART_TX_BUF_SIZE];
    volatile uint16_t head;
    volatile uint16_t tail;
    volatile uint8_t busy;
    uint16_t inflight_len;
} UART_TxRingBuf;

static UART_TxRingBuf uart_tbuf[UART_CHANNEL_MAX];

// Map UART_Channel to UART_HandleTypeDef*
static UART_HandleTypeDef* UART_Handles[UART_CHANNEL_MAX] = {
#if defined USE_UART1
    &huart1,
#endif
#if defined USE_UART2
    &huart2,
#endif
#if defined USE_UART3
    &huart3,
#endif
#if defined USE_UART4
    &huart4,
#endif
#if defined USE_UART5
    &huart5,
#endif
#if defined USE_UART6
    &huart6,
#endif
#if defined USE_UART7
    &huart7,
#endif
#if defined USE_UART8
    &huart8,
#endif
};

static UART_HandleTypeDef* UART_GetHandle(UART_Channel ch)
{
    if (ch >= UART_CHANNEL_MAX) return NULL;
    return UART_Handles[ch];
}

static int UART_HandleToChannel(UART_HandleTypeDef *huart)
{
    for (int i = 0; i < UART_CHANNEL_MAX; i++) {
        if (UART_Handles[i] == huart) {
            return i;
        }
    }
    return -1;
}

static void UART_TxKick(UART_Channel ch)
{
    UART_HandleTypeDef *huart = UART_GetHandle(ch);
    if (huart == NULL) return;

    UART_TxRingBuf *tb = &uart_tbuf[ch];

    __disable_irq();
    if (tb->busy) {
        __enable_irq();
        return;
    }
    if (tb->head == tb->tail) {
        __enable_irq();
        return;
    }

    uint16_t len = 0;
    if (tb->head > tb->tail) {
        len = tb->head - tb->tail;
    } else {
        len = UART_TX_BUF_SIZE - tb->tail;
    }
    if (len == 0) {
        __enable_irq();
        return;
    }

    tb->busy = 1;
    tb->inflight_len = len;
    __enable_irq();

    if (HAL_UART_Transmit_IT(huart, &tb->buf[tb->tail], len) != HAL_OK) {
        __disable_irq();
        tb->busy = 0;
        tb->inflight_len = 0;
        __enable_irq();
    }
}

/* ============================================================================
 * Ring Buffer Operations
 * ========================================================================= */
static void UART_RingBuf_PushFromIRQ(UART_Channel ch, uint8_t byte)
{
    UART_RingBuf *rb = &uart_rbuf[ch];
    uint16_t next = (rb->head + 1) & (UART_RX_BUF_SIZE - 1);
    if (next == rb->tail) return;  // full
    rb->buf[rb->head] = byte;
    rb->head = next;
}

static bool UART_RingBuf_Pop(UART_Channel ch, uint8_t *out)
{
    UART_RingBuf *rb = &uart_rbuf[ch];
    if (rb->head == rb->tail) return false;
    *out = rb->buf[rb->tail];
    rb->tail = (rb->tail + 1) & (UART_RX_BUF_SIZE - 1);
    return true;
}

uint16_t UART_Available(UART_Channel ch)
{
    UART_RingBuf *rb = &uart_rbuf[ch];
    if (rb->head >= rb->tail) return rb->head - rb->tail;
    return UART_RX_BUF_SIZE - (rb->tail - rb->head);
}

/* ============================================================================
 * Initialization
 * ========================================================================= */
void UART_Init(void)
{
    for (int i = 0; i < UART_CHANNEL_MAX; i++) {
        UART_HandleTypeDef *huart = UART_Handles[i];
        if (huart != NULL){
            uart_tbuf[i].head = 0;
            uart_tbuf[i].tail = 0;
            uart_tbuf[i].busy = 0;
            uart_tbuf[i].inflight_len = 0;
            HAL_UART_Receive_IT(huart, &uart_rbuf[i].rx_byte, 1);
        }
    }
}

/* ============================================================================
 * Send/Receive API
 * ========================================================================= */
void UART_Send(UART_Channel channel, const uint8_t *data, uint16_t len)
{
    UART_HandleTypeDef *huart = UART_GetHandle(channel);
    if (huart == NULL || data == NULL || len == 0) return;

    UART_TxRingBuf *tb = &uart_tbuf[channel];

    __disable_irq();
    for (uint16_t i = 0; i < len; i++) {
        uint16_t next = (uint16_t)(tb->head + 1);
        if (next >= UART_TX_BUF_SIZE) next = 0;
        if (next == tb->tail) {
            break;
        }
        tb->buf[tb->head] = data[i];
        tb->head = next;
    }
    __enable_irq();

    UART_TxKick(channel);
}

void UART_SendString(UART_Channel channel, const char *str)
{
    if (!str) return;
    UART_Send(channel, (const uint8_t *)str, (uint16_t)strlen(str));
}

bool UART_Read(UART_Channel ch, uint8_t *out)
{
    __disable_irq();
    bool ok = UART_RingBuf_Pop(ch, out);
    __enable_irq();
    return ok;
}

bool UART_Receive(UART_Channel ch, uint8_t *out, uint32_t timeout_ms)
{
    uint32_t start = HAL_GetTick();
    while ((HAL_GetTick() - start) < timeout_ms) {
        if (UART_Read(ch, out)) return true;
    }
    return false;
}

/* ============================================================================
 * HAL Callback Wrappers
 * ========================================================================= */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    int ch = UART_HandleToChannel(huart);
    if (ch < 0 || ch >= UART_CHANNEL_MAX) return;

    UART_RingBuf_PushFromIRQ((UART_Channel)ch, uart_rbuf[ch].rx_byte);
    HAL_UART_Receive_IT(huart, &uart_rbuf[ch].rx_byte, 1);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    int ch = UART_HandleToChannel(huart);
    if (ch < 0 || ch >= UART_CHANNEL_MAX) return;

    UART_TxRingBuf *tb = &uart_tbuf[ch];

    tb->tail = (uint16_t)(tb->tail + tb->inflight_len);
    if (tb->tail >= UART_TX_BUF_SIZE) {
        tb->tail = (uint16_t)(tb->tail - UART_TX_BUF_SIZE);
    }
    tb->inflight_len = 0;
    tb->busy = 0;

    UART_TxKick((UART_Channel)ch);
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    int ch = UART_HandleToChannel(huart);
    if (ch < 0) return;
    HAL_UART_Receive_IT(huart, &uart_rbuf[ch].rx_byte, 1);
}

void UART_Debug_Printf(const char *fmt, ...)
{
    char buffer[256];
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    if (len > 0)
        UART_Send(UART_DEBUG_CHANNEL, (uint8_t *)buffer, len);
}