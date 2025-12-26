#include "uart.h"

#include <string.h>
#include <stdarg.h>
#include <stdio.h>

// RxDMABuf is now dynamic, stored in uart_rbuf[ch].dma_buf
static volatile uint16_t RxDMAPos[UART_CHANNEL_MAX];

// UART_RingBuf and UART_TxRingBuf are defined in uart.h

static UART_RingBuf uart_rbuf[UART_CHANNEL_MAX];
static UART_TxRingBuf uart_tbuf[UART_CHANNEL_MAX];

static UART_HandleTypeDef* UART_Handles[UART_CHANNEL_MAX] = {NULL};
static UART_RxCallback RxCallbacks[UART_CHANNEL_MAX] = {NULL};

/* ============================================================================
 * Internal Function Prototypes
 * ========================================================================= */
static UART_HandleTypeDef* UART_GetHandle(UART_Channel ch);
static int UART_HandleToChannel(UART_HandleTypeDef *huart);
static void UART_ProcessDMA(UART_Channel ch);
static void UART_TxKick(UART_Channel ch);
static bool UART_RingBuf_Pop(UART_Channel ch, uint8_t *out);

/* ============================================================================
 * Internal Function Implementations
 * ========================================================================= */
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

static void UART_ProcessDMA(UART_Channel ch) {
    UART_HandleTypeDef *huart = UART_GetHandle(ch);
    if (!huart || !huart->hdmarx) return;

    UART_RingBuf *rb = &uart_rbuf[ch];
    if (!rb->buf || !rb->dma_buf) return; // Safety check

    uint16_t dma_size = rb->dma_size;
    uint16_t ring_size = rb->size;
    if (dma_size == 0 || ring_size == 0) return;

    uint8_t *dma_buf = rb->dma_buf;
    
    // Calculate current DMA position
    uint16_t dma_curr_pos = dma_size - __HAL_DMA_GET_COUNTER(huart->hdmarx);
    if (dma_curr_pos >= dma_size) dma_curr_pos = 0;

    uint16_t last_pos = RxDMAPos[ch];

    if (dma_curr_pos != last_pos) {
        uint32_t primask = __get_PRIMASK();
        __disable_irq();
        
        uint16_t head = rb->head;
        
        if (dma_curr_pos > last_pos) {
            for (uint16_t i = last_pos; i < dma_curr_pos; i++) {
                uint16_t next = (head + 1) % ring_size;
                if (next != rb->tail) {
                    rb->buf[head] = dma_buf[i];
                    head = next;
                } else {
                    rb->overrun_cnt++;
                }
            }
        } else {
            // Wrapped around end of DMA buffer
            for (uint16_t i = last_pos; i < dma_size; i++) {
                uint16_t next = (head + 1) % ring_size;
                if (next != rb->tail) {
                    rb->buf[head] = dma_buf[i];
                    head = next;
                } else {
                    rb->overrun_cnt++;
                }
            }
            for (uint16_t i = 0; i < dma_curr_pos; i++) {
                uint16_t next = (head + 1) % ring_size;
                if (next != rb->tail) {
                    rb->buf[head] = dma_buf[i];
                    head = next;
                } else {
                    rb->overrun_cnt++;
                }
            }
        }
        
        rb->head = head;
        RxDMAPos[ch] = dma_curr_pos;
        
        __set_PRIMASK(primask);

        if (RxCallbacks[ch]) {
            RxCallbacks[ch](ch);
        }
    }
}

static void UART_TxKick(UART_Channel ch)
{
    UART_HandleTypeDef *huart = UART_GetHandle(ch);
    if (huart == NULL) return;

    UART_TxRingBuf *tb = &uart_tbuf[ch];
    if (!tb->buf || tb->size == 0) return;

    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    if (tb->busy) {
        __set_PRIMASK(primask);
        return;
    }
    if (tb->head == tb->tail) {
        __set_PRIMASK(primask);
        return;
    }

    uint16_t len = 0;
    if (tb->head > tb->tail) {
        len = tb->head - tb->tail;
    } else {
        len = tb->size - tb->tail;
    }
    
    if (len == 0) {
        __set_PRIMASK(primask);
        return;
    }

    tb->busy = 1;
    tb->inflight_len = len;

    __set_PRIMASK(primask);

    HAL_StatusTypeDef status = HAL_UART_Transmit_DMA(huart, &tb->buf[tb->tail], len);
    if (status != HAL_OK) {
        tb->busy = 0;
        tb->inflight_len = 0;
        uart_rbuf[ch].error_cnt++;
    }
}

static bool UART_RingBuf_Pop(UART_Channel ch, uint8_t *out)
{
    UART_RingBuf *rb = &uart_rbuf[ch];
    if (!rb->buf || rb->size == 0) return false;

    if (rb->head == rb->tail) return false;
    *out = rb->buf[rb->tail];
    rb->tail = (rb->tail + 1) % rb->size;
    return true;
}

/* ============================================================================
 * Public API Implementation
 * ========================================================================= */

void UART_Register(UART_Channel channel, UART_HandleTypeDef *huart, 
                   uint8_t *rx_dma_buf, uint16_t rx_dma_size,
                   uint8_t *rx_ring_buf, uint16_t rx_ring_size,
                   uint8_t *tx_ring_buf, uint16_t tx_ring_size)
{
    if (channel >= UART_CHANNEL_MAX || huart == NULL) return;
    
    UART_Handles[channel] = huart;

    // Config TX
    uart_tbuf[channel].buf = tx_ring_buf;
    uart_tbuf[channel].size = tx_ring_size;
    uart_tbuf[channel].head = 0;
    uart_tbuf[channel].tail = 0;
    uart_tbuf[channel].busy = 0;
    uart_tbuf[channel].inflight_len = 0;
    
    // Config RX
    uart_rbuf[channel].buf = rx_ring_buf;
    uart_rbuf[channel].size = rx_ring_size;
    uart_rbuf[channel].dma_buf = rx_dma_buf;
    uart_rbuf[channel].dma_size = rx_dma_size;
    uart_rbuf[channel].head = 0;
    uart_rbuf[channel].tail = 0;
    
    // Reset Counters
    uart_rbuf[channel].overrun_cnt = 0;
    uart_rbuf[channel].tx_dropped = 0;
    uart_rbuf[channel].error_cnt = 0;
    uart_rbuf[channel].pe_error_cnt = 0;
    uart_rbuf[channel].ne_error_cnt = 0;
    uart_rbuf[channel].fe_error_cnt = 0;
    uart_rbuf[channel].ore_error_cnt = 0;
    uart_rbuf[channel].dma_error_cnt = 0;
    uart_rbuf[channel].error_flag = 0;
    
    RxDMAPos[channel] = 0;

    if (rx_dma_buf && rx_dma_size > 0) {
        HAL_UART_Receive_DMA(huart, rx_dma_buf, rx_dma_size);
    }
}

void UART_SetRxCallback(UART_Channel channel, UART_RxCallback cb)
{
    if (channel >= UART_CHANNEL_MAX) return;
    RxCallbacks[channel] = cb;
}



bool UART_Send(UART_Channel channel, const uint8_t *data, uint16_t len)
{
    UART_HandleTypeDef *huart = UART_GetHandle(channel);
    if (huart == NULL || data == NULL || len == 0) return false;

    UART_TxRingBuf *tb = &uart_tbuf[channel];
    if (!tb->buf || tb->size == 0) return false;

    bool success = true;

    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    for (uint16_t i = 0; i < len; i++) {
        uint16_t next = (tb->head + 1) % tb->size;
        if (next == tb->tail) {
            uart_rbuf[channel].tx_dropped += (len - i);
            success = false;
            break;
        }
        tb->buf[tb->head] = data[i];
        tb->head = next;
    }

    __set_PRIMASK(primask);

    UART_TxKick(channel);
    return success;
}

void UART_SendString(UART_Channel channel, const char *str)
{
    if (!str) return;
    UART_Send(channel, (const uint8_t *)str, (uint16_t)strlen(str));
}

uint16_t UART_Available(UART_Channel ch)
{
    UART_ProcessDMA(ch);
    
    UART_RingBuf *rb = &uart_rbuf[ch];
    if (!rb->buf || rb->size == 0) return 0;

    if (rb->head >= rb->tail) return rb->head - rb->tail;
    return rb->size - (rb->tail - rb->head);
}

bool UART_Read(UART_Channel ch, uint8_t *out)
{
    UART_ProcessDMA(ch);
    return UART_RingBuf_Pop(ch, out);
}

bool UART_Receive(UART_Channel ch, uint8_t *out, uint32_t timeout_ms)
{
    uint32_t start = HAL_GetTick();
    while ((HAL_GetTick() - start) < timeout_ms) {
        if (UART_Read(ch, out)) return true;
    }
    return false;
}

void UART_Flush(UART_Channel ch)
{
    UART_HandleTypeDef *huart = UART_GetHandle(ch);
    if (!huart) return;

    UART_RingBuf *rb = &uart_rbuf[ch];
    // Reset RingBuffer Pointers
    rb->head = 0;
    rb->tail = 0;
    
    // Sync DMA Position
    if (huart->hdmarx && rb->dma_size > 0) {
        RxDMAPos[ch] = rb->dma_size - __HAL_DMA_GET_COUNTER(huart->hdmarx);
        if (RxDMAPos[ch] >= rb->dma_size) RxDMAPos[ch] = 0;
    }
}

bool UART_IsTxBusy(UART_Channel ch)
{
    if (ch >= UART_CHANNEL_MAX) return false;
    
    // Check if DMA is physically busy
    if (uart_tbuf[ch].busy) return true;
    
    // Check if RingBuffer has pending data
    if (uart_tbuf[ch].head != uart_tbuf[ch].tail) return true;
    
    return false;
}

uint32_t UART_GetRxOverrunCount(UART_Channel ch)
{
    if (ch >= UART_CHANNEL_MAX) return 0;
    return uart_rbuf[ch].overrun_cnt;
}

void UART_Poll(void)
{
    for (int i = 0; i < UART_CHANNEL_MAX; i++) {
        UART_HandleTypeDef *huart = UART_GetHandle((UART_Channel)i);
        if (huart != NULL) {
            if (uart_tbuf[i].busy && huart->gState == HAL_UART_STATE_READY) {
                uart_tbuf[i].busy = 0;
                uart_tbuf[i].inflight_len = 0;
            }
            
            // Re-start RX DMA if it stopped (e.g. after error)
            if (huart->RxState != HAL_UART_STATE_BUSY_RX) {
                 if (uart_rbuf[i].dma_buf && uart_rbuf[i].dma_size > 0) {
                    RxDMAPos[i] = 0;
                    HAL_UART_Receive_DMA(huart, uart_rbuf[i].dma_buf, uart_rbuf[i].dma_size);
                 }
            }
        }
        
        UART_ProcessDMA((UART_Channel)i);
        
        if (uart_tbuf[i].head != uart_tbuf[i].tail) {
            UART_TxKick((UART_Channel)i);
        }
    }
}

uint32_t UART_GetTxDropCount(UART_Channel ch)
{
    if (ch >= UART_CHANNEL_MAX) return 0;
    return uart_rbuf[ch].tx_dropped;
}

uint32_t UART_GetErrorCount(UART_Channel ch)
{
    if (ch >= UART_CHANNEL_MAX) return 0;
    return uart_rbuf[ch].error_cnt;
}

uint32_t UART_GetPEErrorCount(UART_Channel ch)
{
    if (ch >= UART_CHANNEL_MAX) return 0;
    return uart_rbuf[ch].pe_error_cnt;
}

uint32_t UART_GetNEErrorCount(UART_Channel ch)
{
    if (ch >= UART_CHANNEL_MAX) return 0;
    return uart_rbuf[ch].ne_error_cnt;
}

uint32_t UART_GetFEErrorCount(UART_Channel ch)
{
    if (ch >= UART_CHANNEL_MAX) return 0;
    return uart_rbuf[ch].fe_error_cnt;
}

uint32_t UART_GetOREErrorCount(UART_Channel ch)
{
    if (ch >= UART_CHANNEL_MAX) return 0;
    return uart_rbuf[ch].ore_error_cnt;
}

uint32_t UART_GetDMAErrorCount(UART_Channel ch)
{
    if (ch >= UART_CHANNEL_MAX) return 0;
    return uart_rbuf[ch].dma_error_cnt;
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    int ch = UART_HandleToChannel(huart);
    if (ch < 0 || ch >= UART_CHANNEL_MAX) return;

    UART_TxRingBuf *tb = &uart_tbuf[ch];
    if (tb->size == 0) return;

    tb->tail = (tb->tail + tb->inflight_len) % tb->size;
    tb->inflight_len = 0;
    tb->busy = 0;

    UART_TxKick((UART_Channel)ch);
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    int ch = UART_HandleToChannel(huart);
    if (ch < 0) return;
    
    UART_RingBuf *rb = &uart_rbuf[ch];
    
    rb->error_cnt++;

    uint32_t error_flags = huart->ErrorCode;
    if (error_flags & HAL_UART_ERROR_PE) rb->pe_error_cnt++;
    if (error_flags & HAL_UART_ERROR_NE) rb->ne_error_cnt++;
    if (error_flags & HAL_UART_ERROR_FE) rb->fe_error_cnt++;
    if (error_flags & HAL_UART_ERROR_ORE) rb->ore_error_cnt++;
    if (error_flags & HAL_UART_ERROR_DMA) rb->dma_error_cnt++;

    __HAL_UART_CLEAR_OREFLAG(huart);
    __HAL_UART_CLEAR_NEFLAG(huart);
    __HAL_UART_CLEAR_FEFLAG(huart);
    __HAL_UART_CLEAR_PEFLAG(huart);
    
    if (huart->RxState != HAL_UART_STATE_BUSY_RX) {
         if (rb->dma_buf && rb->dma_size > 0) {
            RxDMAPos[ch] = 0;
            HAL_UART_Receive_DMA(huart, rb->dma_buf, rb->dma_size);
         }
    }
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
