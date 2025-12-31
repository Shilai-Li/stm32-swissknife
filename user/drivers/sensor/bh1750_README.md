# BH1750 Light Intensity Sensor Driver

I2C digital ambient light sensor (0.5-65535 lux) with error tracking.

## Features
- **High Resolution**: 0.5 lux resolution mode available
- **Error Statistics**: Track I2C failures and success count
- **Soft I2C Based**: Uses software I2C implementation
- **Callback Support**: Error notification

## Hardware Requirements

### I2C Configuration
- **Software I2C**: See `soft_i2c.h` for GPIO setup
- **Address Pin (ADDR)**:
  - ADDR = Low → 0x23 (7-bit)
  - ADDR = High → 0x5C (7-bit)
- **Pull-ups**: 4.7k on SCL/SDA required

## Usage

```c
#include "bh1750.h"
#include "soft_i2c.h"

Soft_I2C_HandleTypeDef i2c;
BH1750_Handle_t light_sensor;

void app_init(void) {
    // Init software I2C first
    Soft_I2C_Init(&i2c, GPIOB, GPIO_PIN_6,  // SCL
                            GPIOB, GPIO_PIN_7); // SDA
    
    // Init BH1750 (ADDR pin = Low)
    BH1750_Init(&light_sensor, &i2c, 0);
    
    // Start continuous measurement
    BH1750_Start(&light_sensor);
}

void read_light(void) {
    float lux = BH1750_ReadLux(&light_sensor);
    
    if (lux >= 0) {
        printf("Light: %.1f lux\r\n", lux);
    } else {
        printf("I2C Error\r\n");
    }
}
```

## API Reference

### Initialization
```c
void BH1750_Init(BH1750_Handle_t *dev, Soft_I2C_HandleTypeDef *i2c,
                 uint8_t addr_pin_state);
```
- `addr_pin_state`: 0 = ADDR Low, 1 = ADDR High

### Measurement
```c
void BH1750_Start(BH1750_Handle_t *dev);
float BH1750_ReadLux(BH1750_Handle_t *dev);
```
- `BH1750_Start`: Power on + enter continuous mode
- `BH1750_ReadLux`: Returns lux value or `-1.0f` on error

### Error Handling
```c
void BH1750_SetErrorCallback(BH1750_Handle_t *dev,
                             void (*callback)(BH1750_Handle_t*));

// Statistics:
// dev->error_cnt
// dev->success_cnt
```

## Measurement Modes
Current driver uses **Continuous H-Res Mode** (1 lux resolution, 120ms/sample).

Available modes (modify source if needed):
- H-Res Mode: 1 lux, 120ms
- H-Res Mode2: 0.5 lux, 120ms
- L-Res Mode: 4 lux, 16ms (faster but less precise)

## Typical Lux Values
- Direct sunlight: 50,000-100,000 lux
- Overcast day: 1,000 lux
- Office lighting: 300-500 lux
- Living room: 50-150 lux
- Moonlight: 0.1 lux

## Troubleshooting

### Always Returns -1.0f
1. Check I2C pull-up resistors (4.7k)
2. Verify BH1750 ADDR pin state matches init parameter
3. Use I2C scanner to detect device address
4. Check VCC (2.4V-3.6V)

### Saturated (65535 lux)
- Sensor saturated, reduce light or use filter

### Unstable Readings
- Add 100nF capacitor near VCC/GND
- Ensure stable I2C timing (soft I2C delay tuning)

## Test Program
See `user/drivers/tests/bh1750_tests.c`
