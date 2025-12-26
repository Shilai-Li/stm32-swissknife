#include "bh1750.h"
#include "delay.h"

// BH1750 Commands
#define BH1750_CMD_POWER_DOWN 0x00
#define BH1750_CMD_POWER_ON   0x01
#define BH1750_CMD_RESET      0x07

// Measurement Modes
#define BH1750_CMD_H_RES_MODE   0x10 // 1lx res, 120ms
#define BH1750_CMD_H_RES_MODE2  0x11 // 0.5lx res, 120ms
#define BH1750_CMD_L_RES_MODE   0x13 // 4lx res, 16ms

static void BH1750_HandleError(BH1750_Handle_t *dev) {
    if (dev) {
        dev->error_cnt++;
        if (dev->error_cb) {
            dev->error_cb(dev);
        }
    }
}

void BH1750_SetErrorCallback(BH1750_Handle_t *dev, BH1750_ErrorCallback cb) {
    if (dev) {
        dev->error_cb = cb;
    }
}

void BH1750_Init(BH1750_Handle_t *dev, Soft_I2C_HandleTypeDef *i2c, uint8_t addr_pin_state) {
    if (!dev) return;
    dev->i2c = i2c;
    dev->address = addr_pin_state ? BH1750_ADDR_H : BH1750_ADDR_L;
    
    dev->error_cnt = 0;
    dev->success_cnt = 0;
    dev->error_cb = NULL;
}

void BH1750_Start(BH1750_Handle_t *dev) {
    if (!dev || !dev->i2c) return;
    
    uint8_t cmd;
    
    // 1. Power On
    cmd = BH1750_CMD_POWER_ON;
    if (Soft_I2C_Master_Transmit(dev->i2c, dev->address, &cmd, 1, 100) != 0) {
        BH1750_HandleError(dev);
        return;
    }
    
    // 2. Set Mode (Continuous H-Res)
    cmd = BH1750_CMD_H_RES_MODE;
    if (Soft_I2C_Master_Transmit(dev->i2c, dev->address, &cmd, 1, 100) != 0) {
        BH1750_HandleError(dev);
        return;
    }
    
    // Wait for first measurement completion (max 180ms per spec)
    Delay_ms(180);
}

float BH1750_ReadLux(BH1750_Handle_t *dev) {
    if (!dev || !dev->i2c) return -1.0f;
    
    uint8_t data[2];
    
    // Read 2 bytes
    if (Soft_I2C_Master_Receive(dev->i2c, dev->address, data, 2, 100) == 0) { // 0 = OK
        
        uint16_t val = (data[0] << 8) | data[1];
        dev->success_cnt++;
        
        // Conversions: Lux = Raw / 1.2
        return (float)val / 1.2f;
    }
    
    BH1750_HandleError(dev);
    return -1.0f; // Error
}
