/**
 * @file xpt2046.h
 * @brief XPT2046 Touch Screen Controller Driver Header File
 * @author Standard Implementation
 * @date 2024
 */

#ifndef __XPT2046_H
#define __XPT2046_H

#include "main.h"

#ifndef __STM32F1xx_HAL_SPI_H
#include "stm32f1xx_hal.h"
#endif

// --- Calibration Constants (Default for 2.4/2.8/3.5 inch screens) ---
// User should call XPT2046_SetCalibration to override these
#define XPT2046_X_MIN       200
#define XPT2046_X_MAX       3900
#define XPT2046_Y_MIN       200
#define XPT2046_Y_MAX       3900

// --- Screen Resolution (Logical) ---
#define XPT2046_WIDTH       320
#define XPT2046_HEIGHT      240

typedef struct {
    SPI_HandleTypeDef *hspi;       // SPI Handle
    GPIO_TypeDef      *CsPort;     // Chip Select Port
    uint16_t           CsPin;      // Chip Select Pin
    GPIO_TypeDef      *IrqPort;    // IRQ/PenIRQ Port (Optional, Input)
    uint16_t           IrqPin;     // IRQ/PenIRQ Pin
    
    // Calibration Data
    uint16_t           x_min;
    uint16_t           x_max;
    uint16_t           y_min;
    uint16_t           y_max;
    uint16_t           width;
    uint16_t           height;
    uint8_t            Rotation;   // 0, 1, 2, 3 (Matches LCD Rotation)
} XPT2046_HandleTypeDef;

/* Function Prototypes */

/**
 * @brief Initialize XPT2046 Driver
 */
void XPT2046_Init(XPT2046_HandleTypeDef *htouch, SPI_HandleTypeDef *hspi, 
                  GPIO_TypeDef *cs_port, uint16_t cs_pin,
                  GPIO_TypeDef *irq_port, uint16_t irq_pin);

/**
 * @brief Set Calibration and Resolution manually
 */
void XPT2046_SetCalibration(XPT2046_HandleTypeDef *htouch, 
                            uint16_t width, uint16_t height,
                            uint16_t x_min, uint16_t x_max, 
                            uint16_t y_min, uint16_t y_max);

/**
 * @brief Set Rotation (0-3) to match LCD orientation
 */
void XPT2046_SetRotation(XPT2046_HandleTypeDef *htouch, uint8_t rotation);

/**
 * @brief Check if screen is currently touched
 * @return 1 if touched, 0 if not (Based on IRQ Pin if available, or Dummy Read)
 */
uint8_t XPT2046_IsTouched(XPT2046_HandleTypeDef *htouch);

/**
 * @brief Get Touch Coordinates (Physical -> Logical mapping)
 * @param x Pointer to store X coordinate
 * @param y Pointer to store Y coordinate
 * @return 1 if valid touch read, 0 if no touch or error
 */
uint8_t XPT2046_GetCoordinates(XPT2046_HandleTypeDef *htouch, uint16_t *x, uint16_t *y);

#endif // __XPT2046_H
