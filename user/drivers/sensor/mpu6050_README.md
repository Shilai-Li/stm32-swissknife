# MPU6050 6-Axis Gyroscope & Accelerometer Driver

I2C-based inertial measurement unit (IMU) driver with comprehensive error tracking.

## Features
- **6-DOF**: 3-axis gyroscope + 3-axis accelerometer
- **Configurable Ranges**: ±2/4/8/16g accel, ±250/500/1000/2000 dps gyro
- **Digital Low-Pass Filter**: Reduce noise (5-260Hz configurable)
- **Error Statistics**: I2C failure tracking and callbacks
- **Flexible Output**: Raw counts or converted physical units

## Hardware Requirements

### I2C Configuration (CubeMX)
- **Mode**: I2C Master
- **Speed**: Standard (100kHz) or Fast (400kHz)
- **Pull-ups**: 4.7kΩ on SCL/SDA (often built-in on modules)

### MPU6050 Address
- **AD0 = Low**: 0x68 (7-bit)
- **AD0 = High**: 0x69 (7-bit)

## Usage

```c
#include "mpu6050.h"

I2C_HandleTypeDef hi2c1;
MPU6050_Handle_t imu;

void app_init(void) {
    HAL_StatusTypeDef status = MPU6050_Init(&imu, &hi2c1, MPU6050_I2C_ADDR_LOW);
    
    if (status == HAL_OK) {
        printf("MPU6050 initialized\r\n");
    }
    
    MPU6050_SetAccelRange(&imu, MPU6050_ACCEL_RANGE_4G);
    MPU6050_SetGyroRange(&imu, MPU6050_GYRO_RANGE_500DPS);
}

void read_imu(void) {
    MPU6050_Data_t data;
    
    if (MPU6050_Read(&imu, &data) == HAL_OK) {
        printf("Accel: %.2f, %.2f, %.2f g\r\n",
               data.accel_x_g, data.accel_y_g, data.accel_z_g);
        printf("Gyro: %.2f, %.2f, %.2f dps\r\n",
               data.gyro_x_dps, data.gyro_y_dps, data.gyro_z_dps);
        printf("Temp: %.1f°C\r\n", data.temp_c);
    }
}
```

## API Reference

### Initialization
```c
HAL_StatusTypeDef MPU6050_Init(MPU6050_Handle_t *dev, 
                               I2C_HandleTypeDef *hi2c, 
                               uint8_t addr_7bit);
```

### Configuration
```c
HAL_StatusTypeDef MPU6050_SetAccelRange(MPU6050_Handle_t *dev, 
                                        MPU6050_AccelRange_t range);
HAL_StatusTypeDef MPU6050_SetGyroRange(MPU6050_Handle_t *dev, 
                                       MPU6050_GyroRange_t range);
HAL_StatusTypeDef MPU6050_SetDLPF(MPU6050_Handle_t *dev, 
                                  MPU6050_DLPF_t dlpf);
HAL_StatusTypeDef MPU6050_SetSampleRateDivider(MPU6050_Handle_t *dev, 
                                               uint8_t divider);
```

### Reading Data
```c
HAL_StatusTypeDef MPU6050_ReadRaw(MPU6050_Handle_t *dev, 
                                  MPU6050_RawData_t *out);
HAL_StatusTypeDef MPU6050_Read(MPU6050_Handle_t *dev, 
                               MPU6050_Data_t *out);
```

### Error Handling
```c
void MPU6050_SetErrorCallback(MPU6050_Handle_t *dev,
                              void (*cb)(MPU6050_Handle_t*));

// Statistics:
// dev->error_cnt
// dev->i2c_error_cnt  
// dev->successful_read_cnt
```

## Configuration Options

### Accelerometer Range
- `MPU6050_ACCEL_RANGE_2G` → ±2g (most sensitive)
- `MPU6050_ACCEL_RANGE_4G` → ±4g
- `MPU6050_ACCEL_RANGE_8G` → ±8g
- `MPU6050_ACCEL_RANGE_16G` → ±16g (least sensitive)

### Gyroscope Range
- `MPU6050_GYRO_RANGE_250DPS` → ±250°/s (most sensitive)
- `MPU6050_GYRO_RANGE_500DPS` → ±500°/s
- `MPU6050_GYRO_RANGE_1000DPS` → ±1000°/s
- `MPU6050_GYRO_RANGE_2000DPS` → ±2000°/s (least sensitive)

### Digital Low-Pass Filter
```c
MPU6050_SetDLPF(&imu, MPU6050_DLPF_42HZ);  // Good balance
```
Options: 5, 10, 20, 42, 94, 184, 260 Hz

### Sample Rate
```c
// Sample Rate = 1kHz / (1 + divider)
MPU6050_SetSampleRateDivider(&imu, 9);  // 100 Hz
```

## Coordinate System
- **X-axis**: Forward (silkscreen direction)
- **Y-axis**: Right
- **Z-axis**: Down (gravity = -1g when flat)

## Calibration

### Zero-Offset Calibration
Place IMU stationary on flat surface:
```c
// Read 100 samples
for (int i = 0; i < 100; i++) {
    MPU6050_Read(&imu, &data);
    offset_x += data.gyro_x_dps;
    // ... accumulate all axes
    HAL_Delay(10);
}
// Calculate average and subtract in application
offset_x /= 100.0f;
```

## Troubleshooting

### Init Returns HAL_ERROR
1. Check I2C pull-up resistors (4.7kΩ)
2. Verify module VCC (3.3V or 5V depending on module)
3. Run I2C scanner to detect address
4. Check WHO_AM_I register (should be 0x68 or 0x69)

### Noisy Readings
1. Enable DLPF: `MPU6050_SetDLPF(&imu, MPU6050_DLPF_20HZ)`
2. Add 100nF capacitor near VCC/GND
3. Use shielded cables for long wires

### Gyro Drift
- Normal behavior; implement complementary filter:
  ```c
  angle = 0.98 * (angle + gyro_dps * dt) + 0.02 * accel_angle;
  ```

## Advanced Features (Future)
- DMP (Digital Motion Processor) integration
- Interrupt-driven data ready signal
- FIFO buffer management

## Test Program
See `user/drivers/tests/mpu6050_tests.c`
