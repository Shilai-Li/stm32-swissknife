#ifndef __MPU6050_DRIVER_H__
#define __MPU6050_DRIVER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

#define MPU6050_I2C_ADDR_LOW   (0x68U)
#define MPU6050_I2C_ADDR_HIGH  (0x69U)

typedef enum {
    MPU6050_ACCEL_RANGE_2G  = 0,
    MPU6050_ACCEL_RANGE_4G  = 1,
    MPU6050_ACCEL_RANGE_8G  = 2,
    MPU6050_ACCEL_RANGE_16G = 3
} MPU6050_AccelRange_t;

typedef enum {
    MPU6050_GYRO_RANGE_250DPS  = 0,
    MPU6050_GYRO_RANGE_500DPS  = 1,
    MPU6050_GYRO_RANGE_1000DPS = 2,
    MPU6050_GYRO_RANGE_2000DPS = 3
} MPU6050_GyroRange_t;

typedef enum {
    MPU6050_DLPF_260HZ = 0,
    MPU6050_DLPF_184HZ = 1,
    MPU6050_DLPF_94HZ  = 2,
    MPU6050_DLPF_42HZ  = 3,
    MPU6050_DLPF_20HZ  = 4,
    MPU6050_DLPF_10HZ  = 5,
    MPU6050_DLPF_5HZ   = 6
} MPU6050_DLPF_t;

typedef struct {
    int16_t accel_x;
    int16_t accel_y;
    int16_t accel_z;
    int16_t temp;
    int16_t gyro_x;
    int16_t gyro_y;
    int16_t gyro_z;
} MPU6050_RawData_t;

typedef struct {
    float accel_x_g;
    float accel_y_g;
    float accel_z_g;
    float temp_c;
    float gyro_x_dps;
    float gyro_y_dps;
    float gyro_z_dps;
} MPU6050_Data_t;

/* Forward declaration for callback */
typedef struct MPU6050_Handle_s MPU6050_Handle_t;
typedef void (*MPU6050_ErrorCallback)(MPU6050_Handle_t *dev);

struct MPU6050_Handle_s {
    I2C_HandleTypeDef *hi2c;
    uint8_t addr_7bit;
    uint32_t timeout_ms;
    MPU6050_AccelRange_t accel_range;
    MPU6050_GyroRange_t gyro_range;
    float accel_lsb_per_g;
    float gyro_lsb_per_dps;
    
    /* Robustness Statistics */
    volatile uint32_t error_cnt;
    volatile uint32_t i2c_error_cnt;
    volatile uint32_t successful_read_cnt;
    
    /* Callbacks */
    MPU6050_ErrorCallback error_cb;
};

HAL_StatusTypeDef MPU6050_Init(MPU6050_Handle_t *dev, I2C_HandleTypeDef *hi2c, uint8_t addr_7bit);

HAL_StatusTypeDef MPU6050_Reset(MPU6050_Handle_t *dev);
HAL_StatusTypeDef MPU6050_Sleep(MPU6050_Handle_t *dev, bool enable);

HAL_StatusTypeDef MPU6050_ReadWhoAmI(MPU6050_Handle_t *dev, uint8_t *whoami);

HAL_StatusTypeDef MPU6050_SetTimeout(MPU6050_Handle_t *dev, uint32_t timeout_ms);
HAL_StatusTypeDef MPU6050_SetClockSource(MPU6050_Handle_t *dev, uint8_t clk_sel);
HAL_StatusTypeDef MPU6050_SetSampleRateDivider(MPU6050_Handle_t *dev, uint8_t divider);
HAL_StatusTypeDef MPU6050_SetDLPF(MPU6050_Handle_t *dev, MPU6050_DLPF_t dlpf);
HAL_StatusTypeDef MPU6050_SetAccelRange(MPU6050_Handle_t *dev, MPU6050_AccelRange_t range);
HAL_StatusTypeDef MPU6050_SetGyroRange(MPU6050_Handle_t *dev, MPU6050_GyroRange_t range);

HAL_StatusTypeDef MPU6050_ReadRaw(MPU6050_Handle_t *dev, MPU6050_RawData_t *out);
void MPU6050_Convert(const MPU6050_Handle_t *dev, const MPU6050_RawData_t *raw, MPU6050_Data_t *out);
HAL_StatusTypeDef MPU6050_Read(MPU6050_Handle_t *dev, MPU6050_Data_t *out);

#ifdef __cplusplus
}
#endif

#endif /* __MPU6050_DRIVER_H__ */
