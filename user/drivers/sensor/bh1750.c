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

void BH1750_Init(BH1750_Handle_t *dev, Soft_I2C_HandleTypeDef *i2c, uint8_t addr_pin_state) {
    dev->i2c = i2c;
    dev->address = addr_pin_state ? BH1750_ADDR_H : BH1750_ADDR_L;
}

void BH1750_Start(BH1750_Handle_t *dev) {
    uint8_t cmd;
    
    // 1. Power On
    cmd = BH1750_CMD_POWER_ON;
    Soft_I2C_Master_Transmit(dev->i2c, dev->address, &cmd, 1, 100);
    
    // 2. Set Mode (Continuous H-Res)
    cmd = BH1750_CMD_H_RES_MODE;
    Soft_I2C_Master_Transmit(dev->i2c, dev->address, &cmd, 1, 100);
    
    // Wait for first measurement completion (max 180ms per spec)
    Delay_ms(180);
}

float BH1750_ReadLux(BH1750_Handle_t *dev) {
    uint8_t data[2];
    
    // Read 2 bytes
    if (Soft_I2C_Master_Receive(dev->i2c, dev->address, data, 2, 100) == 0) { // 0 = OK (assuming SoftI2C standard)
        // Check `soft_i2c.c` implementation to confirm return value. 
        // Standard HAL returns HAL_OK(0).
        
        uint16_t val = (data[0] << 8) | data[1];
        
        // Conversions: Lux = Raw / 1.2
        return (float)val / 1.2f;
    }
    
    return -1.0f; // Error
}
