/*
 * This file is part of the EasyLogger Library.
 *
 * Copyright (c) 2015, Armink, <armink.ztl@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Function: Portable interface for STM32 HAL + UART driver.
 * Created on: 2015-04-28
 * Modified for stm32-swissknife project.
 */

#include <elog.h>
#include "main.h"
#include "drivers/uart.h"

/* 时间缓冲区 */
static char time_buf[16];

/**
 * EasyLogger port initialize
 *
 * @return result
 */
ElogErrCode elog_port_init(void) {
    ElogErrCode result = ELOG_NO_ERR;
    
    /* UART 已在 UART_Init() 中初始化，这里不需要额外操作 */
    
    return result;
}

/**
 * output log port interface
 *
 * @param log output of log
 * @param size log size
 */
void elog_port_output(const char *log, size_t size) {
    /* 使用 UART 驱动发送日志 */
    UART_Send(UART_DEBUG_CHANNEL, (const uint8_t *)log, (uint16_t)size);
}

/**
 * output lock - 禁用中断防止日志输出被打断
 */
void elog_port_output_lock(void) {
    __disable_irq();
}

/**
 * output unlock - 恢复中断
 */
void elog_port_output_unlock(void) {
    __enable_irq();
}

/**
 * get current time interface
 *
 * @return current time string
 */
const char *elog_port_get_time(void) {
    uint32_t tick = HAL_GetTick();
    uint32_t sec = tick / 1000;
    uint32_t ms = tick % 1000;
    
    /* 格式: "123.456" 秒.毫秒 */
    snprintf(time_buf, sizeof(time_buf), "%lu.%03lu", sec, ms);
    
    return time_buf;
}

/**
 * get current process name interface
 *
 * @return current process name
 */
const char *elog_port_get_p_info(void) {
    /* 裸机环境没有进程，返回空字符串 */
    return "";
}

/**
 * get current thread name interface
 *
 * @return current thread name
 */
const char *elog_port_get_t_info(void) {
    /* 裸机环境没有线程，返回空字符串 */
    return "";
}