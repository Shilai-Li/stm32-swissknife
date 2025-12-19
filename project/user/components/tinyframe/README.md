# TinyFrame - 轻量级串口帧协议解析库

## 📚 简介

TinyFrame 是一个专为资源受限的嵌入式系统设计的简单、轻量级的帧协议库。它可以帮助你在串口（UART/RS232）、SPI 或其他点对点通信接口上构建和解析数据帧。

- **官方仓库**: https://github.com/MightyPork/TinyFrame
- **许可证**: MIT License（工业友好）
- **特性**:
  - 极小的内存占用
  - 支持 CRC 校验（可选）
  - 支持帧重传机制
  - 事件驱动的回调机制
  - 支持多种帧类型和ID

## 🏗️ 目录结构

```
tinyframe/
├── csrc/                    # TinyFrame 官方源代码（通过更新脚本维护）
│   ├── TinyFrame.c
│   └── TinyFrame.h
├── tinyframe.h              # 包装头文件（包含 csrc/TinyFrame.h）
├── tinyframe_port.h         # 端口适配层接口
├── tinyframe_port.c         # 端口适配层实现（依赖 UART 驱动）
├── update_tinyframe.ps1     # 自动更新脚本
└── README.md                # 本文件
```

## 🚀 快速开始

### 1. 下载 TinyFrame 源码

```powershell
# 在项目根目录或临时目录
git clone https://github.com/MightyPork/TinyFrame.git

# 或下载 ZIP 并解压
```

### 2. 运行更新脚本

```powershell
cd HAL/user/components/tinyframe
.\update_tinyframe.ps1 -SourcePath "path\to\TinyFrame"
```

### 3. 在代码中使用

```c
#include "tinyframe.h"
#include "tinyframe_port.h"

// 初始化
TinyFrame *tf = TinyFrame_Init();

// 发送数据
TF_Msg msg;
TF_ClearMsg(&msg);
msg.type = 0x01;
msg.data = (uint8_t*)"Hello";
msg.len = 5;
TF_Send(tf, &msg);

// 在主循环中处理接收（自动调用 UART_Read）
TinyFrame_Process(tf);
```

## ⚙️ CubeMX 配置

TinyFrame 本身不需要特殊的硬件配置，但需要配置 UART：

### UART 配置
参考 `HAL/user/drivers/communication/uart.h` 中的配置说明。

确保在 `uart.h` 中启用了需要使用的 UART：
```c
#define USE_UART2  // TinyFrame 默认使用 UART2
```

## 🔧 配置选项

可以在 `tinyframe_port.h` 中修改配置：

```c
// 使用的 UART 通道
#define TINYFRAME_UART_CHANNEL  UART_CHANNEL_2

// TinyFrame 配置（可选）
#define TF_MAX_PAYLOAD_RX  256  // 最大接收负载
#define TF_MAX_PAYLOAD_TX  256  // 最大发送负载
```

## 📝 API 参考

### 核心 API

| 函数 | 说明 |
|------|------|
| `TinyFrame_Init()` | 初始化 TinyFrame |
| `TF_Send(tf, msg)` | 发送帧 |
| `TF_AddGenericListener(tf, cb)` | 添加通用监听器 |
| `TF_AddTypeListener(tf, type, cb)` | 添加特定类型监听器 |
| `TinyFrame_Process(tf)` | 处理接收数据（在主循环调用）|

### 端口层 API

| 函数 | 说明 |
|------|------|
| `TinyFrame_Init()` | 初始化并配置端口 |
| `TinyFrame_Process(tf)` | 从 UART 读取数据并喂给 TinyFrame |

## 🧪 测试

参考 `HAL/user/components/tests/tinyframe_tests.c` 查看完整的测试示例。

编译时在 CMakeLists.txt 中设置：
```cmake
set(TEST_CASE "tinyframe_tests")
```

## 📖 更多资源

- [TinyFrame 官方文档](https://github.com/MightyPork/TinyFrame)
- [TinyFrame Wiki](https://github.com/MightyPork/TinyFrame/wiki)

## ⚠️ 注意事项

1. **不要直接修改 `csrc/` 目录中的文件**，这些文件由更新脚本管理
2. **所有平台相关的代码应放在 `tinyframe_port.c/h` 中**
3. 更新库时，先查看官方 changelog，确认没有破坏性变更
4. TinyFrame 本身不处理线程安全，如果在多任务环境使用需要注意同步

## 🔄 更新流程

1. 下载最新版本的 TinyFrame
2. 运行 `.\update_tinyframe.ps1 -SourcePath "path\to\TinyFrame"`
3. 检查是否有新的配置选项或 API 变更
4. 重新编译并测试

---

**集成时间**: 2025-12-18  
**维护者**: Shilai-Li/stm32-swissknife
