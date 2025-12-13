/*
 * CubeMX 配置/初始化步骤（本测试代码不包含外设初始化）：
 *
 * 1) 在 CubeMX 里打开 .ioc
 *
 * 2) Pinout & Configuration -> Connectivity -> I2C
 *    - 使能一个 I2C（例如 I2C1）为 I2C 模式
 *    - 确认 SDA/SCL 引脚已分配到对应管脚（例如 STM32F103 常见：PB7=SDA, PB6=SCL）
 *
 * 3) I2C 参数建议
 *    - I2C Speed：Standard-mode (100kHz) 或 Fast-mode (400kHz)
 *    - Addressing Mode：7-bit
 *
 * 4) 硬件连线
 *    - MPU6050 VCC 接 3.3V（建议）
 *    - GND 接地
 *    - SDA/SCL 需要上拉电阻（很多模块板载 4.7k/10k，上拉到 3.3V）
 *    - AD0=0 时地址 0x68，AD0=1 时地址 0x69
 *
 * 5) 生成代码
 *    - Generate Code 后会生成/更新 Core/Src/i2c.c 与 Core/Inc/i2c.h（以及 hi2c1 等句柄）
 *
 * 6) 工程里确保已调用 MX_I2C1_Init()（通常在 main.c 的 MX_* 初始化流程中）
 *
 * 7) 可选：为了看到串口打印
 *    - 使能一个 USART（例如 USART2）并生成 usart.c/usart.h
 *    - 本工程可用 UART_Init()/UART_Debug_Printf() 输出调试信息
 */

#include "mpu6050_driver.h"
#include "uart_driver.h"

/* 该句柄由 CubeMX 生成（启用 I2C1 后在 i2c.c 中定义）。
 * 如果你用的是其它 I2C（如 hi2c2），请修改这里。
 */
extern I2C_HandleTypeDef hi2c1;

void User_Entry(void)
{
    UART_Init();
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
            UART_Debug_Printf(
                "ax=%.3fg ay=%.3fg az=%.3fg | gx=%.2fdps gy=%.2fdps gz=%.2fdps | t=%.2fC\r\n",
                data.accel_x_g, data.accel_y_g, data.accel_z_g,
                data.gyro_x_dps, data.gyro_y_dps, data.gyro_z_dps,
                data.temp_c
            );
        } else {
            UART_Debug_Printf("MPU6050_Read failed, status=%d\r\n", (int)st);
        }

        HAL_Delay(200);
    }
}
