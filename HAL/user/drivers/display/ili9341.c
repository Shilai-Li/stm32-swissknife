/**
 * @file ili9341.c
 * @brief ILI9341 SPI TFT LCD Driver Implementation
 * @author Standard Implementation
 * @date 2024
 */

#include "drivers/ili9341.h"
#include "drivers/delay.h"

// --- Command Definitions ---
#define ILI9341_NOP         0x00
#define ILI9341_SWRESET     0x01
#define ILI9341_RDDID       0x04
#define ILI9341_RDDST       0x09

#define ILI9341_SLPIN       0x10
#define ILI9341_SLPOUT      0x11
#define ILI9341_PTLON       0x12
#define ILI9341_NORON       0x13

#define ILI9341_RDMODE      0x0A
#define ILI9341_RDMADCTL    0x0B
#define ILI9341_RDPIXFMT    0x0C
#define ILI9341_RDIMGFMT    0x0D
#define ILI9341_RDSELFDIAG  0x0F

#define ILI9341_INVOFF      0x20
#define ILI9341_INVON       0x21
#define ILI9341_GAMMASET    0x26
#define ILI9341_DISPOFF     0x28
#define ILI9341_DISPON      0x29

#define ILI9341_CASET       0x2A
#define ILI9341_PASET       0x2B
#define ILI9341_RAMWR       0x2C
#define ILI9341_RAMRD       0x2E

#define ILI9341_PTLAR       0x30
#define ILI9341_MADCTL      0x36
#define ILI9341_VSCRSADD    0x37
#define ILI9341_PIXFMT      0x3A

#define ILI9341_FRMCTR1     0xB1
#define ILI9341_FRMCTR2     0xB2
#define ILI9341_FRMCTR3     0xB3
#define ILI9341_INVCTR      0xB4
#define ILI9341_DFUNCTR     0xB6

#define ILI9341_PWCTR1      0xC0
#define ILI9341_PWCTR2      0xC1
#define ILI9341_PWCTR3      0xC2
#define ILI9341_PWCTR4      0xC3
#define ILI9341_PWCTR5      0xC4
#define ILI9341_VMCTR1      0xC5
#define ILI9341_VMCTR2      0xC7

#define ILI9341_RDID1       0xDA
#define ILI9341_RDID2       0xDB
#define ILI9341_RDID3       0xDC
#define ILI9341_RDID4       0xDD

#define ILI9341_GMCTRP1     0xE0
#define ILI9341_GMCTRN1     0xE1

#define MADCTL_MY  0x80
#define MADCTL_MX  0x40
#define MADCTL_MV  0x20
#define MADCTL_ML  0x10
#define MADCTL_RGB 0x00
#define MADCTL_BGR 0x08
#define MADCTL_MH  0x04

// --- Private Functions ---

static void ILI9341_WriteCommand(ILI9341_HandleTypeDef *hlcd, uint8_t cmd) {
    HAL_GPIO_WritePin(hlcd->DcPort, hlcd->DcPin, GPIO_PIN_RESET); // Command
    HAL_GPIO_WritePin(hlcd->CsPort, hlcd->CsPin, GPIO_PIN_RESET); // Select
    HAL_SPI_Transmit(hlcd->hspi, &cmd, 1, 100);
    HAL_GPIO_WritePin(hlcd->CsPort, hlcd->CsPin, GPIO_PIN_SET);   // Deselect
}

static void ILI9341_WriteData(ILI9341_HandleTypeDef *hlcd, uint8_t* buff, size_t buff_size) {
    HAL_GPIO_WritePin(hlcd->DcPort, hlcd->DcPin, GPIO_PIN_SET);   // Data
    HAL_GPIO_WritePin(hlcd->CsPort, hlcd->CsPin, GPIO_PIN_RESET); // Select
    
    // Handle large transfers by chunking
    uint8_t *ptr = buff;
    size_t remaining = buff_size;
    while (remaining > 0) {
        uint16_t chunk = (remaining > 0xFFFF) ? 0xFFFF : (uint16_t)remaining;
        HAL_SPI_Transmit(hlcd->hspi, ptr, chunk, 1000);
        remaining -= chunk;
        ptr += chunk;
    }
    
    HAL_GPIO_WritePin(hlcd->CsPort, hlcd->CsPin, GPIO_PIN_SET);   // Deselect
}

static void ILI9341_WriteSmallData(ILI9341_HandleTypeDef *hlcd, uint8_t data) {
    HAL_GPIO_WritePin(hlcd->DcPort, hlcd->DcPin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(hlcd->CsPort, hlcd->CsPin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(hlcd->hspi, &data, 1, 100);
    HAL_GPIO_WritePin(hlcd->CsPort, hlcd->CsPin, GPIO_PIN_SET);
}

// --- Public Functions ---

uint8_t ILI9341_Init(ILI9341_HandleTypeDef *hlcd, SPI_HandleTypeDef *hspi, 
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
    hlcd->Width = ILI9341_WIDTH;
    hlcd->Height = ILI9341_HEIGHT;
    hlcd->Rotation = 0;

    // Hardware Reset
    HAL_GPIO_WritePin(hlcd->CsPort, hlcd->CsPin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(hlcd->RstPort, hlcd->RstPin, GPIO_PIN_RESET);
    HAL_Delay(50);
    HAL_GPIO_WritePin(hlcd->RstPort, hlcd->RstPin, GPIO_PIN_SET);
    HAL_Delay(100);

    // Initialization Sequence
    ILI9341_WriteCommand(hlcd, 0xEF);
    ILI9341_WriteSmallData(hlcd, 0x03);
    ILI9341_WriteSmallData(hlcd, 0x80);
    ILI9341_WriteSmallData(hlcd, 0x02);

    ILI9341_WriteCommand(hlcd, 0xCF);
    ILI9341_WriteSmallData(hlcd, 0x00);
    ILI9341_WriteSmallData(hlcd, 0xC1);
    ILI9341_WriteSmallData(hlcd, 0x30);

    ILI9341_WriteCommand(hlcd, 0xED);
    ILI9341_WriteSmallData(hlcd, 0x64);
    ILI9341_WriteSmallData(hlcd, 0x03);
    ILI9341_WriteSmallData(hlcd, 0x12);
    ILI9341_WriteSmallData(hlcd, 0x81);

    ILI9341_WriteCommand(hlcd, 0xE8);
    ILI9341_WriteSmallData(hlcd, 0x85);
    ILI9341_WriteSmallData(hlcd, 0x00);
    ILI9341_WriteSmallData(hlcd, 0x78);

    ILI9341_WriteCommand(hlcd, 0xCB);
    ILI9341_WriteSmallData(hlcd, 0x39);
    ILI9341_WriteSmallData(hlcd, 0x2C);
    ILI9341_WriteSmallData(hlcd, 0x00);
    ILI9341_WriteSmallData(hlcd, 0x34);
    ILI9341_WriteSmallData(hlcd, 0x02);

    ILI9341_WriteCommand(hlcd, 0xF7);
    ILI9341_WriteSmallData(hlcd, 0x20);

    ILI9341_WriteCommand(hlcd, 0xEA);
    ILI9341_WriteSmallData(hlcd, 0x00);
    ILI9341_WriteSmallData(hlcd, 0x00);

    ILI9341_WriteCommand(hlcd, ILI9341_PWCTR1); // Power control
    ILI9341_WriteSmallData(hlcd, 0x23);

    ILI9341_WriteCommand(hlcd, ILI9341_PWCTR2); // Power control
    ILI9341_WriteSmallData(hlcd, 0x10);

    ILI9341_WriteCommand(hlcd, ILI9341_VMCTR1); // VCM control
    ILI9341_WriteSmallData(hlcd, 0x3e);
    ILI9341_WriteSmallData(hlcd, 0x28);

    ILI9341_WriteCommand(hlcd, ILI9341_VMCTR2); // VCM control2
    ILI9341_WriteSmallData(hlcd, 0x86);

    ILI9341_WriteCommand(hlcd, ILI9341_MADCTL); // Memory Access Control
    ILI9341_WriteSmallData(hlcd, 0x48);

    ILI9341_WriteCommand(hlcd, ILI9341_PIXFMT);
    ILI9341_WriteSmallData(hlcd, 0x55);

    ILI9341_WriteCommand(hlcd, ILI9341_FRMCTR1);
    ILI9341_WriteSmallData(hlcd, 0x00);
    ILI9341_WriteSmallData(hlcd, 0x18);

    ILI9341_WriteCommand(hlcd, ILI9341_DFUNCTR); // Display Function Control
    ILI9341_WriteSmallData(hlcd, 0x08);
    ILI9341_WriteSmallData(hlcd, 0x82);
    ILI9341_WriteSmallData(hlcd, 0x27);

    ILI9341_WriteCommand(hlcd, 0xF2); // 3Gamma Function Disable
    ILI9341_WriteSmallData(hlcd, 0x00);

    ILI9341_WriteCommand(hlcd, ILI9341_GAMMASET); // Gamma curve selected
    ILI9341_WriteSmallData(hlcd, 0x01);

    ILI9341_WriteCommand(hlcd, ILI9341_GMCTRP1); // Set Gamma
    ILI9341_WriteSmallData(hlcd, 0x0F);
    ILI9341_WriteSmallData(hlcd, 0x31);
    ILI9341_WriteSmallData(hlcd, 0x2B);
    ILI9341_WriteSmallData(hlcd, 0x0C);
    ILI9341_WriteSmallData(hlcd, 0x0E);
    ILI9341_WriteSmallData(hlcd, 0x08);
    ILI9341_WriteSmallData(hlcd, 0x4E);
    ILI9341_WriteSmallData(hlcd, 0xF1);
    ILI9341_WriteSmallData(hlcd, 0x37);
    ILI9341_WriteSmallData(hlcd, 0x07);
    ILI9341_WriteSmallData(hlcd, 0x10);
    ILI9341_WriteSmallData(hlcd, 0x03);
    ILI9341_WriteSmallData(hlcd, 0x0E);
    ILI9341_WriteSmallData(hlcd, 0x09);
    ILI9341_WriteSmallData(hlcd, 0x00);

    ILI9341_WriteCommand(hlcd, ILI9341_GMCTRN1);
    ILI9341_WriteSmallData(hlcd, 0x00);
    ILI9341_WriteSmallData(hlcd, 0x0E);
    ILI9341_WriteSmallData(hlcd, 0x14);
    ILI9341_WriteSmallData(hlcd, 0x03);
    ILI9341_WriteSmallData(hlcd, 0x11);
    ILI9341_WriteSmallData(hlcd, 0x07);
    ILI9341_WriteSmallData(hlcd, 0x31);
    ILI9341_WriteSmallData(hlcd, 0xC1);
    ILI9341_WriteSmallData(hlcd, 0x48);
    ILI9341_WriteSmallData(hlcd, 0x08);
    ILI9341_WriteSmallData(hlcd, 0x0F);
    ILI9341_WriteSmallData(hlcd, 0x0C);
    ILI9341_WriteSmallData(hlcd, 0x31);
    ILI9341_WriteSmallData(hlcd, 0x36);
    ILI9341_WriteSmallData(hlcd, 0x0F);

    ILI9341_WriteCommand(hlcd, ILI9341_SLPOUT); // Exit Sleep
    HAL_Delay(120);
    
    ILI9341_WriteCommand(hlcd, ILI9341_DISPON); // Turn on Display
    
    // Set Rotation 0 default
    ILI9341_SetRotation(hlcd, 0);

    if (hlcd->BlkPort != NULL) {
        HAL_GPIO_WritePin(hlcd->BlkPort, hlcd->BlkPin, GPIO_PIN_SET);
    }

    ILI9341_FillScreen(hlcd, ILI9341_BLACK);
    return 0;
}

void ILI9341_SetRotation(ILI9341_HandleTypeDef *hlcd, uint8_t m) {
    ILI9341_WriteCommand(hlcd, ILI9341_MADCTL);
    uint8_t rotation = m % 4; // can be 0-3
    hlcd->Rotation = rotation;
    
    switch (rotation) {
        case 0:
            ILI9341_WriteSmallData(hlcd, MADCTL_MX | MADCTL_BGR);
            hlcd->Width  = ILI9341_WIDTH;
            hlcd->Height = ILI9341_HEIGHT;
            break;
        case 1:
            ILI9341_WriteSmallData(hlcd, MADCTL_MV | MADCTL_BGR);
            hlcd->Width  = ILI9341_HEIGHT;
            hlcd->Height = ILI9341_WIDTH;
            break;
        case 2:
            ILI9341_WriteSmallData(hlcd, MADCTL_MY | MADCTL_BGR);
            hlcd->Width  = ILI9341_WIDTH;
            hlcd->Height = ILI9341_HEIGHT;
            break;
        case 3:
            ILI9341_WriteSmallData(hlcd, MADCTL_MX | MADCTL_MY | MADCTL_MV | MADCTL_BGR);
            hlcd->Width  = ILI9341_HEIGHT;
            hlcd->Height = ILI9341_WIDTH;
            break;
    }
}

static void ILI9341_SetAddressWindow(ILI9341_HandleTypeDef *hlcd, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    uint8_t data[4];
    
    ILI9341_WriteCommand(hlcd, ILI9341_CASET);
    data[0] = (x0 >> 8) & 0xFF;
    data[1] = x0 & 0xFF;
    data[2] = (x1 >> 8) & 0xFF;
    data[3] = x1 & 0xFF;
    ILI9341_WriteData(hlcd, data, 4);

    ILI9341_WriteCommand(hlcd, ILI9341_PASET);
    data[0] = (y0 >> 8) & 0xFF;
    data[1] = y0 & 0xFF;
    data[2] = (y1 >> 8) & 0xFF;
    data[3] = y1 & 0xFF;
    ILI9341_WriteData(hlcd, data, 4);

    ILI9341_WriteCommand(hlcd, ILI9341_RAMWR);
}

void ILI9341_FillRect(ILI9341_HandleTypeDef *hlcd, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    if ((x >= hlcd->Width) || (y >= hlcd->Height)) return;
    if ((x + w - 1) >= hlcd->Width) w = hlcd->Width - x;
    if ((y + h - 1) >= hlcd->Height) h = hlcd->Height - y;

    ILI9341_SetAddressWindow(hlcd, x, y, x + w - 1, y + h - 1);

    uint8_t color_buff[512]; 
    uint8_t hi = (color >> 8) & 0xFF;
    uint8_t lo = color & 0xFF;
    
    // Fill buffer efficiently
    uint16_t buffer_pixels = sizeof(color_buff) / 2;
    for (uint16_t i = 0; i < buffer_pixels; i++) {
        color_buff[i*2] = hi;
        color_buff[i*2+1] = lo;
    }

    uint32_t total_pixels = w * h;
    while (total_pixels > 0) {
        uint32_t chunk_pixels = (total_pixels > buffer_pixels) ? buffer_pixels : total_pixels;
        ILI9341_WriteData(hlcd, color_buff, chunk_pixels * 2);
        total_pixels -= chunk_pixels;
    }
}

void ILI9341_FillScreen(ILI9341_HandleTypeDef *hlcd, uint16_t color) {
    ILI9341_FillRect(hlcd, 0, 0, hlcd->Width, hlcd->Height, color);
}

void ILI9341_DrawPixel(ILI9341_HandleTypeDef *hlcd, uint16_t x, uint16_t y, uint16_t color) {
    if ((x >= hlcd->Width) || (y >= hlcd->Height)) return;

    ILI9341_SetAddressWindow(hlcd, x, y, x, y);
    uint8_t data[2] = {(color >> 8) & 0xFF, color & 0xFF};
    ILI9341_WriteData(hlcd, data, 2);
}

void ILI9341_DrawImage(ILI9341_HandleTypeDef *hlcd, uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t* data) {
    if ((x >= hlcd->Width) || (y >= hlcd->Height)) return;
    if ((x + w - 1) >= hlcd->Width) return;
    if ((y + h - 1) >= hlcd->Height) return;

    ILI9341_SetAddressWindow(hlcd, x, y, x + w - 1, y + h - 1);
    
    uint32_t total_pixels = w * h;
    uint32_t i = 0;
    uint8_t line_buff[256]; 
    uint16_t chunk_size = 128;
    
    while (i < total_pixels) {
        uint32_t current_chunk = (total_pixels - i > chunk_size) ? chunk_size : (total_pixels - i);
        for (uint32_t k = 0; k < current_chunk; k++) {
            line_buff[k*2]     = (data[i+k] >> 8) & 0xFF;
            line_buff[k*2+1]   = data[i+k] & 0xFF;
        }
        ILI9341_WriteData(hlcd, line_buff, current_chunk * 2);
        i += current_chunk;
    }
}

void ILI9341_InvertColors(ILI9341_HandleTypeDef *hlcd, uint8_t invert) {
    ILI9341_WriteCommand(hlcd, invert ? ILI9341_INVON : ILI9341_INVOFF);
}
