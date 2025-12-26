#ifndef USB_CDC_H
#define USB_CDC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#ifndef USB_RX_BUF_SIZE
#define USB_RX_BUF_SIZE 512
#endif

void USB_CDC_Init(void);
void USB_CDC_Hack_Reset(void);

uint32_t USB_CDC_Available(void);
bool USB_CDC_Read(uint8_t *byte);
bool USB_CDC_Receive(uint8_t *out, uint32_t timeout_ms);
void USB_CDC_Flush(void);

void USB_CDC_Send(const uint8_t *data, uint32_t len);
void USB_CDC_SendString(const char *str);
void USB_CDC_Printf(const char *fmt, ...);

// Hook for usbd_cdc_if.c
void USB_CDC_RxCallback(uint8_t *Buf, uint32_t Len);

#ifdef __cplusplus
}
#endif

#endif // USB_CDC_H
