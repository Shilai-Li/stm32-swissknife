#ifndef __XPT2046_H
#define __XPT2046_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

// --- Constants ---
#define XPT2046_X_MIN       200
#define XPT2046_X_MAX       3900
#define XPT2046_Y_MIN       200
#define XPT2046_Y_MAX       3900
#define XPT2046_WIDTH       320
#define XPT2046_HEIGHT      240

// IO Interface Signature
// Returns 0 on Success, non-zero on Error (matching HAL_OK=0)
typedef uint8_t (*XPT_TransmitReceive_Func)(void *handle, uint8_t *tx_data, uint8_t *rx_data, uint16_t size, uint32_t timeout);

typedef struct {
    // Generic IO
    void                    *handle;    // Defines hardware context (SPI_Handle or Soft_SPI_Handle)
    XPT_TransmitReceive_Func spi_func;  // Function to perform transfer

    GPIO_TypeDef      *CsPort;     
    uint16_t           CsPin;      
    GPIO_TypeDef      *IrqPort;    
    uint16_t           IrqPin;     
    
    // Calibration
    uint16_t           x_min, x_max;
    uint16_t           y_min, y_max;
    uint16_t           width, height;
    uint8_t            Rotation;   
} XPT2046_HandleTypeDef;

// Generic Init
void XPT2046_Init(XPT2046_HandleTypeDef *htouch, 
                  void *spi_handle, XPT_TransmitReceive_Func spi_func,
                  GPIO_TypeDef *cs_port, uint16_t cs_pin,
                  GPIO_TypeDef *irq_port, uint16_t irq_pin);

// Removing specific Soft/Hard Init functions as they are now unified
// void XPT2046_Init_Soft(...) -> Removed

// Set Calibration
void XPT2046_SetCalibration(XPT2046_HandleTypeDef *htouch, 
                            uint16_t width, uint16_t height,
                            uint16_t x_min, uint16_t x_max, 
                            uint16_t y_min, uint16_t y_max);

// Set Rotation (0-3)
void XPT2046_SetRotation(XPT2046_HandleTypeDef *htouch, uint8_t rotation);

// Check if touched (Polling IRQ Pin)
uint8_t XPT2046_IsTouched(XPT2046_HandleTypeDef *htouch);

// Get Coordinates (Returns 1 if valid)
uint8_t XPT2046_GetCoordinates(XPT2046_HandleTypeDef *htouch, uint16_t *x, uint16_t *y);

#ifdef __cplusplus
}
#endif

#endif // __XPT2046_H
