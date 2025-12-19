/**
 * @file elog_port.h
 * @brief EasyLogger 移植层头文件
 * 
 * 定义 EasyLogger 移植接口，用于 STM32 HAL + UART 驱动。
 */

#ifndef __ELOG_PORT_H__
#define __ELOG_PORT_H__

#include <stddef.h>
#include "elog.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief EasyLogger 端口初始化
 * @return ELOG_NO_ERR 成功，其他值表示错误
 */
ElogErrCode elog_port_init(void);

/**
 * @brief 输出日志到硬件
 * @param log 日志字符串
 * @param size 日志长度
 */
void elog_port_output(const char *log, size_t size);

/**
 * @brief 输出锁定（进入临界区）
 */
void elog_port_output_lock(void);

/**
 * @brief 输出解锁（退出临界区）
 */
void elog_port_output_unlock(void);

/**
 * @brief 获取当前时间字符串
 * @return 时间字符串（格式：秒.毫秒）
 */
const char *elog_port_get_time(void);

/**
 * @brief 获取进程信息（裸机环境返回空字符串）
 * @return 进程信息字符串
 */
const char *elog_port_get_p_info(void);

/**
 * @brief 获取线程信息（裸机环境返回空字符串）
 * @return 线程信息字符串
 */
const char *elog_port_get_t_info(void);

#ifdef __cplusplus
}
#endif

#endif /* __ELOG_PORT_H__ */
