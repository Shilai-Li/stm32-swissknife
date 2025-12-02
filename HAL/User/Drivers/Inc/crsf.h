#ifndef CRSF_H
#define CRSF_H
#include <stdint.h>
// 包含协议定义，如果你把协议定义放在了单独的 crsf_protocol.h 里，请取消注释下一行
// #include "crsf_protocol.h"
// 如果你不想用单独的 crsf_protocol.h，可以直接在这里定义核心常量
#ifndef CRSF_PROTOCOL_H
#define CRSF_FRAME_SIZE_MAX 64
#define CRSF_CHANNEL_COUNT 16
#endif
// --- 外部接口函数 ---
/**
 * @brief  初始化 CRSF (如果需要初始化变量)
 */
void crsf_init(void);
/**
 * @brief  处理接收到的字节 (在串口中断中调用)
 * @param  c: 接收到的字节
 * @param  time_us: 当前系统时间(微秒)，用于帧超时判断
 */
void crsf_process_byte(uint8_t c, uint32_t time_us);
/**
 * @brief  获取通道值
 * @param  channel: 通道索引 (0-15)
 * @return PWM值 (通常在 1000 - 2000 之间)
 */
uint16_t crsf_get_channel(int channel);
/**
 * @brief  检查是否已连接 (最近是否收到有效帧)
 * @return 1: 已连接, 0: 未连接
 */
int crsf_is_connected(void);
#endif // CRSF_H