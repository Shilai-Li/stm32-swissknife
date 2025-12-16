#include "uart.h"
#include "usart.h"

#include <string.h>
#include <stdarg.h>
#include <stdio.h>

/* ============================================================================
 * Internal Definitions & Static Data
 * ========================================================================= */

// Note: UART_RingBuf is defined in uart.h. 
// We use a separate static buffer for DMA Circular Mode to avoid exposing it in the public header.
static uint8_t RxDMABuf[UART_CHANNEL_MAX][UART_RX_BUF_SIZE];
static volatile uint16_t RxDMAPos[UART_CHANNEL_MAX]; // Tracks the last processed position by software
static volatile uint8_t rx_pending[UART_CHANNEL_MAX]; // Flag: ISR suggests checking DMA

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

/* ============================================================================
 * Internal Helpers
 * ========================================================================= */

    // Helper to copy data from DMA buffer to RingBuffer
static void UART_ProcessDMA(UART_Channel ch) {
    UART_HandleTypeDef *huart = UART_GetHandle(ch);
    if (!huart || !huart->hdmarx) return;

    // No global critical section here. We only protect shared state updates if needed.
    // Since UART_Poll and UART_Read are typically in the same main loop context, 
    // we assume single writer to 'head' (this function).

    UART_RingBuf *rb = &uart_rbuf[ch];
    uint8_t *dma_buf = RxDMABuf[ch];
    
    // Calculate current DMA position: Size - Remaining
    uint16_t dma_curr_pos = UART_RX_BUF_SIZE - __HAL_DMA_GET_COUNTER(huart->hdmarx);
    if (dma_curr_pos >= UART_RX_BUF_SIZE) dma_curr_pos = 0; // Safety guard

    uint16_t last_pos = RxDMAPos[ch];

    if (dma_curr_pos != last_pos) {
        // We have new data
        uint16_t head = rb->head;  // Local copy of head pointer
        if (dma_curr_pos > last_pos) {
            // Linear copy
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
            // Wrap around copy
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
        
        // Single critical section to update head pointer
        uint32_t primask = __get_PRIMASK();
        __disable_irq();
        rb->head = head;
        __set_PRIMASK(primask);
        
        RxDMAPos[ch] = dma_curr_pos;
    }
}

static void UART_TxKick(UART_Channel ch)
{
    UART_HandleTypeDef *huart = UART_GetHandle(ch);
    if (huart == NULL) return;

    UART_TxRingBuf *tb = &uart_tbuf[ch];

    // Simple lock checks without disabling interrupts to avoid 'cpsid i' errors.
    // HAL_UART_Transmit_IT handles its own locking reasonably well for this simple case.
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

    // Call HAL Transmit via DMA
    // We use DMA to drastically reduce CPU load during high-throughput echo
    HAL_StatusTypeDef status = HAL_UART_Transmit_DMA(huart, &tb->buf[tb->tail], len);
    if (status != HAL_OK) {
        // Clear busy flag to allow retry
        tb->busy = 0;
        tb->inflight_len = 0;
        
        // Increment error counter for debugging
        uart_rbuf[ch].error_cnt++;
        
        // If DMA is busy, abort current transfer and retry
        if (status == HAL_BUSY) {
            HAL_UART_AbortTransmit(huart);
        }
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
    // Sync DMA data to User RingBuffer before checking
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
        if (huart != NULL){
            uart_tbuf[i].head = 0;
            uart_tbuf[i].tail = 0;
            uart_tbuf[i].busy = 0;
            uart_tbuf[i].inflight_len = 0;
            
            uart_rbuf[i].head = 0;
            uart_rbuf[i].tail = 0;
            uart_rbuf[i].overrun_cnt = 0;
            
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
            rx_pending[i] = 0;

            // Defensive: Disable IDLE interrupt to prevent unhandled ISR storms if accidentally enabled
            __HAL_UART_DISABLE_IT(huart, UART_IT_IDLE);
            __HAL_UART_CLEAR_IDLEFLAG(huart);
            
            // Start DMA Reception in Circular Mode
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

    // Use local variable for interrupt control
    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    for (uint16_t i = 0; i < len; i++) {
        uint16_t next = (uint16_t)(tb->head + 1);
        if (next >= UART_TX_BUF_SIZE) next = 0;
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
    // Sync DMA first
    UART_ProcessDMA(ch);
    // Pop from RingBuf
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
            // 检查是否有错误需要处理
            if (uart_rbuf[i].error_flag) {
                // 清除错误标志
                uart_rbuf[i].error_flag = 0;
                
                // 重启DMA接收
                HAL_UART_Receive_DMA(huart, RxDMABuf[i], UART_RX_BUF_SIZE);
                
                // 重置位置跟踪
                RxDMAPos[i] = 0;
            }
            
            // 检查TX是否卡住：HAL报告不忙但我们的busy标志仍设置
            // 这可能发生在DMA错误或回调未触发时
            if (uart_tbuf[i].busy && huart->gState == HAL_UART_STATE_READY) {
                // TX可能卡住了，强制恢复
                uart_tbuf[i].busy = 0;
                uart_tbuf[i].inflight_len = 0;
            }
        }
        
        // 处理RX数据
        if (rx_pending[i]) {
            rx_pending[i] = 0;
            UART_ProcessDMA((UART_Channel)i);
        }
        
        // 重试TX
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
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    int ch = UART_HandleToChannel(huart);
    if (ch >= 0 && ch < UART_CHANNEL_MAX) {
        rx_pending[ch] = 1;
    }
}

void HAL_UART_HalfRxCpltCallback(UART_HandleTypeDef *huart)
{
    int ch = UART_HandleToChannel(huart);
    if (ch >= 0 && ch < UART_CHANNEL_MAX) {
        rx_pending[ch] = 1;
    }
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
    
    UART_RingBuf *rb = &uart_rbuf[ch];
    
    // Count total Hardware Errors
    rb->error_cnt++;

    // Identify and count specific error types
    uint32_t error_flags = huart->ErrorCode;
    if (error_flags & HAL_UART_ERROR_PE) {
        rb->pe_error_cnt++;
    }
    if (error_flags & HAL_UART_ERROR_NE) {
        rb->ne_error_cnt++;
    }
    if (error_flags & HAL_UART_ERROR_FE) {
        rb->fe_error_cnt++;
    }
    if (error_flags & HAL_UART_ERROR_ORE) {
        rb->ore_error_cnt++;
    }
    if (error_flags & HAL_UART_ERROR_DMA) {
        rb->dma_error_cnt++;
    }

    // Set error flag for main loop handling
    rb->error_flag = 1;

    // Clear error flags
    __HAL_UART_CLEAR_OREFLAG(huart);
    __HAL_UART_CLEAR_NEFLAG(huart);
    __HAL_UART_CLEAR_FEFLAG(huart);
    __HAL_UART_CLEAR_PEFLAG(huart);
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