/**
 * @file ili9488.c
 * @brief ILI9488 SPI TFT LCD Driver Implementation
 * @author Standard Implementation
 * @date 2024
 */

#include "drivers/ili9488.h"
#include "drivers/delay.h"

// --- Command Definitions ---
#define ILI9488_SWRESET     0x01
#define ILI9488_SLPOUT      0x11
#define ILI9488_DINVON      0x21 // Display Inversion On
#define ILI9488_DINVOFF     0x20
#define ILI9488_DISPOFF     0x28
#define ILI9488_DISPON      0x29
#define ILI9488_CASET       0x2A
#define ILI9488_PASET       0x2B
#define ILI9488_RAMWR       0x2C
#define ILI9488_MADCTL      0x36
#define ILI9488_PIXFMT      0x3A

#define ILI9488_FRMCTR1     0xB1
#define ILI9488_INVCTR      0xB4
#define ILI9488_DFUNCTR     0xB6

#define ILI9488_PWCTR1      0xC0
#define ILI9488_PWCTR2      0xC1
#define ILI9488_VMCTR1      0xC5
#define ILI9488_VMCTR2      0xC7

#define ILI9488_PGAMCTRL    0xE0
#define ILI9488_NGAMCTRL    0xE1

// MADCTL Bits
#define MADCTL_MY  0x80
#define MADCTL_MX  0x40
#define MADCTL_MV  0x20
#define MADCTL_ML  0x10
#define MADCTL_RGB 0x00
#define MADCTL_BGR 0x08
#define MADCTL_MH  0x04

// --- Private Functions ---

static void ILI9488_WriteCommand(ILI9488_HandleTypeDef *hlcd, uint8_t cmd) {
    HAL_GPIO_WritePin(hlcd->DcPort, hlcd->DcPin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(hlcd->CsPort, hlcd->CsPin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(hlcd->hspi, &cmd, 1, 100);
    HAL_GPIO_WritePin(hlcd->CsPort, hlcd->CsPin, GPIO_PIN_SET);
}

// Write generic data bytes (parameters)
static void ILI9488_WriteData(ILI9488_HandleTypeDef *hlcd, uint8_t* buff, size_t buff_size) {
    HAL_GPIO_WritePin(hlcd->DcPort, hlcd->DcPin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(hlcd->CsPort, hlcd->CsPin, GPIO_PIN_RESET);
    
    uint8_t *ptr = buff;
    size_t remaining = buff_size;
    while (remaining > 0) {
        uint16_t chunk = (remaining > 0xFFFF) ? 0xFFFF : (uint16_t)remaining;
        HAL_SPI_Transmit(hlcd->hspi, ptr, chunk, 1000);
        remaining -= chunk;
        ptr += chunk;
    }
    HAL_GPIO_WritePin(hlcd->CsPort, hlcd->CsPin, GPIO_PIN_SET);
}

static void ILI9488_WriteSmallData(ILI9488_HandleTypeDef *hlcd, uint8_t data) {
    HAL_GPIO_WritePin(hlcd->DcPort, hlcd->DcPin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(hlcd->CsPort, hlcd->CsPin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(hlcd->hspi, &data, 1, 100);
    HAL_GPIO_WritePin(hlcd->CsPort, hlcd->CsPin, GPIO_PIN_SET);
}

// Convert RGB565 to RGB888 and write
// ILI9488 in 4-wire SPI often requires 18-bit (sent as 3 bytes/pixel)
static void ILI9488_WritePixelData(ILI9488_HandleTypeDef *hlcd, const uint16_t *pixels, size_t pixel_count) {
    HAL_GPIO_WritePin(hlcd->DcPort, hlcd->DcPin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(hlcd->CsPort, hlcd->CsPin, GPIO_PIN_RESET);

    // Buffer for chunking conversion to save RAM
    // 3 bytes per pixel
    uint8_t line_buff[384]; // 128 pixels max per chunk
    size_t max_pixels_per_chunk = sizeof(line_buff) / 3;
    
    size_t processed = 0;
    while (processed < pixel_count) {
        size_t current_chunk = (pixel_count - processed > max_pixels_per_chunk) ? max_pixels_per_chunk : (pixel_count - processed);
        size_t buff_idx = 0;
        
        for (size_t i = 0; i < current_chunk; i++) {
            uint16_t color = pixels[processed + i];
            
            // Extract RGB565
            uint8_t r5 = (color >> 11) & 0x1F;
            uint8_t g6 = (color >> 5) & 0x3F;
            uint8_t b5 = (color & 0x1F);
            
            // Convert to RGB888
            uint8_t r8 = (r5 * 527 + 23) >> 6;
            uint8_t g8 = (g6 * 259 + 33) >> 6;
            uint8_t b8 = (b5 * 527 + 23) >> 6;
            
            line_buff[buff_idx++] = r8;
            line_buff[buff_idx++] = g8;
            line_buff[buff_idx++] = b8;
        }
        
        HAL_SPI_Transmit(hlcd->hspi, line_buff, buff_idx, 1000);
        processed += current_chunk;
    }

    HAL_GPIO_WritePin(hlcd->CsPort, hlcd->CsPin, GPIO_PIN_SET);
}

// --- Public Functions ---

uint8_t ILI9488_Init(ILI9488_HandleTypeDef *hlcd, SPI_HandleTypeDef *hspi, 
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
    hlcd->Width = ILI9488_WIDTH;
    hlcd->Height = ILI9488_HEIGHT;
    hlcd->Rotation = 0;

    // Hardware Reset
    HAL_GPIO_WritePin(hlcd->CsPort, hlcd->CsPin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(hlcd->RstPort, hlcd->RstPin, GPIO_PIN_RESET);
    HAL_Delay(50);
    HAL_GPIO_WritePin(hlcd->RstPort, hlcd->RstPin, GPIO_PIN_SET);
    HAL_Delay(100);

    // Init Sequence
    ILI9488_WriteCommand(hlcd, ILI9488_SWRESET); 
    HAL_Delay(100);

    ILI9488_WriteCommand(hlcd, 0xE0); // Positive Gamma Control
    uint8_t gammaP[] = {0x00, 0x03, 0x09, 0x08, 0x16, 0x0A, 0x3F, 0x78, 0x4C, 0x09, 0x0A, 0x08, 0x16, 0x1A, 0x0F};
    ILI9488_WriteData(hlcd, gammaP, 15);

    ILI9488_WriteCommand(hlcd, 0xE1); // Negative Gamma Control
    uint8_t gammaN[] = {0x00, 0x16, 0x19, 0x03, 0x11, 0x05, 0x26, 0x28, 0x44, 0x04, 0x05, 0x05, 0x24, 0x1C, 0x0F};
    ILI9488_WriteData(hlcd, gammaN, 15);

    ILI9488_WriteCommand(hlcd, 0XC0); // Power Control 1
    ILI9488_WriteSmallData(hlcd, 0x17);
    ILI9488_WriteSmallData(hlcd, 0x15);

    ILI9488_WriteCommand(hlcd, 0xC1); // Power Control 2
    ILI9488_WriteSmallData(hlcd, 0x41);

    ILI9488_WriteCommand(hlcd, 0xC5); // VCOM Control
    ILI9488_WriteSmallData(hlcd, 0x00);
    ILI9488_WriteSmallData(hlcd, 0x12);
    ILI9488_WriteSmallData(hlcd, 0x80);

    ILI9488_WriteCommand(hlcd, ILI9488_MADCTL);
    ILI9488_WriteSmallData(hlcd, 0x48); // MX, BGR

    ILI9488_WriteCommand(hlcd, ILI9488_PIXFMT);
    ILI9488_WriteSmallData(hlcd, 0x66); // 18-bit (RGB666) - Required for SPI 4-wire on ILI9488

    ILI9488_WriteCommand(hlcd, 0xB0); // Interface Mode Control
    ILI9488_WriteSmallData(hlcd, 0x00);

    ILI9488_WriteCommand(hlcd, 0xB1); // Frame Rate Control
    ILI9488_WriteSmallData(hlcd, 0xA0);

    ILI9488_WriteCommand(hlcd, 0xB4); // Display Inversion Control
    ILI9488_WriteSmallData(hlcd, 0x02); // 2-dot inversion

    ILI9488_WriteCommand(hlcd, 0xB6); // Display Function Control
    ILI9488_WriteSmallData(hlcd, 0x02);
    ILI9488_WriteSmallData(hlcd, 0x02);

    ILI9488_WriteCommand(hlcd, 0xE9); // Set Image Function
    ILI9488_WriteSmallData(hlcd, 0x00);

    ILI9488_WriteCommand(hlcd, 0xF7); // Adjust Control 3
    ILI9488_WriteSmallData(hlcd, 0xA9);
    ILI9488_WriteSmallData(hlcd, 0x51);
    ILI9488_WriteSmallData(hlcd, 0x2C);
    ILI9488_WriteSmallData(hlcd, 0x82);

    ILI9488_WriteCommand(hlcd, ILI9488_SLPOUT);
    HAL_Delay(120);

    ILI9488_WriteCommand(hlcd, ILI9488_DISPON);
    HAL_Delay(100);

    ILI9488_SetRotation(hlcd, 0);

    if (hlcd->BlkPort != NULL) {
        HAL_GPIO_WritePin(hlcd->BlkPort, hlcd->BlkPin, GPIO_PIN_SET);
    }
    
    ILI9488_FillScreen(hlcd, ILI9488_BLACK);

    return 0;
}

void ILI9488_SetRotation(ILI9488_HandleTypeDef *hlcd, uint8_t m) {
    ILI9488_WriteCommand(hlcd, ILI9488_MADCTL);
    uint8_t rotation = m % 4; 
    hlcd->Rotation = rotation;
    
    switch (rotation) {
        case 0: // Portrait
            ILI9488_WriteSmallData(hlcd, MADCTL_MX | MADCTL_BGR);
            hlcd->Width  = ILI9488_WIDTH;
            hlcd->Height = ILI9488_HEIGHT;
            break;
        case 1: // Landscape
            ILI9488_WriteSmallData(hlcd, MADCTL_MV | MADCTL_BGR);
            hlcd->Width  = ILI9488_HEIGHT;
            hlcd->Height = ILI9488_WIDTH;
            break;
        case 2: // Inverted Portrait
            ILI9488_WriteSmallData(hlcd, MADCTL_MY | MADCTL_BGR);
            hlcd->Width  = ILI9488_WIDTH;
            hlcd->Height = ILI9488_HEIGHT;
            break;
        case 3: // Inverted Landscape
            ILI9488_WriteSmallData(hlcd, MADCTL_MX | MADCTL_MY | MADCTL_MV | MADCTL_BGR);
            hlcd->Width  = ILI9488_HEIGHT;
            hlcd->Height = ILI9488_WIDTH;
            break;
    }
}

static void ILI9488_SetAddressWindow(ILI9488_HandleTypeDef *hlcd, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    uint8_t data[4];
    
    ILI9488_WriteCommand(hlcd, ILI9488_CASET);
    data[0] = (x0 >> 8) & 0xFF;
    data[1] = x0 & 0xFF;
    data[2] = (x1 >> 8) & 0xFF;
    data[3] = x1 & 0xFF;
    ILI9488_WriteData(hlcd, data, 4);

    ILI9488_WriteCommand(hlcd, ILI9488_PASET);
    data[0] = (y0 >> 8) & 0xFF;
    data[1] = y0 & 0xFF;
    data[2] = (y1 >> 8) & 0xFF;
    data[3] = y1 & 0xFF;
    ILI9488_WriteData(hlcd, data, 4);

    ILI9488_WriteCommand(hlcd, ILI9488_RAMWR);
}

void ILI9488_FillRect(ILI9488_HandleTypeDef *hlcd, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    if ((x >= hlcd->Width) || (y >= hlcd->Height)) return;
    if ((x + w - 1) >= hlcd->Width) w = hlcd->Width - x;
    if ((y + h - 1) >= hlcd->Height) h = hlcd->Height - y;

    ILI9488_SetAddressWindow(hlcd, x, y, x + w - 1, y + h - 1);
    
    // We need to write pixel by pixel (converted to 3 bytes)
    // To be efficient, we prepare a buffer of IDENTICAL pixels
    uint32_t total_pixels = w * h;
    
    // Prepare a small buffer of the converted color
    // 64 pixels * 3 bytes = 192 bytes
    uint8_t r5 = (color >> 11) & 0x1F;
    uint8_t g6 = (color >> 5) & 0x3F;
    uint8_t b5 = (color & 0x1F);
    
    uint8_t r8 = (r5 * 527 + 23) >> 6;
    uint8_t g8 = (g6 * 259 + 33) >> 6;
    uint8_t b8 = (b5 * 527 + 23) >> 6;
    
    uint8_t chunk_buff[192]; // holds 64 pixels
    for (int i=0; i<64; i++) {
        chunk_buff[i*3] = r8;
        chunk_buff[i*3+1] = g8;
        chunk_buff[i*3+2] = b8;
    }
    
    HAL_GPIO_WritePin(hlcd->DcPort, hlcd->DcPin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(hlcd->CsPort, hlcd->CsPin, GPIO_PIN_RESET);
    
    while(total_pixels > 0) {
        uint32_t chunk = (total_pixels > 64) ? 64 : total_pixels;
        HAL_SPI_Transmit(hlcd->hspi, chunk_buff, chunk * 3, 1000);
        total_pixels -= chunk;
    }
    
    HAL_GPIO_WritePin(hlcd->CsPort, hlcd->CsPin, GPIO_PIN_SET);
}

void ILI9488_FillScreen(ILI9488_HandleTypeDef *hlcd, uint16_t color) {
    ILI9488_FillRect(hlcd, 0, 0, hlcd->Width, hlcd->Height, color);
}

void ILI9488_DrawPixel(ILI9488_HandleTypeDef *hlcd, uint16_t x, uint16_t y, uint16_t color) {
    if ((x >= hlcd->Width) || (y >= hlcd->Height)) return;

    ILI9488_SetAddressWindow(hlcd, x, y, x, y);
    // Convert RGB565 to RGB888
    uint8_t r5 = (color >> 11) & 0x1F;
    uint8_t g6 = (color >> 5) & 0x3F;
    uint8_t b5 = (color & 0x1F);
    
    uint8_t data[3];
    data[0] = (r5 * 527 + 23) >> 6; // R
    data[1] = (g6 * 259 + 33) >> 6; // G
    data[2] = (b5 * 527 + 23) >> 6; // B
    
    ILI9488_WriteData(hlcd, data, 3);
}

// Assumes 'data' is array of RGB565
void ILI9488_DrawImage(ILI9488_HandleTypeDef *hlcd, uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t* data) {
    if ((x >= hlcd->Width) || (y >= hlcd->Height)) return;
    ILI9488_SetAddressWindow(hlcd, x, y, x + w - 1, y + h - 1);
    
    uint32_t total_pixels = w * h;
    ILI9488_WritePixelData(hlcd, data, total_pixels);
}

void ILI9488_InvertColors(ILI9488_HandleTypeDef *hlcd, uint8_t invert) {
    ILI9488_WriteCommand(hlcd, invert ? ILI9488_DINVON : ILI9488_DINVOFF);
}
