#include "mpu6050.h"

#include <string.h>

#define MPU6050_REG_SMPLRT_DIV      0x19U
#define MPU6050_REG_CONFIG          0x1AU
#define MPU6050_REG_GYRO_CONFIG     0x1BU
#define MPU6050_REG_ACCEL_CONFIG    0x1CU
#define MPU6050_REG_INT_ENABLE      0x38U
#define MPU6050_REG_ACCEL_XOUT_H    0x3BU
#define MPU6050_REG_PWR_MGMT_1      0x6BU
#define MPU6050_REG_WHO_AM_I        0x75U

#define MPU6050_PWR1_DEVICE_RESET   0x80U
#define MPU6050_PWR1_SLEEP          0x40U

static uint16_t mpu6050_addr8(uint8_t addr_7bit)
{
    return (uint16_t)(addr_7bit << 1);
}

static HAL_StatusTypeDef mpu6050_write_u8(MPU6050_Handle_t *dev, uint8_t reg, uint8_t val)
{
    if (dev == NULL || dev->hi2c == NULL) return HAL_ERROR;
    return HAL_I2C_Mem_Write(dev->hi2c, mpu6050_addr8(dev->addr_7bit), reg, I2C_MEMADD_SIZE_8BIT, &val, 1, dev->timeout_ms);
}

static HAL_StatusTypeDef mpu6050_read(MPU6050_Handle_t *dev, uint8_t reg, uint8_t *buf, uint16_t len)
{
    if (dev == NULL || dev->hi2c == NULL || buf == NULL || len == 0) return HAL_ERROR;
    return HAL_I2C_Mem_Read(dev->hi2c, mpu6050_addr8(dev->addr_7bit), reg, I2C_MEMADD_SIZE_8BIT, buf, len, dev->timeout_ms);
}

static HAL_StatusTypeDef mpu6050_update_bits(MPU6050_Handle_t *dev, uint8_t reg, uint8_t mask, uint8_t value)
{
    uint8_t v = 0;
    HAL_StatusTypeDef st = mpu6050_read(dev, reg, &v, 1);
    if (st != HAL_OK) return st;

    v = (uint8_t)((v & ~mask) | (value & mask));
    return mpu6050_write_u8(dev, reg, v);
}

static float mpu6050_accel_lsb_per_g(MPU6050_AccelRange_t range)
{
    switch (range) {
        case MPU6050_ACCEL_RANGE_2G:  return 16384.0f;
        case MPU6050_ACCEL_RANGE_4G:  return 8192.0f;
        case MPU6050_ACCEL_RANGE_8G:  return 4096.0f;
        case MPU6050_ACCEL_RANGE_16G: return 2048.0f;
        default: return 16384.0f;
    }
}

static float mpu6050_gyro_lsb_per_dps(MPU6050_GyroRange_t range)
{
    switch (range) {
        case MPU6050_GYRO_RANGE_250DPS:  return 131.0f;
        case MPU6050_GYRO_RANGE_500DPS:  return 65.5f;
        case MPU6050_GYRO_RANGE_1000DPS: return 32.8f;
        case MPU6050_GYRO_RANGE_2000DPS: return 16.4f;
        default: return 131.0f;
    }
}

HAL_StatusTypeDef MPU6050_SetTimeout(MPU6050_Handle_t *dev, uint32_t timeout_ms)
{
    if (dev == NULL) return HAL_ERROR;
    dev->timeout_ms = timeout_ms;
    return HAL_OK;
}

HAL_StatusTypeDef MPU6050_Init(MPU6050_Handle_t *dev, I2C_HandleTypeDef *hi2c, uint8_t addr_7bit)
{
    if (dev == NULL || hi2c == NULL) return HAL_ERROR;

    memset(dev, 0, sizeof(*dev));
    dev->hi2c = hi2c;
    dev->addr_7bit = addr_7bit;
    dev->timeout_ms = 100;

    dev->accel_range = MPU6050_ACCEL_RANGE_2G;
    dev->gyro_range = MPU6050_GYRO_RANGE_250DPS;
    dev->accel_lsb_per_g = mpu6050_accel_lsb_per_g(dev->accel_range);
    dev->gyro_lsb_per_dps = mpu6050_gyro_lsb_per_dps(dev->gyro_range);

    uint8_t who = 0;
    HAL_StatusTypeDef st = MPU6050_ReadWhoAmI(dev, &who);
    if (st != HAL_OK) return st;

    if (who != 0x68U && who != 0x69U) {
        return HAL_ERROR;
    }

    st = MPU6050_Sleep(dev, false);
    if (st != HAL_OK) return st;

    st = MPU6050_SetClockSource(dev, 1U);
    if (st != HAL_OK) return st;

    st = MPU6050_SetDLPF(dev, MPU6050_DLPF_42HZ);
    if (st != HAL_OK) return st;

    st = MPU6050_SetSampleRateDivider(dev, 7U);
    if (st != HAL_OK) return st;

    st = MPU6050_SetGyroRange(dev, dev->gyro_range);
    if (st != HAL_OK) return st;

    st = MPU6050_SetAccelRange(dev, dev->accel_range);
    if (st != HAL_OK) return st;

    (void)mpu6050_write_u8(dev, MPU6050_REG_INT_ENABLE, 0x00U);

    return HAL_OK;
}

HAL_StatusTypeDef MPU6050_ReadWhoAmI(MPU6050_Handle_t *dev, uint8_t *whoami)
{
    if (whoami == NULL) return HAL_ERROR;
    return mpu6050_read(dev, MPU6050_REG_WHO_AM_I, whoami, 1);
}

HAL_StatusTypeDef MPU6050_Reset(MPU6050_Handle_t *dev)
{
    HAL_StatusTypeDef st = mpu6050_write_u8(dev, MPU6050_REG_PWR_MGMT_1, MPU6050_PWR1_DEVICE_RESET);
    if (st != HAL_OK) return st;

    HAL_Delay(100);
    return HAL_OK;
}

HAL_StatusTypeDef MPU6050_Sleep(MPU6050_Handle_t *dev, bool enable)
{
    return mpu6050_update_bits(dev, MPU6050_REG_PWR_MGMT_1, MPU6050_PWR1_SLEEP, enable ? MPU6050_PWR1_SLEEP : 0x00U);
}

HAL_StatusTypeDef MPU6050_SetClockSource(MPU6050_Handle_t *dev, uint8_t clk_sel)
{
    uint8_t v = (uint8_t)(clk_sel & 0x07U);
    return mpu6050_update_bits(dev, MPU6050_REG_PWR_MGMT_1, 0x07U, v);
}

HAL_StatusTypeDef MPU6050_SetSampleRateDivider(MPU6050_Handle_t *dev, uint8_t divider)
{
    return mpu6050_write_u8(dev, MPU6050_REG_SMPLRT_DIV, divider);
}

HAL_StatusTypeDef MPU6050_SetDLPF(MPU6050_Handle_t *dev, MPU6050_DLPF_t dlpf)
{
    uint8_t v = (uint8_t)(dlpf & 0x07U);
    return mpu6050_update_bits(dev, MPU6050_REG_CONFIG, 0x07U, v);
}

HAL_StatusTypeDef MPU6050_SetAccelRange(MPU6050_Handle_t *dev, MPU6050_AccelRange_t range)
{
    if (dev == NULL) return HAL_ERROR;

    uint8_t v = (uint8_t)((uint8_t)range << 3);
    HAL_StatusTypeDef st = mpu6050_update_bits(dev, MPU6050_REG_ACCEL_CONFIG, 0x18U, v);
    if (st != HAL_OK) return st;

    dev->accel_range = range;
    dev->accel_lsb_per_g = mpu6050_accel_lsb_per_g(range);
    return HAL_OK;
}

HAL_StatusTypeDef MPU6050_SetGyroRange(MPU6050_Handle_t *dev, MPU6050_GyroRange_t range)
{
    if (dev == NULL) return HAL_ERROR;

    uint8_t v = (uint8_t)((uint8_t)range << 3);
    HAL_StatusTypeDef st = mpu6050_update_bits(dev, MPU6050_REG_GYRO_CONFIG, 0x18U, v);
    if (st != HAL_OK) return st;

    dev->gyro_range = range;
    dev->gyro_lsb_per_dps = mpu6050_gyro_lsb_per_dps(range);
    return HAL_OK;
}

HAL_StatusTypeDef MPU6050_ReadRaw(MPU6050_Handle_t *dev, MPU6050_RawData_t *out)
{
    if (out == NULL) return HAL_ERROR;

    uint8_t buf[14];
    HAL_StatusTypeDef st = mpu6050_read(dev, MPU6050_REG_ACCEL_XOUT_H, buf, (uint16_t)sizeof(buf));
    if (st != HAL_OK) return st;

    out->accel_x = (int16_t)((buf[0] << 8) | buf[1]);
    out->accel_y = (int16_t)((buf[2] << 8) | buf[3]);
    out->accel_z = (int16_t)((buf[4] << 8) | buf[5]);
    out->temp    = (int16_t)((buf[6] << 8) | buf[7]);
    out->gyro_x  = (int16_t)((buf[8] << 8) | buf[9]);
    out->gyro_y  = (int16_t)((buf[10] << 8) | buf[11]);
    out->gyro_z  = (int16_t)((buf[12] << 8) | buf[13]);

    return HAL_OK;
}

void MPU6050_Convert(const MPU6050_Handle_t *dev, const MPU6050_RawData_t *raw, MPU6050_Data_t *out)
{
    if (dev == NULL || raw == NULL || out == NULL) return;

    out->accel_x_g = (float)raw->accel_x / dev->accel_lsb_per_g;
    out->accel_y_g = (float)raw->accel_y / dev->accel_lsb_per_g;
    out->accel_z_g = (float)raw->accel_z / dev->accel_lsb_per_g;

    out->gyro_x_dps = (float)raw->gyro_x / dev->gyro_lsb_per_dps;
    out->gyro_y_dps = (float)raw->gyro_y / dev->gyro_lsb_per_dps;
    out->gyro_z_dps = (float)raw->gyro_z / dev->gyro_lsb_per_dps;

    out->temp_c = ((float)raw->temp / 340.0f) + 36.53f;
}

HAL_StatusTypeDef MPU6050_Read(MPU6050_Handle_t *dev, MPU6050_Data_t *out)
{
    MPU6050_RawData_t raw;
    HAL_StatusTypeDef st = MPU6050_ReadRaw(dev, &raw);
    if (st != HAL_OK) return st;

    MPU6050_Convert(dev, &raw, out);
    return HAL_OK;
}
