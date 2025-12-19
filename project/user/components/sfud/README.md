# SFUD Component

**SFUD** (Serial Flash Universal Driver) 是一个开源的串行 Flash 通用驱动库。

## 📚 简介

SFUD 提供了统一的 API 接口用于操作不同品牌和型号的 SPI Flash 芯片，无需为每个 Flash 型号单独编写驱动代码。

### 特性

- ✅ **通用性强**：支持使用 SFDP 标准的 SPI Flash（几乎所有主流厂商）
- ✅ **功能完整**：读、写、擦除、快速读取、状态查询等
- ✅ **移植简单**：只需实现 SPI 读写函数即可
- ✅ **资源占用小**：ROM ~12KB，RAM ~1KB + 设备数量 × 140 字节
- ✅ **中文文档**：完善的中文使用说明

### 支持的 Flash 型号（部分）

| 厂商 | 型号 |
|------|------|
| Winbond | W25Q80, W25Q16, W25Q32, W25Q64, W25Q128, W25Q256 |
| GigaDevice | GD25Q16, GD25Q32, GD25Q64, GD25Q128 |
| Macronix | MX25L3206E, MX25L6406E, MX25L12835F |
| ISSI | IS25LP064, IS25LP128 |
| Microchip | SST25VF016B, SST26VF064B |

完整列表请参考：https://github.com/armink/SFUD#支持的-flash

## 📁 目录结构

```
sfud/
├── csrc/                    # SFUD 核心源码（从官方仓库复制）
│   ├── sfud.c              # 核心实现
│   ├── sfud.h              # 核心头文件
│   ├── sfud_sfdp.c         # SFDP 解析
│   ├── sfud_cfg.h          # 配置文件（已定制）
│   ├── sfud_def.h          # 宏定义
│   └── sfud_flash_def.h    # Flash 信息表
├── sfud.h                   # 组件包装头文件
├── sfud_port.h              # 移植层头文件
├── sfud_port.c              # 移植层实现（STM32 HAL）
└── update_sfud.ps1          # 更新脚本
```

## 🚀 快速开始

### 1. 硬件连接

以 W25Q64 为例，连接到 STM32 的 SPI1：

```
STM32F103           W25Q64 (SOP-8)
┌──────────┐        ┌──────────────┐
│          │        │  1 CS   VCC 8│──── 3.3V
│      PA4 ├────────┤  2 DO   GND 7│──── GND
│          │        │  3 WP   HOLD 6│── (Pull High or NC)
│     GND  ├────────┤  4 GND  CLK 5│
│          │        └──────────────┘
│      PA5 ├──────────────────────┘
│      PA6 ├──────────────────────────┘
│      PA7 ├─────────────────────────────────┘
└──────────┘
```

### 2. CubeMX 配置

#### SPI1 配置

| 参数 | 值 |
|------|-----|
| Mode | Full-Duplex Master |
| NSS | Disable (使用软件 CS) |
| Baud Rate | 36 MHz (或尽可能高) |
| CPOL | Low |
| CPHA | 1 Edge |
| Data Size | 8 Bits |
| First Bit | MSB First |

#### GPIO 配置

| 引脚 | 模式 | 标签 |
|------|------|-------|
| PA5 | SPI1_SCK | - |
| PA6 | SPI1_MISO | - |
| PA7 | SPI1_MOSI | - |
| PA4 | GPIO_Output | FLASH_CS |

⚠️ **重要**：确保 PA4 在 SPI 初始化前设置为 HIGH

### 3. 代码集成

#### 在 main.c 中：

```c
#include "sfud.h"
#include "sfud_port.h"

void User_Entry(void)
{
    // 初始化 SFUD
    if (SFUD_Port_Init() == SFUD_SUCCESS) {
        printf("Flash initialized successfully!\r\n");
        
        // 获取 Flash 设备句柄
        const sfud_flash *flash = SFUD_Port_GetDefaultFlash();
        
        uint8_t buffer[256];
        
        // 读取数据
        sfud_read(flash, 0x000000, sizeof(buffer), buffer);
        
        // 擦除扇区（4KB）
        sfud_erase(flash, 0x000000, 4096);
        
        // 写入数据
        sfud_write(flash, 0x000000, sizeof(buffer), buffer);
        
        // 擦除后写入（推荐）
        sfud_erase_write(flash, 0x000000, sizeof(buffer), buffer);
    }
    
    while (1) {
        HAL_Delay(1000);
    }
}
```

### 4. 运行测试

在 CMakeLists.txt 中设置：

```cmake
set(TEST_CASE "sfud" CACHE STRING "Select which test to run" FORCE)
```

然后编译并烧录到 STM32，测试程序会自动运行以下测试：

1. ✅ Flash 探测（读取 ID、容量等信息）
2. ✅ Flash 擦除测试
3. ✅ Flash 写入测试
4. ✅ Flash 读取与验证测试
5. ✅ Flash 擦除写入测试

## 📖 API 说明

### 初始化

```c
// 初始化 SFUD（推荐使用 Port 封装）
sfud_err SFUD_Port_Init(void);

// 获取默认 Flash 设备
const sfud_flash *SFUD_Port_GetDefaultFlash(void);

// 原始初始化（如果需要更多控制）
sfud_err sfud_init(void);
const sfud_flash *sfud_get_device_table(void);
```

### 基本操作

```c
// 读取数据
sfud_err sfud_read(const sfud_flash *flash, 
                   uint32_t addr, 
                   size_t size, 
                   uint8_t *data);

// 写入数据（Flash 区域必须已擦除）
sfud_err sfud_write(const sfud_flash *flash, 
                    uint32_t addr, 
                    size_t size, 
                    const uint8_t *data);

// 擦除扇区/块
sfud_err sfud_erase(const sfud_flash *flash, 
                    uint32_t addr, 
                    size_t size);

// 擦除后写入（推荐，自动处理擦除）
sfud_err sfud_erase_write(const sfud_flash *flash, 
                          uint32_t addr, 
                          size_t size, 
                          const uint8_t *data);

// 擦除整个芯片
sfud_err sfud_chip_erase(const sfud_flash *flash);
```

### Flash 信息

```c
const sfud_flash *flash = SFUD_Port_GetDefaultFlash();

// Flash 名称
printf("Name: %s\r\n", flash->name);

// 制造商 ID、类型 ID、容量 ID
printf("Manufacturer: 0x%02X\r\n", flash->chip.mf_id);
printf("Type: 0x%02X\r\n", flash->chip.type_id);
printf("Capacity: 0x%02X\r\n", flash->chip.capacity_id);

// 总容量（字节）
printf("Size: %ld bytes\r\n", flash->chip.capacity);

// 擦除粒度（通常是 4KB）
printf("Erase granularity: %ld bytes\r\n", flash->chip.erase_gran);
```

## ⚙️ 配置说明

### 修改 Flash 设备表

编辑 `csrc/sfud_cfg.h`：

```c
enum {
    SFUD_W25Q_DEVICE_INDEX = 0,
    SFUD_GD25Q_DEVICE_INDEX = 1,  // 添加更多设备
};

#define SFUD_FLASH_DEVICE_TABLE                                    \
{                                                                  \
    [SFUD_W25Q_DEVICE_INDEX] = {.name = "W25Q64", .spi.name = "SPI1"}, \
    [SFUD_GD25Q_DEVICE_INDEX] = {.name = "GD25Q32", .spi.name = "SPI2"}, \
}
```

### 修改 CS 引脚

编辑 `sfud_port.c`：

```c
#define SFUD_CS_PORT    GPIOB
#define SFUD_CS_PIN     GPIO_PIN_12
```

### 禁用调试模式（减少代码大小）

编辑 `csrc/sfud_cfg.h`：

```c
// #define SFUD_DEBUG_MODE  // 注释掉这行
```

## 🔄 更新 SFUD 库

使用提供的 PowerShell 脚本更新到最新版本：

```powershell
# 1. 克隆官方仓库
git clone https://github.com/armink/SFUD.git

# 2. 运行更新脚本
cd HAL/user/components/sfud
.\update_sfud.ps1 -SourcePath "path\to\SFUD"
```

脚本会自动：
- 备份当前版本
- 复制新的核心文件
- **保留你的配置文件** (`sfud_cfg.h`)
- **保留你的移植文件** (`sfud_port.c/h`)

## 📚 更多资源

- 官方 GitHub：https://github.com/armink/SFUD
- 中文文档：https://github.com/armink/SFUD/blob/master/docs/zh-cn/api.md
- API 参考：https://github.com/armink/SFUD/blob/master/docs/zh-cn/api.md
- 常见问题：https://github.com/armink/SFUD/blob/master/docs/zh-cn/faq.md

## 🐛 故障排除

### Flash 无法识别

1. 检查 SPI 时钟频率是否过高（尝试降低到 1-2 MHz）
2. 确认 CS 引脚配置正确
3. 使用示波器或逻辑分析仪检查 SPI 信号
4. 确认 Flash 供电正常（3.3V）

### 数据读写错误

1. 确保在写入前已擦除对应区域
2. 推荐使用 `sfud_erase_write()` 而不是单独的擦除和写入
3. 检查地址对齐（某些操作需要对齐到扇区边界）

### 编译错误

1. 确认已包含所有必要的源文件（`sfud.c`, `sfud_sfdp.c`）
2. 确认 include 路径正确
3. 检查 `sfud_cfg.h` 中的宏定义

## 📝 许可证

SFUD 库使用 MIT 许可证。详见官方仓库 LICENSE 文件。

---

**维护者**: stm32-swissknife 项目团队  
**最后更新**: 2025-12-18
