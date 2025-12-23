/**
 * @file st7735.c
 * @brief ST7735 SPI TFT LCD Driver Implementation
 * @author Standard Implementation
 * @date 2024
 */

#include "st7735.h"
#include "delay.h"

// --- ST7735 Command Definitions ---
#define ST7735_NOP     0x00
#define ST7735_SWRESET 0x01
#define ST7735_RDDID   0x04
#define ST7735_RDDST   0x09

#define ST7735_SLPIN   0x10
#define ST7735_SLPOUT  0x11
#define ST7735_PTLON   0x12
#define ST7735_NORON   0x13

#define ST7735_INVOFF  0x20
#define ST7735_INVON   0x21
#define ST7735_GAMSET  0x26
#define ST7735_DISPOFF 0x28
#define ST7735_DISPON  0x29
#define ST7735_CASET   0x2A
#define ST7735_RASET   0x2B
#define ST7735_RAMWR   0x2C
#define ST7735_RAMRD   0x2E

#define ST7735_PTLAR   0x30
#define ST7735_MADCTL  0x36
#define ST7735_COLMOD  0x3A

#define ST7735_FRMCTR1 0xB1
#define ST7735_FRMCTR2 0xB2
#define ST7735_FRMCTR3 0xB3
#define ST7735_INVCTR  0xB4
#define ST7735_DISSET5 0xB6

#define ST7735_PWCTR1  0xC0
#define ST7735_PWCTR2  0xC1
#define ST7735_PWCTR3  0xC2
#define ST7735_PWCTR4  0xC3
#define ST7735_PWCTR5  0xC4
#define ST7735_VMCTR1  0xC5

#define ST7735_RDID1   0xDA
#define ST7735_RDID2   0xDB
#define ST7735_RDID3   0xDC
#define ST7735_RDID4   0xDD

#define ST7735_GMCTRP1 0xE0
#define ST7735_GMCTRN1 0xE1

#define MADCTL_MY 0x80
#define MADCTL_MX 0x40
#define MADCTL_MV 0x20
#define MADCTL_ML 0x10
#define MADCTL_RGB 0x00
#define MADCTL_BGR 0x08
#define MADCTL_MH 0x04

// --- Private Functions ---

static void ST7735_WriteCommand(ST7735_HandleTypeDef *hlcd, uint8_t cmd) {
    HAL_GPIO_WritePin(hlcd->DcPort, hlcd->DcPin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(hlcd->CsPort, hlcd->CsPin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(hlcd->hspi, &cmd, 1, 100);
    HAL_GPIO_WritePin(hlcd->CsPort, hlcd->CsPin, GPIO_PIN_SET);
}

static void ST7735_WriteData(ST7735_HandleTypeDef *hlcd, uint8_t* buff, size_t buff_size) {
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

static void ST7735_WriteSmallData(ST7735_HandleTypeDef *hlcd, uint8_t data) {
    HAL_GPIO_WritePin(hlcd->DcPort, hlcd->DcPin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(hlcd->CsPort, hlcd->CsPin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(hlcd->hspi, &data, 1, 100);
    HAL_GPIO_WritePin(hlcd->CsPort, hlcd->CsPin, GPIO_PIN_SET);
}

// --- Public Functions ---

uint8_t ST7735_Init(ST7735_HandleTypeDef *hlcd, SPI_HandleTypeDef *hspi, 
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
    
    // Default 1.8" settings
    hlcd->Width = ST7735_WIDTH;
    hlcd->Height = ST7735_HEIGHT;
    hlcd->XOffset = ST7735_X_OFFSET;
    hlcd->YOffset = ST7735_Y_OFFSET;
    hlcd->Rotation = 0;

    // Hard Reset
    HAL_GPIO_WritePin(hlcd->CsPort, hlcd->CsPin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(hlcd->RstPort, hlcd->RstPin, GPIO_PIN_RESET);
    HAL_Delay(50);
    HAL_GPIO_WritePin(hlcd->RstPort, hlcd->RstPin, GPIO_PIN_SET);
    HAL_Delay(50);

    // Init Sequence for ST7735R (Most common) ("Red Tab")
    ST7735_WriteCommand(hlcd, ST7735_SWRESET);
    HAL_Delay(150);

    ST7735_WriteCommand(hlcd, ST7735_SLPOUT);
    HAL_Delay(255);

    ST7735_WriteCommand(hlcd, ST7735_FRMCTR1);
    ST7735_WriteSmallData(hlcd, 0x01);
    ST7735_WriteSmallData(hlcd, 0x2C);
    ST7735_WriteSmallData(hlcd, 0x2D);

    ST7735_WriteCommand(hlcd, ST7735_FRMCTR2);
    ST7735_WriteSmallData(hlcd, 0x01);
    ST7735_WriteSmallData(hlcd, 0x2C);
    ST7735_WriteSmallData(hlcd, 0x2D);

    ST7735_WriteCommand(hlcd, ST7735_FRMCTR3);
    ST7735_WriteSmallData(hlcd, 0x01);
    ST7735_WriteSmallData(hlcd, 0x2C);
    ST7735_WriteSmallData(hlcd, 0x2D);
    ST7735_WriteSmallData(hlcd, 0x01);
    ST7735_WriteSmallData(hlcd, 0x2C);
    ST7735_WriteSmallData(hlcd, 0x2D);

    ST7735_WriteCommand(hlcd, ST7735_INVCTR);
    ST7735_WriteSmallData(hlcd, 0x07);

    ST7735_WriteCommand(hlcd, ST7735_PWCTR1);
    ST7735_WriteSmallData(hlcd, 0xA2);
    ST7735_WriteSmallData(hlcd, 0x02);
    ST7735_WriteSmallData(hlcd, 0x84);

    ST7735_WriteCommand(hlcd, ST7735_PWCTR2);
    ST7735_WriteSmallData(hlcd, 0xC5);

    ST7735_WriteCommand(hlcd, ST7735_PWCTR3);
    ST7735_WriteSmallData(hlcd, 0x0A);
    ST7735_WriteSmallData(hlcd, 0x00);

    ST7735_WriteCommand(hlcd, ST7735_PWCTR4);
    ST7735_WriteSmallData(hlcd, 0x8A);
    ST7735_WriteSmallData(hlcd, 0x2A);

    ST7735_WriteCommand(hlcd, ST7735_PWCTR5);
    ST7735_WriteSmallData(hlcd, 0x8A);
    ST7735_WriteSmallData(hlcd, 0xEE);

    ST7735_WriteCommand(hlcd, ST7735_VMCTR1);
    ST7735_WriteSmallData(hlcd, 0x0E);

    ST7735_WriteCommand(hlcd, ST7735_INVOFF);

    ST7735_WriteCommand(hlcd, ST7735_MADCTL);
    ST7735_WriteSmallData(hlcd, MADCTL_MX | MADCTL_MY | MADCTL_BGR); // Default rotation

    ST7735_WriteCommand(hlcd, ST7735_COLMOD);
    ST7735_WriteSmallData(hlcd, 0x05);

    ST7735_WriteCommand(hlcd, ST7735_GMCTRP1);
    ST7735_WriteSmallData(hlcd, 0x02);
    ST7735_WriteSmallData(hlcd, 0x1c);
    ST7735_WriteSmallData(hlcd, 0x07);
    ST7735_WriteSmallData(hlcd, 0x12);
    ST7735_WriteSmallData(hlcd, 0x37);
    ST7735_WriteSmallData(hlcd, 0x32);
    ST7735_WriteSmallData(hlcd, 0x29);
    ST7735_WriteSmallData(hlcd, 0x2d);
    ST7735_WriteSmallData(hlcd, 0x29);
    ST7735_WriteSmallData(hlcd, 0x25);
    ST7735_WriteSmallData(hlcd, 0x2B);
    ST7735_WriteSmallData(hlcd, 0x39);
    ST7735_WriteSmallData(hlcd, 0x00);
    ST7735_WriteSmallData(hlcd, 0x01);
    ST7735_WriteSmallData(hlcd, 0x03);
    ST7735_WriteSmallData(hlcd, 0x10);

    ST7735_WriteCommand(hlcd, ST7735_GMCTRN1);
    ST7735_WriteSmallData(hlcd, 0x03);
    ST7735_WriteSmallData(hlcd, 0x1d);
    ST7735_WriteSmallData(hlcd, 0x07);
    ST7735_WriteSmallData(hlcd, 0x06);
    ST7735_WriteSmallData(hlcd, 0x2E);
    ST7735_WriteSmallData(hlcd, 0x2C);
    ST7735_WriteSmallData(hlcd, 0x29);
    ST7735_WriteSmallData(hlcd, 0x2D);
    ST7735_WriteSmallData(hlcd, 0x2E);
    ST7735_WriteSmallData(hlcd, 0x2E);
    ST7735_WriteSmallData(hlcd, 0x37);
    ST7735_WriteSmallData(hlcd, 0x3F);
    ST7735_WriteSmallData(hlcd, 0x00);
    ST7735_WriteSmallData(hlcd, 0x00);
    ST7735_WriteSmallData(hlcd, 0x02);
    ST7735_WriteSmallData(hlcd, 0x10);

    ST7735_WriteCommand(hlcd, ST7735_NORON);
    HAL_Delay(10);
    
    ST7735_WriteCommand(hlcd, ST7735_DISPON);
    HAL_Delay(100);

    if (hlcd->BlkPort != NULL) {
        HAL_GPIO_WritePin(hlcd->BlkPort, hlcd->BlkPin, GPIO_PIN_SET);
    }
    
    ST7735_FillScreen(hlcd, ST7735_BLACK);
    return 0;
}

void ST7735_SetType(ST7735_HandleTypeDef *hlcd, uint16_t x_off, uint16_t y_off, uint16_t w, uint16_t h) {
    hlcd->XOffset = x_off;
    hlcd->YOffset = y_off;
    hlcd->Width = w;
    hlcd->Height = h;
}

void ST7735_SetRotation(ST7735_HandleTypeDef *hlcd, uint8_t m) {
    ST7735_WriteCommand(hlcd, ST7735_MADCTL);
    uint8_t rotation = m % 4; 
    hlcd->Rotation = rotation;
    
    switch (rotation) {
        case 0:
            ST7735_WriteSmallData(hlcd, MADCTL_MX | MADCTL_MY | MADCTL_BGR);
            // Re-apply offsets if swapping dimensions is needed, but for simple 0-3 usually we swap W/H logic in SW or here.
            // ST7735 offsets often relative to physical RAM, so they change with rotation.
            // Simplified handling:
            break;
        case 1:
            ST7735_WriteSmallData(hlcd, MADCTL_MY | MADCTL_MV | MADCTL_BGR);
            break;
        case 2:
            ST7735_WriteSmallData(hlcd, MADCTL_BGR);
            break;
        case 3:
            ST7735_WriteSmallData(hlcd, MADCTL_MX | MADCTL_MV | MADCTL_BGR);
            break;
    }
}

static void ST7735_SetAddressWindow(ST7735_HandleTypeDef *hlcd, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    uint8_t data[4];
    uint16_t x_start = x0 + hlcd->XOffset;
    uint16_t x_end   = x1 + hlcd->XOffset;
    uint16_t y_start = y0 + hlcd->YOffset;
    uint16_t y_end   = y1 + hlcd->YOffset;

    ST7735_WriteCommand(hlcd, ST7735_CASET);
    data[0] = (x_start >> 8) & 0xFF;
    data[1] = x_start & 0xFF;
    data[2] = (x_end >> 8) & 0xFF;
    data[3] = x_end & 0xFF;
    ST7735_WriteData(hlcd, data, 4);

    ST7735_WriteCommand(hlcd, ST7735_RASET);
    data[0] = (y_start >> 8) & 0xFF;
    data[1] = y_start & 0xFF;
    data[2] = (y_end >> 8) & 0xFF;
    data[3] = y_end & 0xFF;
    ST7735_WriteData(hlcd, data, 4);

    ST7735_WriteCommand(hlcd, ST7735_RAMWR);
}

void ST7735_FillRect(ST7735_HandleTypeDef *hlcd, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    if ((x >= hlcd->Width) || (y >= hlcd->Height)) return;
    if ((x + w - 1) >= hlcd->Width) w = hlcd->Width - x;
    if ((y + h - 1) >= hlcd->Height) h = hlcd->Height - y;

    ST7735_SetAddressWindow(hlcd, x, y, x + w - 1, y + h - 1);

    uint8_t color_buff[256]; 
    uint8_t hi = (color >> 8) & 0xFF;
    uint8_t lo = color & 0xFF;
    
    // Fill buffer
    uint16_t buffer_pixels = sizeof(color_buff) / 2;
    for (uint16_t i = 0; i < buffer_pixels; i++) {
        color_buff[i*2] = hi;
        color_buff[i*2+1] = lo;
    }

    uint32_t total_pixels = w * h;
    while (total_pixels > 0) {
        uint32_t chunk_pixels = (total_pixels > buffer_pixels) ? buffer_pixels : total_pixels;
        ST7735_WriteData(hlcd, color_buff, chunk_pixels * 2);
        total_pixels -= chunk_pixels;
    }
}

void ST7735_FillScreen(ST7735_HandleTypeDef *hlcd, uint16_t color) {
    ST7735_FillRect(hlcd, 0, 0, hlcd->Width, hlcd->Height, color);
}

void ST7735_DrawPixel(ST7735_HandleTypeDef *hlcd, uint16_t x, uint16_t y, uint16_t color) {
    if ((x >= hlcd->Width) || (y >= hlcd->Height)) return;

    ST7735_SetAddressWindow(hlcd, x, y, x, y);
    uint8_t data[2] = {(color >> 8) & 0xFF, color & 0xFF};
    ST7735_WriteData(hlcd, data, 2);
}

void ST7735_DrawImage(ST7735_HandleTypeDef *hlcd, uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t* data) {
    if ((x >= hlcd->Width) || (y >= hlcd->Height)) return;
    ST7735_SetAddressWindow(hlcd, x, y, x + w - 1, y + h - 1);
    
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
        ST7735_WriteData(hlcd, line_buff, current_chunk * 2);
        i += current_chunk;
    }
}

void ST7735_InvertColors(ST7735_HandleTypeDef *hlcd, uint8_t invert) {
    ST7735_WriteCommand(hlcd, invert ? ST7735_INVON : ST7735_INVOFF);
}
