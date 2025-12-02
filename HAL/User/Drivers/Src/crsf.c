#include "crsf.h"
#include <string.h>
// --- 配置宏 ---
#define CRSF_TIME_NEEDED_PER_FRAME_US   1750 // 一帧传输所需最大时间
#define CRSF_BAUDRATE                   420000
// --- 协议常量定义 (如果你没有 crsf_protocol.h) ---
#define CRSF_SYNC_BYTE 0xC8
#define CRSF_FRAMETYPE_RC_CHANNELS_PACKED 0x16
#define CRSF_ADDRESS_FLIGHT_CONTROLLER 0xC8
// 内部结构体定义
typedef struct {
    unsigned int chan0 : 11;
    unsigned int chan1 : 11;
    unsigned int chan2 : 11;
    unsigned int chan3 : 11;
    unsigned int chan4 : 11;
    unsigned int chan5 : 11;
    unsigned int chan6 : 11;
    unsigned int chan7 : 11;
    unsigned int chan8 : 11;
    unsigned int chan9 : 11;
    unsigned int chan10 : 11;
    unsigned int chan11 : 11;
    unsigned int chan12 : 11;
    unsigned int chan13 : 11;
    unsigned int chan14 : 11;
    unsigned int chan15 : 11;
} __attribute__ ((__packed__)) crsfPayloadRcChannelsPacked_t;
typedef struct {
    uint8_t deviceAddress;
    uint8_t frameLength;
    uint8_t type;
    uint8_t payload[CRSF_FRAME_SIZE_MAX - 4];
} crsfFrameDef_t;
typedef union {
    uint8_t bytes[CRSF_FRAME_SIZE_MAX];
    crsfFrameDef_t frame;
} crsfFrame_t;
// --- 静态变量 ---
static crsfFrame_t crsfFrame;
static uint8_t crsfFramePosition = 0;
static uint32_t crsfFrameStartAtUs = 0;
static uint16_t crsfChannels[CRSF_CHANNEL_COUNT];
static uint32_t lastFrameTimeUs = 0;
// --- 内部函数: CRC8 计算 ---
static uint8_t crc8_calc(uint8_t crc, unsigned char a) {
    crc ^= a;
    for (int ii = 0; ii < 8; ++ii) {
        if (crc & 0x80) {
            crc = (crc << 1) ^ 0xD5;
        } else {
            crc = crc << 1;
        }
    }
    return crc;
}
// --- 内部函数: 转换通道值 ---
static uint16_t crsf_scale_channel(uint16_t raw) {
    // 11bit (0-2047) -> PWM (1000-2000) 的线性映射
    // CRSF 协议: 172 -> 988us, 1811 -> 2012us
    return (uint16_t)((raw * 0.62477120195241f) + 881);
}
// --- 外部接口实现 ---
void crsf_init(void) {
    crsfFramePosition = 0;
    memset(crsfChannels, 0, sizeof(crsfChannels));
}
void crsf_process_byte(uint8_t c, uint32_t time_us) {
    // 1. 超时检测：如果字节间隔过长，重置接收索引
    if ((time_us - crsfFrameStartAtUs) > CRSF_TIME_NEEDED_PER_FRAME_US) {
        crsfFramePosition = 0;
    }
    if (crsfFramePosition == 0) {
        crsfFrameStartAtUs = time_us;
    }
    // 2. 防止缓冲区溢出
    if (crsfFramePosition < CRSF_FRAME_SIZE_MAX) {
        crsfFrame.bytes[crsfFramePosition++] = c;
        // 3. 只有收到至少3个字节(Addr, Len, Type)才能判断帧长
        if (crsfFramePosition > 2) {
            // frameLength 字段的值不包含 Address(1) 和 FrameLength(1) 本身
            uint8_t totalFrameLen = crsfFrame.frame.frameLength + 2;
            // 4. 接收完整一帧
            if (crsfFramePosition >= totalFrameLen) {
                crsfFramePosition = 0; // 重置，准备下一帧
                // 5. CRC 校验
                // CRC 范围：从 Type 开始 到 Payload 结束
                uint8_t calculatedCRC = 0;
                calculatedCRC = crc8_calc(calculatedCRC, crsfFrame.frame.type);

                // Payload 长度 = 总长 - Addr(1) - Len(1) - Type(1) - CRC(1) = frameLength - 2
                int payloadLen = crsfFrame.frame.frameLength - 2;
                for (int i = 0; i < payloadLen; i++) {
                    calculatedCRC = crc8_calc(calculatedCRC, crsfFrame.frame.payload[i]);
                }
                // 比较 CRC
                if (calculatedCRC == crsfFrame.bytes[totalFrameLen - 1]) {
                    // 6. 校验通过，解析 RC 包
                    if (crsfFrame.frame.deviceAddress == CRSF_ADDRESS_FLIGHT_CONTROLLER &&
                        crsfFrame.frame.type == CRSF_FRAMETYPE_RC_CHANNELS_PACKED) {

                        lastFrameTimeUs = time_us; // 更新最后活跃时间

                        const crsfPayloadRcChannelsPacked_t* rc = (const crsfPayloadRcChannelsPacked_t*)crsfFrame.frame.payload;

                        crsfChannels[0] = crsf_scale_channel(rc->chan0);
                        crsfChannels[1] = crsf_scale_channel(rc->chan1);
                        crsfChannels[2] = crsf_scale_channel(rc->chan2);
                        crsfChannels[3] = crsf_scale_channel(rc->chan3);
                        crsfChannels[4] = crsf_scale_channel(rc->chan4);
                        crsfChannels[5] = crsf_scale_channel(rc->chan5);
                        crsfChannels[6] = crsf_scale_channel(rc->chan6);
                        crsfChannels[7] = crsf_scale_channel(rc->chan7);
                        // ... 其他通道同理
                    }
                }
            }
        }
    }
}
uint16_t crsf_get_channel(int channel) {
    if (channel < 0 || channel >= CRSF_CHANNEL_COUNT) return 0;
    return crsfChannels[channel];
}
int crsf_is_connected(void) {
    // 这里假设如果 500ms 没收到数据就算断开，你需要提供一个获取当前时间的函数
    // 或者你在外部自己判断 lastFrameTimeUs
    return 1; // 简单返回1，实际使用建议判断时间差
}