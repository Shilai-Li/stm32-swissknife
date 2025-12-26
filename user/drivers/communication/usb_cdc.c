/**
 * @file usb_cdc.c
 * @brief USB CDC Wrapper Implementation
 */

#include "usb_cdc.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

// Include HAL/CubeMX generated Headers
#include "main.h" 
// Expecting "usbd_cdc_if.h" to be available in include path for CDC_Transmit_FS
// If user has not generated USB code, this will fail to link/compile.
#include "usbd_cdc_if.h" 
#include "usbd_core.h" // For USBD_STATE_CONFIGURED

extern USBD_HandleTypeDef hUsbDeviceFS;

/* ============================================================================
 * Internal Types
 * ========================================================================= */
typedef struct {
    uint8_t buf[USB_RX_BUF_SIZE];
    volatile uint32_t head;
    volatile uint32_t tail;
    volatile uint32_t overrun_cnt;
} USB_RingBuf_t;

static USB_RingBuf_t rx_rb;

/* ============================================================================
 * Internal Helpers
 * ========================================================================= */
static void RB_Push(USB_RingBuf_t *rb, uint8_t b) {
    uint32_t next = (rb->head + 1) % USB_RX_BUF_SIZE;
    if (next != rb->tail) {
        rb->buf[rb->head] = b;
        rb->head = next;
    } else {
        rb->overrun_cnt++;
    }
}

static bool RB_Pop(USB_RingBuf_t *rb, uint8_t *b) {
    if (rb->head == rb->tail) return false;
    *b = rb->buf[rb->tail];
    rb->tail = (rb->tail + 1) % USB_RX_BUF_SIZE;
    return true;
}

/* ============================================================================
 * Public API
 * ========================================================================= */

// Call this at the very beginning of main() for F103 boards
void USB_CDC_Hack_Reset(void) {
#if defined(STM32F103xB)
    // Hack: Force USB Re-enumeration on F103 Blue Pill
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    
    // 1. Take control of PA12 (USB D+)
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    // 2. Pull Low to simulate disconnect
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);
    HAL_Delay(50); 
    // 3. Release back to USB Peripheral
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_SET); 
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_12);
#endif
}

void USB_CDC_Init(void) {
    rx_rb.head = 0;
    rx_rb.tail = 0;
    rx_rb.overrun_cnt = 0;
}

uint32_t USB_CDC_Available(void) {
    USB_RingBuf_t *rb = &rx_rb;
    if (rb->head >= rb->tail) 
        return rb->head - rb->tail;
    else 
        return USB_RX_BUF_SIZE - (rb->tail - rb->head);
}

bool USB_CDC_Read(uint8_t *byte) {
    return RB_Pop(&rx_rb, byte);
}

bool USB_CDC_Receive(uint8_t *out, uint32_t timeout_ms) {
    uint32_t start = HAL_GetTick();
    while ((HAL_GetTick() - start) < timeout_ms) {
        if (USB_CDC_Read(out)) return true;
    }
    return false;
}

void USB_CDC_Flush(void) {
    rx_rb.head = 0;
    rx_rb.tail = 0;
}

void USB_CDC_Send(const uint8_t *data, uint32_t len) {
    if (len == 0 || data == NULL) return;

    // Check if USB is Configured (Enumerated by PC)
    // If not, DROP the data. Sending now would lock up the TxState forever
    // because no ISR will fire to clear it.
    if (hUsbDeviceFS.dev_state != USBD_STATE_CONFIGURED) {
        return; 
    }

    // Direct call to ST CDC Driver
    // CDC_Transmit_FS is non-blocking BUT returns BUSY if previous transfer is not done.
    // We add a small timeout retry mechanism.
    
    uint32_t start = HAL_GetTick();
    uint8_t result;
    
    do {
        result = CDC_Transmit_FS((uint8_t*)data, (uint16_t)len);
        if (result == USBD_OK) return;
        
        // Wait 1ms before retry to let USB work
        // Note: DON'T use HAL_Delay inside interrupts, but this is usually main loop
        // If in Interrupt context, this loop might block briefly.
    } while ((HAL_GetTick() - start) < 50); // 50ms Timeout
}

void USB_CDC_SendString(const char *str) {
    if (str) USB_CDC_Send((const uint8_t*)str, strlen(str));
}

void USB_CDC_Printf(const char *fmt, ...) {
    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    
    USB_CDC_Send((uint8_t*)buf, strlen(buf));
}

/* ============================================================================
 * Hooks
 * ========================================================================= */

void USB_CDC_RxCallback(uint8_t *Buf, uint32_t Len) {
    for (uint32_t i = 0; i < Len; i++) {
        RB_Push(&rx_rb, Buf[i]);
    }
}
