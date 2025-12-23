/**
 * @file st7789.h
 * @brief ST7789 SPI TFT LCD Driver Header File
 * 
 * =================================================================================
 *                       >>> INTEGRATION GUIDE <<<
 * =================================================================================
 * 1. CubeMX Config (Connectivity -> SPIx):
 *    - Mode: Transmit Only Master (or Full-Duplex Master)
 *    - Data Size: 8 bits
 *    - Prescaler: As fast as possible (e.g. 36MHz or 18MHz).
 *    - CPOL: High, CPHA: 2 Edge (Mode 3 usually, or Mode 0).
 *      * ST7789 is versatile, usually Mode 3 works best.
 * 
 * 2. DMA (Optional but Recommended):
 *    - SPIx_TX: Priority High, Mode Normal, Data Width Byte.
 * 
 * 3. GPIO:
 *    - CS (Chip Select): Output.
 *    - DC (Data/Command): Output.
 *    - RES (Reset): Output.
 *    - BLK (Backlight): Output (or PWM).
 * =================================================================================
 */

#ifndef __ST7789_H
#define __ST7789_H

#include "main.h"

#ifndef __STM32F1xx_HAL_SPI_H
#include "stm32f1xx_hal.h"
#endif

// --- Configuration ---
#define ST7789_WIDTH  240
#define ST7789_HEIGHT 240 // or 320 for 2 inch screens

// --- Color Definitions (RGB565) ---
#define ST7789_BLACK   0x0000
#define ST7789_BLUE    0x001F
#define ST7789_RED     0xF800
#define ST7789_GREEN   0x07E0
#define ST7789_CYAN    0x07FF
#define ST7789_MAGENTA 0xF81F
#define ST7789_YELLOW  0xFFE0
#define ST7789_WHITE   0xFFFF
#define ST7789_ORANGE  0xFD20

typedef struct {
    SPI_HandleTypeDef *hspi;
    GPIO_TypeDef      *CsPort;
    uint16_t          CsPin;
    GPIO_TypeDef      *DcPort;
    uint16_t          DcPin;
    GPIO_TypeDef      *RstPort;
    uint16_t          RstPin;
    GPIO_TypeDef      *BlkPort; // Backlight, optional (set to NULL if not used)
    uint16_t          BlkPin;
} ST7789_HandleTypeDef;

/* Function Prototypes */

/**
 * @brief Initialize the ST7789 LCD
 * @param hlcd Pointer to the ST7789 handle
 * @param hspi Pointer to the SPI handle
 * @param cs_port Chip Select GPIO Port
 * @param cs_pin Chip Select GPIO Pin
 * @param dc_port Data/Command GPIO Port
 * @param dc_pin Data/Command GPIO Pin
 * @param rst_port Reset GPIO Port
 * @param rst_pin Reset GPIO Pin
 * @param blk_port Backlight GPIO Port (optional, can be NULL)
 * @param blk_pin Backlight GPIO Pin (optional)
 * @return 0 on success
 */
uint8_t ST7789_Init(ST7789_HandleTypeDef *hlcd, SPI_HandleTypeDef *hspi, 
                    GPIO_TypeDef *cs_port, uint16_t cs_pin,
                    GPIO_TypeDef *dc_port, uint16_t dc_pin,
                    GPIO_TypeDef *rst_port, uint16_t rst_pin,
                    GPIO_TypeDef *blk_port, uint16_t blk_pin);

void ST7789_SetRotation(ST7789_HandleTypeDef *hlcd, uint8_t m);
void ST7789_InvertColors(ST7789_HandleTypeDef *hlcd, uint8_t invert);

void ST7789_FillScreen(ST7789_HandleTypeDef *hlcd, uint16_t color);
void ST7789_DrawPixel(ST7789_HandleTypeDef *hlcd, uint16_t x, uint16_t y, uint16_t color);
void ST7789_FillRect(ST7789_HandleTypeDef *hlcd, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void ST7789_DrawImage(ST7789_HandleTypeDef *hlcd, uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t* data);

// Simple test pattern
void ST7789_TestSequence(ST7789_HandleTypeDef *hlcd);

#endif // __ST7789_H
