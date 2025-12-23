/**
 * @file easylogger_tests.c
 * @brief EasyLogger 功能测试
 * 
 * 测试 EasyLogger 的各种功能：
 * - 不同级别的日志输出
 * - 彩色输出
 * - 时间戳
 * - 标签过滤
 */

#include "main.h"
#include "uart.h"

/* 定义模块标签 - 必须在 include elog.h 之前定义 */
#define LOG_TAG "main"
#include "elog.h"

void User_Entry(void) {
    // 1. 初始化 UART 驱动
    UART_Init();
    
    // 2. 初始化 EasyLogger
    elog_init();
    
    // 3. 设置日志格式 (启用各种信息)
    // 可选: ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME | ELOG_FMT_P_INFO | ELOG_FMT_T_INFO | ELOG_FMT_DIR | ELOG_FMT_FUNC | ELOG_FMT_LINE
    elog_set_fmt(ELOG_LVL_ASSERT, ELOG_FMT_ALL);
    elog_set_fmt(ELOG_LVL_ERROR, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
    elog_set_fmt(ELOG_LVL_WARN, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
    elog_set_fmt(ELOG_LVL_INFO, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
    elog_set_fmt(ELOG_LVL_DEBUG, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
    elog_set_fmt(ELOG_LVL_VERBOSE, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
    
    // 4. 启动 EasyLogger
    elog_start();
    
    // 5. 打印欢迎信息
    log_i("===========================================");
    log_i("  EasyLogger Demo - STM32 SwissKnife");
    log_i("===========================================");
    log_i("EasyLogger initialized successfully!");
    
    // 6. 演示不同级别的日志
    log_a("This is an ASSERT message (highest priority)");
    log_e("This is an ERROR message");
    log_w("This is a WARNING message");
    log_i("This is an INFO message");
    log_d("This is a DEBUG message");
    log_v("This is a VERBOSE message (lowest priority)");
    
    // 7. 演示带变量的日志
    int sensor_value = 42;
    float temperature = 25.5f;
    log_i("Sensor value: %d", sensor_value);
    // 浮点数打印：手动拆分整数和小数部分（避免使用 %f）
    int temp_int = (int)temperature;
    int temp_dec = (int)((temperature - temp_int) * 10);  // 1位小数
    log_i("Temperature: %d.%d C", temp_int, temp_dec);
    
    // 8. 主循环 - 周期性日志输出
    uint32_t counter = 0;
    uint32_t last_log_time = 0;
    
    while (1) {
        UART_Poll();  // 驱动 UART 状态机
        
        uint32_t now = HAL_GetTick();
        
        // 每2秒输出一次日志
        if (now - last_log_time >= 2000) {
            last_log_time = now;
            counter++;
            
            log_i("Heartbeat #%lu, uptime: %lu.%03lu s", 
                  counter, 
                  now / 1000, 
                  now % 1000);
            
            // 模拟不同级别的事件
            if (counter % 5 == 0) {
                log_w("Warning: Counter reached %lu", counter);
            }
            if (counter % 10 == 0) {
                log_e("Error simulation at count %lu", counter);
            }
        }
    }
}
