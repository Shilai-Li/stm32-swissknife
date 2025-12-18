# lwrb - 轻量级环形缓冲区库

## 📚 简介

lwrb (Lightweight Ring Buffer) 是一个通用的、高性能的环形缓冲区实现，特别适合嵌入式系统中的数据缓冲需求。它提供了线程安全（中断安全）的操作，非常适合与 DMA 配合使用。

- **官方仓库**: https://github.com/MaJerle/lwrb
- **许可证**: MIT License（工业友好）
- **特性**:
  - 极小的内存和代码占用
  - 线程安全（中断安全）
  - 零拷贝操作支持
  - 支持读写指针直接访问（适合 DMA）
  - 纯 C 实现，无依赖

## 🏗️ 目录结构

```
lwrb/
├── csrc/                    # lwrb 官方源代码（通过更新脚本维护）
│   ├── lwrb.c
│   └── lwrb.h
├── lwrb.h                   # 包装头文件（包含 csrc/lwrb.h）
├── lwrb_port.h              # 端口适配层接口（配置选项）
├── lwrb_port.c              # 端口适配层实现（可选）
├── update_lwrb.ps1          # 自动更新脚本
└── README.md                # 本文件
```

## 🚀 快速开始

### 1. 下载 lwrb 源码

```powershell
# 在项目根目录或临时目录
git clone https://github.com/MaJerle/lwrb.git

# 或下载 ZIP 并解压
```

### 2. 运行更新脚本

```powershell
cd HAL/user/components/lwrb
.\update_lwrb.ps1 -SourcePath "path\to\lwrb"
```

### 3. 在代码中使用

```c
#include "lwrb.h"

// 定义缓冲区
static uint8_t buffer_data[256];
static lwrb_t buff;

// 初始化
lwrb_init(&buff, buffer_data, sizeof(buffer_data));

// 写入数据
uint8_t data[] = "Hello";
lwrb_write(&buff, data, sizeof(data));

// 读取数据
uint8_t out[10];
size_t read = lwrb_read(&buff, out, sizeof(out));

// 查询可用数据
size_t available = lwrb_get_full(&buff);
```

## 💡 为什么需要 lwrb？

虽然我们的 UART 驱动已经有了环形缓冲区实现，但 lwrb 提供了：

1. **标准化接口**: 可以在不同的驱动之间复用
2. **更丰富的 API**: 支持批量读写、预读（peek）、跳过等操作
3. **DMA 优化**: 提供获取连续内存区域的 API，适合 DMA 操作
4. **可移植性**: 可以用于 UART 以外的缓冲场景（SPI、I2C 等）

### 使用场景示例

#### 场景 1: 作为通用缓冲区库
```c
// 可以用于任何需要环形缓冲的地方
lwrb_t spi_rx_buff;
lwrb_t sensor_data_buff;
```

#### 场景 2: 与 TinyFrame 配合
```c
// 可以作为 TinyFrame 的底层缓冲
lwrb_t frame_rx_buff;
// 先读到 lwrb，再批量解析
```

#### 场景 3: 替代或增强现有 UART 缓冲
```c
// 如果需要更复杂的缓冲操作，可以基于 lwrb 重构
// 但当前 UART 驱动已足够高效，暂不需要
```

## ⚙️ 配置选项

lwrb 本身几乎不需要配置，但可以在编译时定义：

```c
// lwrb_port.h
#define LWRB_DISABLE_ATOMIC     0  // 启用原子操作保护
#define LWRB_MEMCPY             memcpy
#define LWRB_MEMSET             memset
```

## 📝 API 参考

### 核心 API

| 函数 | 说明 |
|------|------|
| `lwrb_init(&buff, data, size)` | 初始化缓冲区 |
| `lwrb_write(&buff, data, len)` | 写入数据 |
| `lwrb_read(&buff, data, len)` | 读取数据 |
| `lwrb_peek(&buff, pos, data, len)` | 预读数据（不移除）|
| `lwrb_skip(&buff, len)` | 跳过数据 |
| `lwrb_get_full(&buff)` | 获取已用空间 |
| `lwrb_get_free(&buff)` | 获取剩余空间 |
| `lwrb_reset(&buff)` | 重置缓冲区 |

### DMA 优化 API

| 函数 | 说明 |
|------|------|
| `lwrb_get_linear_block_read_length(&buff)` | 获取连续可读长度 |
| `lwrb_get_linear_block_write_length(&buff)` | 获取连续可写长度 |
| `lwrb_skip(&buff, len)` | 配合 DMA 使用，跳过已处理数据 |
| `lwrb_advance(&buff, len)` | 配合 DMA 使用，推进写指针 |

## 🧪 测试

参考 `HAL/user/components/tests/lwrb_tests.c` 查看完整的测试示例。

编译时在 CMakeLists.txt 中设置：
```cmake
set(TEST_CASE "lwrb_tests")
```

## 📖 更多资源

- [lwrb 官方文档](https://github.com/MaJerle/lwrb)
- [lwrb API 文档](https://majerle.eu/documentation/lwrb/html/index.html)

## ⚠️ 注意事项

1. **不要直接修改 `csrc/` 目录中的文件**，这些文件由更新脚本管理
2. lwrb 是线程安全的（在单生产者-单消费者模型下）
3. lwrb 不分配内存，需要你提供缓冲区空间
4. 缓冲区大小建议为 2 的幂，以获得最佳性能（非强制）

## 🔄 更新流程

1. 下载最新版本的 lwrb
2. 运行 `.\update_lwrb.ps1 -SourcePath "path\to\lwrb"`
3. 检查是否有新的配置选项或 API 变更
4. 重新编译并测试

## 🆚 与现有 UART 驱动的关系

当前项目的 UART 驱动（`uart.c/h`）已经实现了高效的环形缓冲区：
- ✅ DMA 循环接收
- ✅ 中断安全的发送缓冲
- ✅ 错误检测和恢复

**lwrb 的定位**：
- 🔧 作为**通用工具库**，可用于其他需要缓冲的场景
- 📦 提供**标准化接口**，便于代码复用和移植
- 🚀 **不替代** UART 驱动的现有缓冲实现

**何时使用 lwrb**：
- ✅ 需要在应用层添加额外的缓冲层
- ✅ 需要批量解析或预读功能
- ✅ 在非 UART 的场景（SPI、自定义协议等）
- ❌ 不建议仅为了使用而修改已稳定的 UART 驱动

---

**集成时间**: 2025-12-18  
**维护者**: Shilai-Li/stm32-swissknife
