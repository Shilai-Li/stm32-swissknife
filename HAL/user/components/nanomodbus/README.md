# nanoMODBUS Component

nanoMODBUS是一个为微控制器设计的轻量级 Modbus RTU/TCP C 库。

## 特性

- ✅ **轻量级** - 无动态内存分配
- ✅ **无OS依赖** - 支持裸机运行
- ✅ **MIT许可** - 商业友好
- ✅ **支持RTU和TCP** - 灵活的传输层
- ✅ **主从机模式** - Client 和 Server 都支持
- ✅ **完整的功能码** - 支持所有常用 Modbus 功能码

## 目录结构

```
components/nanomodbus/
├── csrc/
│   ├── nanomodbus.c           # 核心库实现
│   └── nanomodbus.h           # 核心库头文件
├── nanomodbus.h               # 包装头文件
├── nanomodbus_port.h          # STM32 HAL 移植层头文件
├── nanomodbus_port.c          # STM32 HAL 移植层实现（裸机）
└── README.md                  # 本文件
```

## CubeMX 配置

### UART 配置（Modbus RTU）

1. **外设选择**: USART1 或 USART2
2. **模式**: Asynchronous
3. **波特率**: 9600 bps（或根据实际需求配置: 19200, 115200）
4. **数据位**: 8 Bits
5. **校验位**: Even（偶校验）或 None（无校验）
6. **停止位**: 1
7. **方向**: Receive and Transmit

### GPIO 配置（可选）

如果使用 RS485 收发器，需要配置 DE/RE 控制引脚。

## API 使用

### Server（从机）模式

```c
#include "nanomodbus.h"

nmbs_t modbus_server;
nmbs_server_data_t server_data;

void Modbus_Init(void) {
    // 初始化 Modbus Server (Unit ID = 1, UART2)
    nmbs_error err = nmbs_server_init_rtu(&modbus_server, &server_data, 0x01, &huart2);
    
    // 初始化一些寄存器数据
    server_data.regs[0] = 0x1234;
    server_data.regs[1] = 0x5678;
    nmbs_bitfield_write(server_data.coils, 0, 1);  // Coil 0 = ON
}

void Modbus_Loop(void) {
    while (1) {
        // 轮询处理 Modbus 请求
        nmbs_error err = nmbs_server_poll(&modbus_server);
        
        if (err == NMBS_ERROR_NONE) {
            // 请求处理成功
        } else if (err == NMBS_ERROR_TIMEOUT) {
            // 超时（正常，无请求时会超时）
        } else {
            // 其他错误
        }
    }
}
```

### Client（主机）模式

```c
#include "nanomodbus.h"

nmbs_t modbus_client;

void Modbus_Init(void) {
    // 初始化 Modbus Client (UART2)
    nmbs_error err = nmbs_client_init_rtu(&modbus_client, &huart2);
    
    // 设置目标从机地址
    nmbs_set_destination_rtu_address(&modbus_client, 0x01);
}

void Modbus_Test(void) {
    uint16_t regs[10];
    
    // 读取保持寄存器 (FC 03)
    nmbs_error err = nmbs_read_holding_registers(&modbus_client, 0, 3, regs);
    if (err == NMBS_ERROR_NONE) {
        // 读取成功，regs 包含数据
    }
    
    // 写单个寄存器 (FC 06)
    err = nmbs_write_single_register(&modbus_client, 0, 0x1234);
    
    // 写多个寄存器 (FC 16)
    uint16_t write_data[3] = {0xAABB, 0xCCDD, 0xEEFF};
    err = nmbs_write_multiple_registers(&modbus_client, 0, 3, write_data);
    
    // 读取线圈 (FC 01)
    uint8_t coils[32];
    err = nmbs_read_coils(&modbus_client, 0, 8, coils);
    
    // 写单个线圈 (FC 05)
    err = nmbs_write_single_coil(&modbus_client, 0, true);
}
```

## 支持的功能码

| 功能码 | 名称 | 客户端 | 服务端 |
|--------|------|--------|--------|
| 01 | Read Coils | ✅ | ✅ |
| 02 | Read Discrete Inputs | ✅ | ✅ |
| 03 | Read Holding Registers | ✅ | ✅ |
| 04 | Read Input Registers | ✅ | ✅ |
| 05 | Write Single Coil | ✅ | ✅ |
| 06 | Write Single Register | ✅ | ✅ |
| 15 (0x0F) | Write Multiple Coils | ✅ | ✅ |
| 16 (0x10) | Write Multiple Registers | ✅ | ✅ |

## 测试

使用以下测试用例：

```bash
# 在 HAL/user/CMakeLists.txt 中设置:
set(TEST_CASE "nanomodbus" ...)
```

然后编译并烧录到STM32。

## 配置说明

### 缓冲区大小

在 `nanomodbus_port.h` 中可以调整：

```c
#define NMBS_COIL_BUF_SIZE 256   /* Coils 缓冲区大小 (bits) */
#define NMBS_REG_BUF_SIZE  256   /* Registers 缓冲区大小 (16-bit) */
```

### 超时设置

在移植层初始化时设置：

```c
nmbs_set_byte_timeout(nmbs, 100);   /* 字节间超时: 100ms */
nmbs_set_read_timeout(nmbs, 1000);  /* 读超时: 1000ms */
```

## 参考资料

- 官方GitHub: https://github.com/debevv/nanoMODBUS
- Modbus协议: https://www.modbus.org
- Modbus RTU规范: https://www.modbus.org/docs/Modbus_over_serial_line_V1_02.pdf

## 许可证

MIT License - 商业友好，可以自由使用
