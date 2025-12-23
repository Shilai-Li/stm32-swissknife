/**
 * @file ili9341.h
 * @brief ILI9341 SPI TFT LCD Driver Header File
 * @author Standard Implementation
 * @date 2024
 */

#ifndef __ILI9341_H
#define __ILI9341_H

#include "main.h"

#ifndef __STM32F1xx_HAL_SPI_H
#include "stm32f1xx_hal.h"
#endif

// --- Configuration ---
#define ILI9341_WIDTH  240
#define ILI9341_HEIGHT 320

// --- Color Definitions (RGB565) ---
#define ILI9341_BLACK       0x0000
#define ILI9341_NAVY        0x000F
#define ILI9341_DARKGREEN   0x03E0
#define ILI9341_DARKCYAN    0x03EF
#define ILI9341_MAROON      0x7800
#define ILI9341_PURPLE      0x780F
#define ILI9341_OLIVE       0x7BE0
#define ILI9341_LIGHTGREY   0xC618
#define ILI9341_DARKGREY    0x7BEF
#define ILI9341_BLUE        0x001F
#define ILI9341_GREEN       0x07E0
#define ILI9341_CYAN        0x07FF
#define ILI9341_RED         0xF800
#define ILI9341_MAGENTA     0xF81F
#define ILI9341_YELLOW      0xFFE0
#define ILI9341_WHITE       0xFFFF
#define ILI9341_ORANGE      0xFD20
#define ILI9341_GREENYELLOW 0xAFE5
#define ILI9341_PINK        0xF81F

typedef struct {
    SPI_HandleTypeDef *hspi;
    GPIO_TypeDef      *CsPort;
    uint16_t          CsPin;
    GPIO_TypeDef      *DcPort;
    uint16_t          DcPin;
    GPIO_TypeDef      *RstPort;
    uint16_t          RstPin;
    GPIO_TypeDef      *BlkPort; // Backlight, optional
    uint16_t          BlkPin;
    uint16_t          Width;
    uint16_t          Height;
    uint8_t           Rotation; // 0-3
} ILI9341_HandleTypeDef;

/* Function Prototypes */

/**
 * @brief Initialize the ILI9341 LCD
 */
uint8_t ILI9341_Init(ILI9341_HandleTypeDef *hlcd, SPI_HandleTypeDef *hspi, 
                     GPIO_TypeDef *cs_port, uint16_t cs_pin,
                     GPIO_TypeDef *dc_port, uint16_t dc_pin,
                     GPIO_TypeDef *rst_port, uint16_t rst_pin,
                     GPIO_TypeDef *blk_port, uint16_t blk_pin);

/**
 * @brief Set Display Rotation
 * @param m 0:0deg, 1:90deg, 2:180deg, 3:270deg
 */
void ILI9341_SetRotation(ILI9341_HandleTypeDef *hlcd, uint8_t m);

void ILI9341_FillScreen(ILI9341_HandleTypeDef *hlcd, uint16_t color);
void ILI9341_DrawPixel(ILI9341_HandleTypeDef *hlcd, uint16_t x, uint16_t y, uint16_t color);
void ILI9341_FillRect(ILI9341_HandleTypeDef *hlcd, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void ILI9341_DrawImage(ILI9341_HandleTypeDef *hlcd, uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t* data);
void ILI9341_InvertColors(ILI9341_HandleTypeDef *hlcd, uint8_t invert);

#endif // __ILI9341_H
