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
