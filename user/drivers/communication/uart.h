#ifndef __UART_DRIVER_H__
#define __UART_DRIVER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

// Logic channel ID, managed by user application
typedef uint8_t UART_Channel;

#define UART_CHANNEL_MAX 3 // Reduced to save RAM (was 8)

/* ============================================================================
 * UART Configuration
 * ========================================================================= */
#ifndef UART_RX_BUF_SIZE
#define UART_RX_BUF_SIZE  128  // Reduced to save RAM (was 2048)
#endif

#ifndef UART_TX_BUF_SIZE
#define UART_TX_BUF_SIZE  128  // Reduced to save RAM (was 2048)
#endif

// UART_DEBUG_CHANNEL should be defined by user if they use UART_Debug_Printf, 
// usually as a macro in main.h or here if hardcoded. Defaulting to 0.
#ifndef UART_DEBUG_CHANNEL
#define UART_DEBUG_CHANNEL 0
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
// Register a hardware handle to a logic channel
// huart should be (UART_HandleTypeDef *)
void UART_Register(UART_Channel channel, void *huart);
void UART_Init(void); // Optional: global init if needed, or remove. I will remove it to force registration.
// Actually keeping UART_Init empty or removing it. Removing it is better.
// But wait, the previous code had UART_Init calling HAL_UART_Receive_DMA. 
// We should move that logic to UART_Register.
// So let's just add UART_Register.

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
