/*
 * W25Qxx SPI Flash 驱动测试代码
 *
 * CubeMX 配置步骤建议：
 * 1. Connectivity -> SPI1 (或其它SPI)
 *    - Mode: Full-Duplex Master
 *    - Data Size: 8 bits
 *    - Prescaler: 根据系统时钟选择，建议先用较低速度测试 (e.g. 256/128/64)
 *    - CPOL/CPHA: Mode 0 (CPOL=Low, CPHA=1 Edge) 或 Mode 3 (High, 2 Edge) - W25Qxx 支持两者，通常用 Mode 0 或 3
 *    - NSS: Disable (使用软件控制 CS)
 *
 * 2. System Core -> GPIO
 *    - 配置一个引脚作为 CS (片选)，Output Push Pull
 *    - User Label: W25QXX_CS (可选)
 *
 * 3. 连线
 *    - CS   -> 配置的 CS 引脚
 *    - CLK  -> SPI SCK
 *    - MISO -> SPI MISO
 *    - MOSI -> SPI MOSI
 *    - VCC  -> 3.3V
 *    - GND  -> GND
 */

#include "w25qxx.h"
#include "uart.h"
#include <string.h>

// --- 配置区域 ---

// 定义使用的 SPI 句柄 (通常在 main.c/spi.c 中定义)
extern SPI_HandleTypeDef hspi1;

// 定义 CS 引脚 (假如未在 main.h 中定义)
#ifndef W25QXX_CS_GPIO_PORT
#define W25QXX_CS_GPIO_PORT GPIOB // 请修改为实际端口
#endif
#ifndef W25QXX_CS_PIN
#define W25QXX_CS_PIN GPIO_PIN_0  // 请修改为实际引脚
#endif

// --- 测试入口 ---

static W25QXX_HandleTypeDef w25q;

void app_main(void)
{
    UART_Init();
    UART_Debug_Printf("\r\n--- W25Qxx Flash Test Start ---\r\n");

    // 1. 初始化
    UART_Debug_Printf("Initializing W25Qxx...\r\n");
    if (W25QXX_Init(&w25q, &hspi1, W25QXX_CS_GPIO_PORT, W25QXX_CS_PIN)) {
        UART_Debug_Printf("Init Success!\r\n");
        UART_Debug_Printf("Chip ID: 0x%X\r\n", w25q.Info.ID);
        UART_Debug_Printf("Capacity: %lu KB\r\n", w25q.Info.CapacityInKiloByte);
        UART_Debug_Printf("Block Count: %lu\r\n", w25q.Info.BlockCount);
    } else {
        UART_Debug_Printf("Init Failed! Check wiring and SPI config.\r\n");
        while(1) HAL_Delay(1000);
    }

    // 2. 擦除测试 (Sector 0)
    uint32_t test_addr = 0x000000;
    UART_Debug_Printf("Erasing Sector 0 @ 0x%06X...\r\n", test_addr);
    W25QXX_Erase_Sector(&w25q, test_addr);
    UART_Debug_Printf("Erase Done.\r\n");

    // 验证擦除 (读出来应该是 0xFF)
    uint8_t read_buf[32];
    memset(read_buf, 0, sizeof(read_buf));
    W25QXX_Read(&w25q, read_buf, test_addr, sizeof(read_buf));
    
    int is_erased = 1;
    for (int i = 0; i < sizeof(read_buf); i++) {
        if (read_buf[i] != 0xFF) {
            is_erased = 0;
            break;
        }
    }
    
    if (is_erased) {
        UART_Debug_Printf("Erase Verification Passed (All 0xFF).\r\n");
    } else {
        UART_Debug_Printf("Erase Verification FAILED!\r\n");
        UART_Debug_Printf("Data read: %02X %02X %02X ...\r\n", read_buf[0], read_buf[1], read_buf[2]);
    }

    // 3. 写入测试
    uint8_t write_data[] = "Hello STM32 W25Qxx Driver!";
    UART_Debug_Printf("Writing data: \"%s\"\r\n", write_data);
    W25QXX_Write(&w25q, write_data, test_addr, sizeof(write_data));
    UART_Debug_Printf("Write Done.\r\n");

    // 4. 读取验证
    memset(read_buf, 0, sizeof(read_buf));
    W25QXX_Read(&w25q, read_buf, test_addr, sizeof(write_data)); // 读取写入的长度
    UART_Debug_Printf("Read Back: \"%s\"\r\n", read_buf);

    if (memcmp(write_data, read_buf, sizeof(write_data)) == 0) {
        UART_Debug_Printf("Write/Read Verification Passed!\r\n");
    } else {
        UART_Debug_Printf("Write/Read Verification FAILED!\r\n");
    }
    
    // 5. 读取唯一ID
    W25QXX_ReadUniqID(&w25q);
    UART_Debug_Printf("Unique ID: ");
    for(int i=0; i<8; i++) UART_Debug_Printf("%02X ", w25q.Info.UniqID[i]);
    UART_Debug_Printf("\r\n");

    UART_Debug_Printf("--- Test Loop Start ---\r\n");
    while (1) {
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13); // Blink LED if available (PC13 typical)
        HAL_Delay(1000);
    }
}
