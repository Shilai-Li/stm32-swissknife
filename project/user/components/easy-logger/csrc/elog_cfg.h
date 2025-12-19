/*
 * This file is part of the EasyLogger Library.
 *
 * Copyright (c) 2015-2016, Armink, <armink.ztl@gmail.com>
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
 * Function: It is the configure head file for this library.
 * Created on: 2015-07-30
 * 
 * Modified for STM32 bare-metal (no RTOS) by stm32-swissknife project.
 */

#ifndef _ELOG_CFG_H_
#define _ELOG_CFG_H_

/*---------------------------------------------------------------------------*/
/* 基本配置 */
/*---------------------------------------------------------------------------*/

/* 启用日志输出 */
#define ELOG_OUTPUT_ENABLE

/* 静态输出日志级别: ELOG_LVL_ASSERT ~ ELOG_LVL_VERBOSE */
#define ELOG_OUTPUT_LVL                          ELOG_LVL_VERBOSE

/* 启用断言检查 */
#define ELOG_ASSERT_ENABLE

/* 每行日志的缓冲区大小 */
#define ELOG_LINE_BUF_SIZE                       256

/* 行号最大长度 */
#define ELOG_LINE_NUM_MAX_LEN                    5

/* 过滤标签最大长度 */
#define ELOG_FILTER_TAG_MAX_LEN                  16

/* 过滤关键字最大长度 */
#define ELOG_FILTER_KW_MAX_LEN                   16

/* 标签级别过滤最大数量 */
#define ELOG_FILTER_TAG_LVL_MAX_NUM              5

/* 换行符 (串口终端通常需要 \r\n) */
#define ELOG_NEWLINE_SIGN                        "\r\n"

/*---------------------------------------------------------------------------*/
/* 颜色配置 */
/*---------------------------------------------------------------------------*/

/* 启用日志颜色 (ANSI escape codes) */
#define ELOG_COLOR_ENABLE

/* 各级别颜色定义 */
#define ELOG_COLOR_ASSERT                        (F_MAGENTA B_NULL S_NORMAL)
#define ELOG_COLOR_ERROR                         (F_RED B_NULL S_NORMAL)
#define ELOG_COLOR_WARN                          (F_YELLOW B_NULL S_NORMAL)
#define ELOG_COLOR_INFO                          (F_CYAN B_NULL S_NORMAL)
#define ELOG_COLOR_DEBUG                         (F_GREEN B_NULL S_NORMAL)
#define ELOG_COLOR_VERBOSE                       (F_BLUE B_NULL S_NORMAL)

/*---------------------------------------------------------------------------*/
/* 异步输出配置 (裸机环境禁用) */
/*---------------------------------------------------------------------------*/

/* 禁用异步输出模式 - 裸机环境不支持 */
// #define ELOG_ASYNC_OUTPUT_ENABLE
// #define ELOG_ASYNC_OUTPUT_LVL                    ELOG_LVL_ASSERT
// #define ELOG_ASYNC_OUTPUT_BUF_SIZE               (ELOG_LINE_BUF_SIZE * 10)
// #define ELOG_ASYNC_LINE_OUTPUT
// #define ELOG_ASYNC_OUTPUT_USING_PTHREAD  /* 不使用 pthread */

/*---------------------------------------------------------------------------*/
/* 缓冲输出配置 (可选) */
/*---------------------------------------------------------------------------*/

/* 禁用缓冲输出模式 - 简化实现 */
// #define ELOG_BUF_OUTPUT_ENABLE
// #define ELOG_BUF_OUTPUT_BUF_SIZE                 (ELOG_LINE_BUF_SIZE * 10)

#endif /* _ELOG_CFG_H_ */
