/**
 * @file st7735.h
 * @brief ST7735 SPI TFT LCD Driver Header File
 * @author Standard Implementation
 * @date 2024
 */

#ifndef __ST7735_H
#define __ST7735_H

#include "main.h"

#ifndef __STM32F1xx_HAL_SPI_H
#include "stm32f1xx_hal.h"
#endif

// --- Configuration ---
// Common ST7735 Resolutions
// 1.8" usually 128x160
// 0.96" usually 80x160
// 1.44" usually 128x128
#define ST7735_WIDTH  128
#define ST7735_HEIGHT 160

// Offset management (often needed for ST7735 variants)
// For generic 1.8" Red Tab: x=0, y=0
// For 0.96" IPS: x=26, y=1
// We will store these in the handle
#define ST7735_X_OFFSET 0
#define ST7735_Y_OFFSET 0

// --- Color Definitions (RGB565) ---
#define ST7735_BLACK   0x0000
#define ST7735_BLUE    0x001F
#define ST7735_RED     0xF800
#define ST7735_GREEN   0x07E0
#define ST7735_CYAN    0x07FF
#define ST7735_MAGENTA 0xF81F
#define ST7735_YELLOW  0xFFE0
#define ST7735_WHITE   0xFFFF
#define ST7735_ORANGE  0xFD20

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
    
    // Limits and Offsets for different panel variants
    uint16_t          Width;
    uint16_t          Height;
    uint16_t          XOffset;
    uint16_t          YOffset;
    uint8_t           Rotation;
} ST7735_HandleTypeDef;

/* Function Prototypes */

/**
 * @brief Initialize the ST7735 LCD
 */
uint8_t ST7735_Init(ST7735_HandleTypeDef *hlcd, SPI_HandleTypeDef *hspi, 
                    GPIO_TypeDef *cs_port, uint16_t cs_pin,
                    GPIO_TypeDef *dc_port, uint16_t dc_pin,
                    GPIO_TypeDef *rst_port, uint16_t rst_pin,
                    GPIO_TypeDef *blk_port, uint16_t blk_pin);

/**
 * @brief Set screen type specific offsets (Call after Init if you have a specific panel)
 * @param x_off X Offset (e.g. 26 for 0.96" IPS)
 * @param y_off Y Offset (e.g. 1)
 * @param w Width
 * @param h Height
 */
void ST7735_SetType(ST7735_HandleTypeDef *hlcd, uint16_t x_off, uint16_t y_off, uint16_t w, uint16_t h);

void ST7735_SetRotation(ST7735_HandleTypeDef *hlcd, uint8_t m);
void ST7735_InvertColors(ST7735_HandleTypeDef *hlcd, uint8_t invert);

void ST7735_FillScreen(ST7735_HandleTypeDef *hlcd, uint16_t color);
void ST7735_DrawPixel(ST7735_HandleTypeDef *hlcd, uint16_t x, uint16_t y, uint16_t color);
void ST7735_FillRect(ST7735_HandleTypeDef *hlcd, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void ST7735_DrawImage(ST7735_HandleTypeDef *hlcd, uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t* data);

#endif // __ST7735_H
