/**
 * @file usb_serial.h
 * @brief USB CDC Virtual Serial Port Wrapper
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
 *       #include "usb_serial.h"
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

#ifndef USB_SERIAL_H
#define USB_SERIAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/* Config: Buffer Size */
#define USB_RX_BUF_SIZE 512
#define USB_TX_BUF_SIZE 512

/**
 * @brief  Init Packet (Reset buffers)
 */
void USB_Serial_Init(void);

/* ============================================================================
 * INTERFACE FOR USER APPLICATION (Call these in main.c)
 * ========================================================================= */

/**
 * @brief  Check how many bytes are waiting in the RX Buffer
 */
uint32_t USB_Serial_Available(void);

/**
 * @brief  Read one byte from buffer
 * @return true if success, false if buffer empty
 */
bool USB_Serial_Read(uint8_t *byte);

/**
 * @brief  Send Data via USB (Non-Blocking)
 */
void USB_Serial_Write(const uint8_t *data, uint32_t len);

/**
 * @brief  Print formatted string to USB
 */
void USB_Serial_Printf(const char *fmt, ...);

/* ============================================================================
 * HOOKS FOR CUBEMX GENERATED CODE (Call these in usbd_cdc_if.c)
 * ========================================================================= */

/**
 * @brief  Push data into Rx RingBuffer.
 * @note   Call this inside 'CDC_Receive_FS' in 'usbd_cdc_if.c'.
 * @param  Buf Buffer received from USB Stack
 * @param  Len Length of data
 */
void USB_Serial_RxCallback(uint8_t *Buf, uint32_t Len);

/**
 * @brief  Polls Tx transmission.
 * @note   If loop mode is needed or checking busy state. 
 *         Usually not strictly needed if CDC_Transmit_FS checks busy internaly.
 */
void USB_Serial_Task(void);

#ifdef __cplusplus
}
#endif

#endif // USB_SERIAL_H
