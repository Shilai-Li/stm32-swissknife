#ifndef __BH1750_H
#define __BH1750_H

#include "main.h"
#include "soft_i2c.h"

// BH1750 Address
// If ADDR pin is LOW:  0x23 (7-bit) -> 0x46 (Write), 0x47 (Read)
// If ADDR pin is HIGH: 0x5C (7-bit) -> 0xB8 (Write), 0xB9 (Read)
#define BH1750_ADDR_L  0x46
#define BH1750_ADDR_H  0xB8

/* Forward callback declaration */
typedef struct BH1750_Handle_s BH1750_Handle_t;
typedef void (*BH1750_ErrorCallback)(BH1750_Handle_t *dev);

struct BH1750_Handle_s {
    Soft_I2C_HandleTypeDef *i2c;
    uint8_t address; // 8-bit write address
    uint8_t mode;
    
    /* Stats */
    volatile uint32_t error_cnt;
    volatile uint32_t success_cnt;
    
    /* Callbacks */
    BH1750_ErrorCallback error_cb;
};

void BH1750_SetErrorCallback(BH1750_Handle_t *dev, BH1750_ErrorCallback cb);

/**
 * @brief Initialize BH1750
 * @param dev Device Handle
 * @param i2c SoftI2C Handle
 * @param addr_pin_state 0 = ADDR Low (0x46), 1 = ADDR High (0xB8)
 */
void BH1750_Init(BH1750_Handle_t *dev, Soft_I2C_HandleTypeDef *i2c, uint8_t addr_pin_state);

/**
 * @brief Start Measurement (Power On + Continuous High Res Mode)
 */
void BH1750_Start(BH1750_Handle_t *dev);

/**
 * @brief Read Light Level in Lux
 * @return Lux value, or -1.0f on error
 */
float BH1750_ReadLux(BH1750_Handle_t *dev);

#endif
