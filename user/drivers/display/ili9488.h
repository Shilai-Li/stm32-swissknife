/**
 * @file ili9488.h
 * @brief ILI9488 SPI TFT LCD Driver Header File
 * @author Standard Implementation
 * @date 2024
 */

#ifndef __ILI9488_H
#define __ILI9488_H

#include "main.h"

#ifndef __STM32F1xx_HAL_SPI_H
#include "main.h"
#endif

// --- Configuration ---
#define ILI9488_WIDTH  320
#define ILI9488_HEIGHT 480

// --- Color Definitions (RGB565) ---
#define ILI9488_BLACK       0x0000
#define ILI9488_NAVY        0x000F
#define ILI9488_DARKGREEN   0x03E0
#define ILI9488_DARKCYAN    0x03EF
#define ILI9488_MAROON      0x7800
#define ILI9488_PURPLE      0x780F
#define ILI9488_OLIVE       0x7BE0
#define ILI9488_LIGHTGREY   0xC618
#define ILI9488_DARKGREY    0x7BEF
#define ILI9488_BLUE        0x001F
#define ILI9488_GREEN       0x07E0
#define ILI9488_CYAN        0x07FF
#define ILI9488_RED         0xF800
#define ILI9488_MAGENTA     0xF81F
#define ILI9488_YELLOW      0xFFE0
#define ILI9488_WHITE       0xFFFF
#define ILI9488_ORANGE      0xFD20
#define ILI9488_GREENYELLOW 0xAFE5
#define ILI9488_PINK        0xF81F

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
} ILI9488_HandleTypeDef;

/* Function Prototypes */

/**
 * @brief Initialize the ILI9488 LCD
 */
uint8_t ILI9488_Init(ILI9488_HandleTypeDef *hlcd, SPI_HandleTypeDef *hspi, 
                     GPIO_TypeDef *cs_port, uint16_t cs_pin,
                     GPIO_TypeDef *dc_port, uint16_t dc_pin,
                     GPIO_TypeDef *rst_port, uint16_t rst_pin,
                     GPIO_TypeDef *blk_port, uint16_t blk_pin);

void ILI9488_SetRotation(ILI9488_HandleTypeDef *hlcd, uint8_t m);

void ILI9488_FillScreen(ILI9488_HandleTypeDef *hlcd, uint16_t color);
void ILI9488_DrawPixel(ILI9488_HandleTypeDef *hlcd, uint16_t x, uint16_t y, uint16_t color);
void ILI9488_FillRect(ILI9488_HandleTypeDef *hlcd, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void ILI9488_DrawImage(ILI9488_HandleTypeDef *hlcd, uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t* data);
void ILI9488_InvertColors(ILI9488_HandleTypeDef *hlcd, uint8_t invert);

#endif // __ILI9488_H
