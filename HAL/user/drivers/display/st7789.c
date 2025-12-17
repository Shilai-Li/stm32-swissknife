/**
 * @file st7789.c
 * @brief ST7789 SPI TFT LCD Driver Implementation
 * @author Standard Implementation
 * @date 2024
 */

#include "drivers/st7789.h"
#include "drivers/delay.h" // Assuming user has delay_driver.h or similar

// --- Command Definitions ---
#define ST7789_SWRESET    0x01
#define ST7789_SLPIN      0x10
#define ST7789_SLPOUT     0x11
#define ST7789_NORON      0x13
#define ST7789_INVOFF     0x20
#define ST7789_INVON      0x21
#define ST7789_DISPOFF    0x28
#define ST7789_DISPON     0x29
#define ST7789_CASET      0x2A
#define ST7789_RASET      0x2B
#define ST7789_RAMWR      0x2C
#define ST7789_MADCTL     0x36
#define ST7789_COLMOD     0x3A

#define ST7789_MADCTL_MY  0x80
#define ST7789_MADCTL_MX  0x40
#define ST7789_MADCTL_MV  0x20
#define ST7789_MADCTL_ML  0x10
#define ST7789_MADCTL_RGB 0x00

// --- Private Functions ---

static void ST7789_WriteCommand(ST7789_HandleTypeDef *hlcd, uint8_t cmd) {
    HAL_GPIO_WritePin(hlcd->DcPort, hlcd->DcPin, GPIO_PIN_RESET); // Command mode
    HAL_GPIO_WritePin(hlcd->CsPort, hlcd->CsPin, GPIO_PIN_RESET); // Select
    HAL_SPI_Transmit(hlcd->hspi, &cmd, 1, 100);
    HAL_GPIO_WritePin(hlcd->CsPort, hlcd->CsPin, GPIO_PIN_SET);   // Deselect
}

static void ST7789_WriteData(ST7789_HandleTypeDef *hlcd, uint8_t* buff, size_t buff_size) {
    HAL_GPIO_WritePin(hlcd->DcPort, hlcd->DcPin, GPIO_PIN_SET);   // Data mode
    HAL_GPIO_WritePin(hlcd->CsPort, hlcd->CsPin, GPIO_PIN_RESET); // Select
    
    // Split large transfers if necessary (uint16_t size limit in HAL depends on version, usually 65535)
    // But safely we can just transmit. 
    // Note: HAL_SPI_Transmit takes uint16_t for size in some F1 versions, check `HAL_SPI_Transmit`.
    // F1 HAL usually takes uint16_t Size. If > 65535, loop.
    
    size_t remaining = buff_size;
    uint8_t *ptr = buff;
    while (remaining > 0) {
        uint16_t chunk = (remaining > 0xFFFF) ? 0xFFFF : (uint16_t)remaining;
        HAL_SPI_Transmit(hlcd->hspi, ptr, chunk, 1000);
        remaining -= chunk;
        ptr += chunk;
    }

    HAL_GPIO_WritePin(hlcd->CsPort, hlcd->CsPin, GPIO_PIN_SET);   // Deselect
}

static void ST7789_WriteSmallData(ST7789_HandleTypeDef *hlcd, uint8_t data) {
    HAL_GPIO_WritePin(hlcd->DcPort, hlcd->DcPin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(hlcd->CsPort, hlcd->CsPin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(hlcd->hspi, &data, 1, 100);
    HAL_GPIO_WritePin(hlcd->CsPort, hlcd->CsPin, GPIO_PIN_SET);
}

// --- Public Functions ---

uint8_t ST7789_Init(ST7789_HandleTypeDef *hlcd, SPI_HandleTypeDef *hspi, 
                    GPIO_TypeDef *cs_port, uint16_t cs_pin,
                    GPIO_TypeDef *dc_port, uint16_t dc_pin,
                    GPIO_TypeDef *rst_port, uint16_t rst_pin,
                    GPIO_TypeDef *blk_port, uint16_t blk_pin) 
{
    hlcd->hspi = hspi;
    hlcd->CsPort = cs_port; hlcd->CsPin = cs_pin;
    hlcd->DcPort = dc_port; hlcd->DcPin = dc_pin;
    hlcd->RstPort = rst_port; hlcd->RstPin = rst_pin;
    hlcd->BlkPort = blk_port; hlcd->BlkPin = blk_pin;

    // Hard Reset
    HAL_GPIO_WritePin(hlcd->CsPort, hlcd->CsPin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(hlcd->RstPort, hlcd->RstPin, GPIO_PIN_RESET);
    HAL_Delay(50);
    HAL_GPIO_WritePin(hlcd->RstPort, hlcd->RstPin, GPIO_PIN_SET);
    HAL_Delay(50);

    // Initialization Sequence
    ST7789_WriteCommand(hlcd, ST7789_SWRESET);
    HAL_Delay(150);

    ST7789_WriteCommand(hlcd, ST7789_SLPOUT);
    HAL_Delay(255);

    ST7789_WriteCommand(hlcd, ST7789_COLMOD);
    ST7789_WriteSmallData(hlcd, 0x55); // 16-bit color format
    HAL_Delay(10);

    ST7789_WriteCommand(hlcd, ST7789_MADCTL); 
    ST7789_WriteSmallData(hlcd, 0x00); // Default orientation

    ST7789_WriteCommand(hlcd, ST7789_INVON); // Most panels are IPS and need inversion
    HAL_Delay(10);

    ST7789_WriteCommand(hlcd, ST7789_NORON);
    HAL_Delay(10);

    ST7789_WriteCommand(hlcd, ST7789_DISPON);
    HAL_Delay(10); // Wait for display to start

    if (hlcd->BlkPort != NULL) {
        HAL_GPIO_WritePin(hlcd->BlkPort, hlcd->BlkPin, GPIO_PIN_SET);
    }
    
    // Clear Screen
    ST7789_FillScreen(hlcd, ST7789_BLACK);
    
    return 0;
}

static void ST7789_SetAddressWindow(ST7789_HandleTypeDef *hlcd, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    uint8_t data[4];
    
    ST7789_WriteCommand(hlcd, ST7789_CASET);
    data[0] = (x0 >> 8) & 0xFF;
    data[1] = x0 & 0xFF;
    data[2] = (x1 >> 8) & 0xFF;
    data[3] = x1 & 0xFF;
    ST7789_WriteData(hlcd, data, 4);

    ST7789_WriteCommand(hlcd, ST7789_RASET);
    data[0] = (y0 >> 8) & 0xFF;
    data[1] = y0 & 0xFF;
    data[2] = (y1 >> 8) & 0xFF;
    data[3] = y1 & 0xFF;
    ST7789_WriteData(hlcd, data, 4);

    ST7789_WriteCommand(hlcd, ST7789_RAMWR);
}

void ST7789_FillRect(ST7789_HandleTypeDef *hlcd, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    if ((x >= ST7789_WIDTH) || (y >= ST7789_HEIGHT)) return;
    if ((x + w - 1) >= ST7789_WIDTH) w = ST7789_WIDTH - x;
    if ((y + h - 1) >= ST7789_HEIGHT) h = ST7789_HEIGHT - y;

    ST7789_SetAddressWindow(hlcd, x, y, x + w - 1, y + h - 1);

    // Create a line buffer to speed up filling
    uint16_t buffer_size = w; // Fill one line at a time or more
    // Make sure buffer isn't too huge, say 256 pixels
    if (buffer_size > 256) buffer_size = 256; 
    
    // We need bytes: buffer_size * 2
    uint8_t color_buff[512]; 
    uint8_t hi = (color >> 8) & 0xFF;
    uint8_t lo = color & 0xFF;
    
    for (uint16_t i = 0; i < buffer_size; i++) {
        color_buff[i*2] = hi;
        color_buff[i*2+1] = lo;
    }

    uint32_t total_pixels = w * h;
    while (total_pixels > 0) {
        uint32_t chunk_pixels = (total_pixels > buffer_size) ? buffer_size : total_pixels;
        ST7789_WriteData(hlcd, color_buff, chunk_pixels * 2);
        total_pixels -= chunk_pixels;
    }
}

void ST7789_FillScreen(ST7789_HandleTypeDef *hlcd, uint16_t color) {
    ST7789_FillRect(hlcd, 0, 0, ST7789_WIDTH, ST7789_HEIGHT, color);
}

void ST7789_DrawPixel(ST7789_HandleTypeDef *hlcd, uint16_t x, uint16_t y, uint16_t color) {
    if ((x >= ST7789_WIDTH) || (y >= ST7789_HEIGHT)) return;

    ST7789_SetAddressWindow(hlcd, x, y, x, y);
    uint8_t data[2] = {(color >> 8) & 0xFF, color & 0xFF};
    ST7789_WriteData(hlcd, data, 2);
}

void ST7789_DrawImage(ST7789_HandleTypeDef *hlcd, uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t* data) {
    if ((x >= ST7789_WIDTH) || (y >= ST7789_HEIGHT)) return;
    if ((x + w - 1) >= ST7789_WIDTH) return;
    if ((y + h - 1) >= ST7789_HEIGHT) return;

    ST7789_SetAddressWindow(hlcd, x, y, x + w - 1, y + h - 1);
    
    // Data is assumed to be in correct endianness (Big Endian usually for displays, but STM32 is Little Endian)
    // ST7789 expects MSB first. 
    // If input data is uint16_t array in Little Endian (normal C array), we need to swap bytes unless the image was prepared for big endian.
    // Efficient way: convert line by line or use a buffer.
    
    // Assuming 'data' is raw RGB565 Little Endian from an image converter that outputs standard C arrays.
    // We need to send [Hi, Lo, Hi, Lo...]
    
    // Optimization: If we trust the source is Big Endian, we can send directly.
    // Usually images are stored as array of bytes or ushorts. 
    // Let's assume standard behavior: we need to swap bytes for transmission if the MCU is LE and display expects BE.
    // Yes, we need to swap.
    
    // To safe RAM, process in chunks.
    uint32_t total_pixels = w * h;
    uint32_t i = 0;
    uint8_t line_buff[256]; // 128 pixels max
    uint16_t chunk_size = 128;
    
    while (i < total_pixels) {
        uint32_t current_chunk = (total_pixels - i > chunk_size) ? chunk_size : (total_pixels - i);
        for (uint32_t k = 0; k < current_chunk; k++) {
            line_buff[k*2]     = (data[i+k] >> 8) & 0xFF;
            line_buff[k*2+1]   = data[i+k] & 0xFF;
        }
        ST7789_WriteData(hlcd, line_buff, current_chunk * 2);
        i += current_chunk;
    }
}

void ST7789_InvertColors(ST7789_HandleTypeDef *hlcd, uint8_t invert) {
    ST7789_WriteCommand(hlcd, invert ? ST7789_INVON : ST7789_INVOFF);
}
