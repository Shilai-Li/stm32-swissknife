#include "uart.h"
#include "usart.h"

#include <string.h>
#include <stdarg.h>
#include <stdio.h>

/* ============================================================================
 * Internal Definitions & Static Data
 * ========================================================================= */

// DMA Circular Mode 接收缓冲区
static uint8_t RxDMABuf[UART_CHANNEL_MAX][UART_RX_BUF_SIZE];
static volatile uint16_t RxDMAPos[UART_CHANNEL_MAX];

// TX Ring Buffer (Internal)
typedef struct {
    uint8_t buf[UART_TX_BUF_SIZE];
    volatile uint16_t head;
    volatile uint16_t tail;
    volatile uint8_t busy;
    uint16_t inflight_len;
} UART_TxRingBuf;

static UART_RingBuf uart_rbuf[UART_CHANNEL_MAX];
static UART_TxRingBuf uart_tbuf[UART_CHANNEL_MAX];

/* ============================================================================
 * Hardware Mapping Helpers
 * ========================================================================= */

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

/* ============================================================================
 * Internal Helpers - RX DMA Processing
 * ========================================================================= */

// 从 DMA 缓冲区复制数据到软件环形缓冲区
static void UART_ProcessDMA(UART_Channel ch) {
    UART_HandleTypeDef *huart = UART_GetHandle(ch);
    if (!huart || !huart->hdmarx) return;

    UART_RingBuf *rb = &uart_rbuf[ch];
    uint8_t *dma_buf = RxDMABuf[ch];
    
    // 计算当前 DMA 位置
    uint16_t dma_curr_pos = UART_RX_BUF_SIZE - __HAL_DMA_GET_COUNTER(huart->hdmarx);
    if (dma_curr_pos >= UART_RX_BUF_SIZE) dma_curr_pos = 0;

    uint16_t last_pos = RxDMAPos[ch];

    if (dma_curr_pos != last_pos) {
        uint16_t head = rb->head;
        
        if (dma_curr_pos > last_pos) {
            // 线性复制
            for (uint16_t i = last_pos; i < dma_curr_pos; i++) {
                uint16_t next = (head + 1) % UART_RX_BUF_SIZE;
                if (next != rb->tail) {
                    rb->buf[head] = dma_buf[i];
                    head = next;
                } else {
                    rb->overrun_cnt++;
                }
            }
        } else {
            // 环绕复制
            for (uint16_t i = last_pos; i < UART_RX_BUF_SIZE; i++) {
                uint16_t next = (head + 1) % UART_RX_BUF_SIZE;
                if (next != rb->tail) {
                    rb->buf[head] = dma_buf[i];
                    head = next;
                } else {
                    rb->overrun_cnt++;
                }
            }
            for (uint16_t i = 0; i < dma_curr_pos; i++) {
                uint16_t next = (head + 1) % UART_RX_BUF_SIZE;
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
    }
}

/* ============================================================================
 * Internal Helpers - TX
 * ========================================================================= */

static void UART_TxKick(UART_Channel ch)
{
    UART_HandleTypeDef *huart = UART_GetHandle(ch);
    if (huart == NULL) return;

    UART_TxRingBuf *tb = &uart_tbuf[ch];

    if (tb->busy) return;
    if (tb->head == tb->tail) return;

    uint16_t len = 0;
    if (tb->head > tb->tail) {
        len = tb->head - tb->tail;
    } else {
        len = UART_TX_BUF_SIZE - tb->tail;
    }
    if (len == 0) return;

    tb->busy = 1;
    tb->inflight_len = len;

    // 使用 DMA 发送（Normal 模式）
    HAL_StatusTypeDef status = HAL_UART_Transmit_DMA(huart, &tb->buf[tb->tail], len);
    if (status != HAL_OK) {
        tb->busy = 0;
        tb->inflight_len = 0;
        uart_rbuf[ch].error_cnt++;
    }
}

/* ============================================================================
 * Ring Buffer Operations
 * ========================================================================= */

static bool UART_RingBuf_Pop(UART_Channel ch, uint8_t *out)
{
    UART_RingBuf *rb = &uart_rbuf[ch];
    if (rb->head == rb->tail) return false;
    *out = rb->buf[rb->tail];
    rb->tail = (rb->tail + 1) % UART_RX_BUF_SIZE;
    return true;
}

uint16_t UART_Available(UART_Channel ch)
{
    UART_ProcessDMA(ch);
    
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
        if (huart != NULL) {
            // 初始化 TX 缓冲区
            uart_tbuf[i].head = 0;
            uart_tbuf[i].tail = 0;
            uart_tbuf[i].busy = 0;
            uart_tbuf[i].inflight_len = 0;
            
            // 初始化 RX 缓冲区
            uart_rbuf[i].head = 0;
            uart_rbuf[i].tail = 0;
            uart_rbuf[i].overrun_cnt = 0;
            uart_rbuf[i].tx_dropped = 0;
            uart_rbuf[i].error_cnt = 0;
            uart_rbuf[i].pe_error_cnt = 0;
            uart_rbuf[i].ne_error_cnt = 0;
            uart_rbuf[i].fe_error_cnt = 0;
            uart_rbuf[i].ore_error_cnt = 0;
            uart_rbuf[i].dma_error_cnt = 0;
            uart_rbuf[i].error_flag = 0;
            
            RxDMAPos[i] = 0;

            // 启动 DMA 接收（Circular 模式）
            HAL_UART_Receive_DMA(huart, RxDMABuf[i], UART_RX_BUF_SIZE);
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

    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    for (uint16_t i = 0; i < len; i++) {
        uint16_t next = (tb->head + 1) % UART_TX_BUF_SIZE;
        if (next == tb->tail) {
            uart_rbuf[channel].tx_dropped += (len - i);
            break;
        }
        tb->buf[tb->head] = data[i];
        tb->head = next;
    }

    __set_PRIMASK(primask);

    UART_TxKick(channel);
}

void UART_SendString(UART_Channel channel, const char *str)
{
    if (!str) return;
    UART_Send(channel, (const uint8_t *)str, (uint16_t)strlen(str));
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
            // 检查 TX 是否卡住
            if (uart_tbuf[i].busy && huart->gState == HAL_UART_STATE_READY) {
                uart_tbuf[i].busy = 0;
                uart_tbuf[i].inflight_len = 0;
            }
            
            // 检查 RX DMA 是否停止
            if (huart->RxState != HAL_UART_STATE_BUSY_RX) {
                RxDMAPos[i] = 0;
                HAL_UART_Receive_DMA(huart, RxDMABuf[i], UART_RX_BUF_SIZE);
            }
        }
        
        // 处理 RX 数据
        UART_ProcessDMA((UART_Channel)i);
        
        // 重试 TX
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

/* ============================================================================
 * HAL Callback Wrappers
 * ========================================================================= */

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    int ch = UART_HandleToChannel(huart);
    if (ch < 0 || ch >= UART_CHANNEL_MAX) return;

    UART_TxRingBuf *tb = &uart_tbuf[ch];

    tb->tail = (tb->tail + tb->inflight_len) % UART_TX_BUF_SIZE;
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

    // 清除错误标志
    __HAL_UART_CLEAR_OREFLAG(huart);
    __HAL_UART_CLEAR_NEFLAG(huart);
    __HAL_UART_CLEAR_FEFLAG(huart);
    __HAL_UART_CLEAR_PEFLAG(huart);
    
    // 如果 RX DMA 停止了，重启它
    if (huart->RxState != HAL_UART_STATE_BUSY_RX) {
        RxDMAPos[ch] = 0;
        HAL_UART_Receive_DMA(huart, RxDMABuf[ch], UART_RX_BUF_SIZE);
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