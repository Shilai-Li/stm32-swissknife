/**
 * @file xpt2046.c
 * @brief XPT2046 Touch Screen Controller Driver Implementation
 * @author Standard Implementation
 * @date 2024
 */

#include "xpt2046.h"

// Command Definitions
// Bit 7: Start Bit (Always 1)
// Bit 6-4: Channel Select (X=001(0x10), Y=101(0x50), Z1=011, Z2=100)
// Bit 3: Mode (0=12bit, 1=8bit) -> Always 0 for 12bit
// Bit 2: SER/DFR (0=Differential, 1=SingleEnded) -> Always 0 (Diff is better)
// Bit 1-0: Power Mode (00=PenIRQ Enable, 01=No IRQ)
#define CMD_X_READ  0x90
#define CMD_Y_READ  0xD0

static uint16_t XPT2046_ReadRaw(XPT2046_HandleTypeDef *htouch, uint8_t cmd) {
    uint8_t tx[3] = {cmd, 0x00, 0x00};
    uint8_t rx[3] = {0, 0, 0};
    
    HAL_GPIO_WritePin(htouch->CsPort, htouch->CsPin, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive(htouch->hspi, tx, rx, 3, 100);
    HAL_GPIO_WritePin(htouch->CsPort, htouch->CsPin, GPIO_PIN_SET);
    
    // Result is in rx[1] and rx[2]
    // 12-bit result is (rx[1]<<4) | (rx[2]>>4)
    return ((rx[1] << 4) | (rx[2] >> 4)); 
}

// Median Filter: Read 5 times, sort, take middle
#define READ_TIMES 5
static uint16_t XPT2046_ReadFiltered(XPT2046_HandleTypeDef *htouch, uint8_t cmd) {
    uint16_t buffer[READ_TIMES];
    // Read
    for(int i=0; i<READ_TIMES; i++) {
        buffer[i] = XPT2046_ReadRaw(htouch, cmd);
    }
    // Sort (Bubble sort is fine for 5 items)
    for(int i=0; i<READ_TIMES-1; i++) {
        for(int j=0; j<READ_TIMES-i-1; j++) {
            if(buffer[j] > buffer[j+1]) {
                uint16_t temp = buffer[j];
                buffer[j] = buffer[j+1];
                buffer[j+1] = temp;
            }
        }
    }
    // Return median
    return buffer[READ_TIMES/2];
}

void XPT2046_Init(XPT2046_HandleTypeDef *htouch, SPI_HandleTypeDef *hspi, 
                  GPIO_TypeDef *cs_port, uint16_t cs_pin,
                  GPIO_TypeDef *irq_port, uint16_t irq_pin) 
{
    htouch->hspi = hspi;
    htouch->CsPort = cs_port; htouch->CsPin = cs_pin;
    htouch->IrqPort = irq_port; htouch->IrqPin = irq_pin;
    
    // Defaults
    htouch->width = XPT2046_WIDTH;
    htouch->height = XPT2046_HEIGHT;
    htouch->x_min = XPT2046_X_MIN; htouch->x_max = XPT2046_X_MAX;
    htouch->y_min = XPT2046_Y_MIN; htouch->y_max = XPT2046_Y_MAX;
    htouch->Rotation = 0;
    
    HAL_GPIO_WritePin(htouch->CsPort, htouch->CsPin, GPIO_PIN_SET);
}

void XPT2046_SetCalibration(XPT2046_HandleTypeDef *htouch, 
                            uint16_t width, uint16_t height,
                            uint16_t x_min, uint16_t x_max, 
                            uint16_t y_min, uint16_t y_max)
{
    htouch->width = width; htouch->height = height;
    htouch->x_min = x_min; htouch->x_max = x_max;
    htouch->y_min = y_min; htouch->y_max = y_max;
}

void XPT2046_SetRotation(XPT2046_HandleTypeDef *htouch, uint8_t rotation) {
    htouch->Rotation = rotation % 4;
}

uint8_t XPT2046_IsTouched(XPT2046_HandleTypeDef *htouch) {
    if (htouch->IrqPort != NULL) {
        return (HAL_GPIO_ReadPin(htouch->IrqPort, htouch->IrqPin) == GPIO_PIN_RESET);
    }
    // If no IRQ pin, we might need to query Z-axis (Pressure), but usually IRQ is mandatory for XPT2046 usage.
    // Return 0 if no IRQ pin configured to avoid false positives.
    return 0;
}

uint8_t XPT2046_GetCoordinates(XPT2046_HandleTypeDef *htouch, uint16_t *x, uint16_t *y) {
    if (!XPT2046_IsTouched(htouch)) return 0;
    
    uint16_t raw_x = XPT2046_ReadFiltered(htouch, CMD_X_READ);
    uint16_t raw_y = XPT2046_ReadFiltered(htouch, CMD_Y_READ);
    
    if (raw_x == 0 || raw_y == 0) return 0; // Invalid
    // XPT2046 Raw: 0-4095
    
    // Normalize to 0-1.0 float first
    float norm_x = (float)(raw_x - htouch->x_min) / (float)(htouch->x_max - htouch->x_min);
    float norm_y = (float)(raw_y - htouch->y_min) / (float)(htouch->y_max - htouch->y_min);
    
    // Clamp
    if(norm_x < 0) norm_x = 0; if(norm_x > 1) norm_x = 1;
    if(norm_y < 0) norm_y = 0; if(norm_y > 1) norm_y = 1;
    
    // Coordinate Mapping based on Rotation
    // Assuming standard panel: X corresponds to long side?, Y to short?
    // Usually XPT2046 coordinates are fixed relative to panel glass.
    // Need to swap/invert based on screen rotation.
    // This mapping depends highly on physical wiring.
    // Standard TFT Modules: Y is usually Long axis, X is Short axis? Or vice versa.
    // Below is a generic best-guess satisfying most ILI9341 modules.
    
    uint16_t out_x, out_y;
    
    switch(htouch->Rotation) {
        case 0: // Portrait
            out_x = norm_x * htouch->width;
            out_y = norm_y * htouch->height;
            break;
        case 1: // Landscape (90 deg)
            out_x = norm_y * htouch->width; // Swap X/Y axes logic often
            out_y = (1.0f - norm_x) * htouch->height;
            break;
        case 2: // Portrait Inverted (180 deg)
            out_x = (1.0f - norm_x) * htouch->width;
            out_y = (1.0f - norm_y) * htouch->height;
            break;
        case 3: // Landscape Inverted (270 deg)
            out_x = (1.0f - norm_y) * htouch->width;
            out_y = norm_x * htouch->height;
            break;
        default:
            out_x = 0; out_y = 0;
    }
    
    *x = out_x;
    *y = out_y;
    
    return 1;
}
