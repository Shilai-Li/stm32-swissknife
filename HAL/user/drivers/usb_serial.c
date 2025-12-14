/**
 * @file usb_serial.c
 * @brief USB Serial Wrapper Implementation
 * 
 * =================================================================================
 *                       >>> INTEGRATION GUIDE <<<
 * =================================================================================
 * 
 * 1. Enable USB in CubeMX:
 *    - Connectivity -> USB -> Device (FS)
 *    - Middleware -> USB_DEVICE -> Class For FS IP -> Communication Device Class (VCP)
 *    - Generate Code.
 * 
 * 2. Modify 'USB_DEVICE/App/usbd_cdc_if.c':
 *    
 *    a) Add Include:
 *       / * USER CODE BEGIN Includes * /
 *       #include "drivers/usb_serial.h"
 *       / * USER CODE END Includes * /
 * 
 *    b) Modify 'CDC_Receive_FS' function:
 *       static int8_t CDC_Receive_FS(uint8_t* Buf, uint32_t *Len)
 *       {
 *         / * USER CODE BEGIN 6 * /
 *         
 *         // === ADD THIS LINE ===
 *         USB_Serial_RxCallback(Buf, *Len);
 *         // =====================
 * 
 *         USBD_CDC_SetRxBuffer(&hUsbDeviceFS, &Buf[0]);
 *         USBD_CDC_ReceivePacket(&hUsbDeviceFS);
 *         return (USBD_OK);
 *         / * USER CODE END 6 * /
 *       }
 * 
 * 3. Use in main.c:
 *    USB_Serial_Init();
 *    USB_Serial_Printf("Hello World\n");
 * =================================================================================
 */

#include "usb_serial.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

// Include the CubeMX generated header to access CDC functions
// Note: This path depends on where CubeMX puts files. 
// Usually: "usbd_cdc_if.h" is in proper include path.
#include "usbd_cdc_if.h" 

/* We check if usbd_cdc_if.h was actually found/valid by checking a CDC macro.
   If not compiling with USB enabled, these will just be stubs to prevent error.
*/
// USBD_CDC_IF_H is standard define guard in ST generated code usually.
// But mostly we rely on the user to ONLY add this driver if they have USB.
// We can use a weak reference or just assume it links.

typedef struct {
    uint8_t buf[USB_RX_BUF_SIZE];
    volatile uint32_t head;
    volatile uint32_t tail;
} RingBuf_t;

static RingBuf_t rx_rb;

void USB_Serial_Init(void) {
    rx_rb.head = 0;
    rx_rb.tail = 0;
}

// Helper: Push
static void RB_Push(RingBuf_t *rb, uint8_t b) {
    uint32_t next = (rb->head + 1) % USB_RX_BUF_SIZE;
    if (next != rb->tail) {
        rb->buf[rb->head] = b;
        rb->head = next;
    }
}

// Helper: Pop
static bool RB_Pop(RingBuf_t *rb, uint8_t *b) {
    if (rb->head == rb->tail) return false;
    *b = rb->buf[rb->tail];
    rb->tail = (rb->tail + 1) % USB_RX_BUF_SIZE;
    return true;
}

/* --- Public User API --- */

uint32_t USB_Serial_Available(void) {
    if (rx_rb.head >= rx_rb.tail) 
        return rx_rb.head - rx_rb.tail;
    else 
        return USB_RX_BUF_SIZE - (rx_rb.tail - rx_rb.head);
}

bool USB_Serial_Read(uint8_t *byte) {
    return RB_Pop(&rx_rb, byte);
}

void USB_Serial_Write(const uint8_t *data, uint32_t len) {
    // Call the generated function
    // Note: CDC_Transmit_FS is non-waiting. If busy, it returns USBD_BUSY.
    // A robust driver would wait or buffer TX.
    // For simplicity, we spin-wait briefly (not ideal but standard for simple logging)
    
    // We need to cast usually.
    // Return type is uint8_t (USBD_StatusTypeDef)
    
    // Simple retry loop
    uint32_t start = HAL_GetTick();
    while (CDC_Transmit_FS((uint8_t*)data, (uint16_t)len) == USBD_BUSY) {
        if (HAL_GetTick() - start > 10) break; // 10ms timeout drop
    }
}

void USB_Serial_Printf(const char *fmt, ...) {
    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    
    USB_Serial_Write((uint8_t*)buf, strlen(buf));
}

/* --- Hook for Stack --- */

void USB_Serial_RxCallback(uint8_t *Buf, uint32_t Len) {
    for (uint32_t i = 0; i < Len; i++) {
        RB_Push(&rx_rb, Buf[i]);
    }
}
