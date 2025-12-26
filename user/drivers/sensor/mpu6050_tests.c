/*
 * MPU6050 Driver Test Case
 * For detailed integration steps, please see mpu6050-README.md in the drivers/sensor directory.
 */

#include "mpu6050.h"
#include "usart.h"
#include "uart.h"
#include <stdlib.h>

/* 该句柄由 CubeMX 生成（启用 I2C1 后在 i2c.c 中定义）。
 * 如果你用的是其它 I2C（如 hi2c2），请修改这里。
 */
extern I2C_HandleTypeDef hi2c1;

extern UART_HandleTypeDef huart2;

// UART Buffers
static uint8_t uart_rx_dma[64];
static uint8_t uart_rx_buf[256];
static uint8_t uart_tx_buf[512];

void app_main(void)
{
    // Register UART for Debug Printf (channel 0)
    UART_Register(0, &huart2, 
                  uart_rx_dma, sizeof(uart_rx_dma),
                  uart_rx_buf, sizeof(uart_rx_buf),
                  uart_tx_buf, sizeof(uart_tx_buf));

    UART_Debug_Printf("MPU6050 test start\r\n");

    MPU6050_Handle_t imu;
    HAL_StatusTypeDef st = MPU6050_Init(&imu, &hi2c1, MPU6050_I2C_ADDR_LOW);
    if (st != HAL_OK) {
        UART_Debug_Printf("MPU6050_Init failed, status=%d\r\n", (int)st);
        while (1) {
            HAL_Delay(1000);
        }
    }

    uint8_t who = 0;
    st = MPU6050_ReadWhoAmI(&imu, &who);
    UART_Debug_Printf("MPU6050 WHO_AM_I=0x%02X, status=%d\r\n", who, (int)st);

    (void)MPU6050_SetAccelRange(&imu, MPU6050_ACCEL_RANGE_2G);
    (void)MPU6050_SetGyroRange(&imu, MPU6050_GYRO_RANGE_250DPS);
    (void)MPU6050_SetDLPF(&imu, MPU6050_DLPF_42HZ);

    MPU6050_Data_t data;

    while (1)
    {
        st = MPU6050_Read(&imu, &data);
        if (st == HAL_OK) {
            int32_t ax = (int32_t)(data.accel_x_g * 1000);
            int32_t ay = (int32_t)(data.accel_y_g * 1000);
            int32_t az = (int32_t)(data.accel_z_g * 1000);
            
            int32_t gx = (int32_t)(data.gyro_x_dps * 100);
            int32_t gy = (int32_t)(data.gyro_y_dps * 100);
            int32_t gz = (int32_t)(data.gyro_z_dps * 100);
            
            int32_t temp = (int32_t)(data.temp_c * 100);

            UART_Debug_Printf(
                "ax=%s%d.%03dg ay=%s%d.%03dg az=%s%d.%03dg | gx=%s%d.%02ddps gy=%s%d.%02ddps gz=%s%d.%02ddps | t=%s%d.%02dC\r\n",
                ax < 0 ? "-" : "", abs(ax) / 1000, abs(ax) % 1000,
                ay < 0 ? "-" : "", abs(ay) / 1000, abs(ay) % 1000,
                az < 0 ? "-" : "", abs(az) / 1000, abs(az) % 1000,
                gx < 0 ? "-" : "", abs(gx) / 100, abs(gx) % 100,
                gy < 0 ? "-" : "", abs(gy) / 100, abs(gy) % 100,
                gz < 0 ? "-" : "", abs(gz) / 100, abs(gz) % 100,
                temp < 0 ? "-" : "", abs(temp) / 100, abs(temp) % 100
            );
        } else {
            UART_Debug_Printf("MPU6050_Read failed, status=%d\r\n", (int)st);
        }

        HAL_Delay(200);
    }
}
