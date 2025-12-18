# MPU6050 驱动集成指南

本指南介绍如何在 STM32 工程中集成 MPU6050 6轴传感器（加速度计 + 陀螺仪）驱动。

## 1. 硬件连接 (Hardware Connection)

MPU6050 模块通常使用 I2C 接口通信。

| MPU6050 Pin | STM32 Pin (示例) | 说明 |
| :--- | :--- | :--- |
| **VCC** | 3.3V | 推荐接 3.3V。若模块带稳压芯片也可接 5V，但 I2C 电平最好匹配单片机电压。 |
| **GND** | GND | 共地 |
| **SCL** | I2C_SCL (如 PB6) | 建议外接 4.7kΩ 或 10kΩ 上拉电阻 (模块通常自带) |
| **SDA** | I2C_SDA (如 PB7) | 建议外接 4.7kΩ 或 10kΩ 上拉电阻 (模块通常自带) |
| **AD0** | GND / 3.3V | **GND**: I2C 地址 = `0x68`<br>**3.3V**: I2C 地址 = `0x69` |
| **INT** | GPIO (可选) | 如果需要使用数据就绪中断功能，可连接到任意 GPIO 输入 |

## 2. CubeMX 配置 (Configuration)

### 2.1 启用 I2C
1. 打开工程的 `.ioc` 文件。
2. 进入 **Connectivity** -> **I2C1** (或 I2Cx)。
3. 将 **I2C** 设置为 `I2C` 模式。
4. **Parameter Settings**:
   - **I2C Speed Mode**: `Standard Mode` (100kHz) 或 `Fast Mode` (400kHz)。建议先用 100kHz 测试。
   - **Addressing Mode**: `7-bit`。
5. **GPIO Settings**:
   - 确认 SCL/SDA 引脚映射正确（例如 STM32F103C8T6 默认为 PB6/PB7）。
   - GPIO Pull-up/Pull-down: 如果外部有上拉电阻，可设为 `No pull-up and no pull-down`；如果外部没有，建议设为 `Pull-up`。

### 2.2 (可选) 启用 UART 调试打印
为了查看测试结果，建议配置串口。
1. **Connectivity** -> **USART1** (或其它)。
2. **Mode**: `Asynchronous`。
3. **Baud Rate**: `115200` (或其它常用波特率)。

### 2.3 生成代码
点击 **GENERATE CODE**，CubeMX 会生成 `i2c.c/h` 等文件。

## 3. 软件集成 (Software Integration)

### 3.1 文件包含
确保以下文件在工程目录中并被编译：
- `mpu6050.c`
- `mpu6050.h`

### 3.2 初始化代码
在 `main.c` 或任务线程中调用初始化函数。

```c
#include "mpu6050.h"

// 引用 CubeMX 生成的 I2C 句柄
extern I2C_HandleTypeDef hi2c1;

MPU6050_Handle_t imu;

void App_Init(void) {
    // 1. 初始化，指定 I2C 句柄和设备地址
    // AD0 接 GND -> MPU6050_I2C_ADDR_LOW (0x68)
    // AD0 接 3.3V -> MPU6050_I2C_ADDR_HIGH (0x69)
    if (MPU6050_Init(&imu, &hi2c1, MPU6050_I2C_ADDR_LOW) != HAL_OK) {
        // 初始化失败处理
        printf("MPU6050 Init Failed!\n");
    }

    // 2. 配置传感器参数 (可选，Init 默认已有配置)
    MPU6050_SetAccelRange(&imu, MPU6050_ACCEL_RANGE_8G);
    MPU6050_SetGyroRange(&imu, MPU6050_GYRO_RANGE_500DPS);
}
```

### 3.3 数据读取
建议在主循环或定时器中周期性读取。

```c
void App_Loop(void) {
    MPU6050_Data_t data;
    
    // 读取并转换后的工程值 (g, dps, degC)
    if (MPU6050_Read(&imu, &data) == HAL_OK) {
        // 使用 float 打印 (若不支持 %f，请使用整数转换技巧)
        printf("Ax: %.2f g, Ay: %.2f g, Az: %.2f g\n", 
               data.accel_x_g, data.accel_y_g, data.accel_z_g);
    }
    
    HAL_Delay(100); // 10Hz 采样
}
```

## 4. 常见问题 (FAQ)

- **初始化返回 status=2 (HAL_BUSY)**
  - 检查 I2C 总线是否有其他设备占用。
  - 尝试复位单片机或给模块重新上电。

- **读出的数据全为 0 或固定值**
  - 检查设备地址 (0x68 vs 0x69) 是否设对。
  - 检查 SCL/SDA 线是否接反。
  - 检查上拉电阻是否连接良好。

- **printf 无法打印浮点数**
  - 很多嵌入式标准库默认关闭 `%f` 支持以节省空间。
  - 解决方案：手动将 float 放大 1000 倍转为 int 打印，或者在编译器链接选项中添加 `-u _printf_float` (会增加代码体积)。
