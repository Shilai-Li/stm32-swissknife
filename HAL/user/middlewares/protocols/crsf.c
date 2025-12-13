#include "crsf.h"
#include <string.h>

// --- Configuration Macros ---
#define CRSF_TIME_NEEDED_PER_FRAME_US   1750 // Max time needed for one frame transmission
#define CRSF_BAUDRATE                   420000

// --- Protocol Constants Definition (if you don't have crsf_protocol.h) ---
#define CRSF_SYNC_BYTE 0xC8
#define CRSF_FRAMETYPE_RC_CHANNELS_PACKED 0x16
#define CRSF_ADDRESS_FLIGHT_CONTROLLER 0xC8

// Internal Structure Definitions
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

// --- Static Variables ---
static crsfFrame_t crsfFrame;
static uint8_t crsfFramePosition = 0;
static uint32_t crsfFrameStartAtUs = 0;
static uint16_t crsfChannels[CRSF_CHANNEL_COUNT];
static uint32_t lastFrameTimeUs = 0;

// --- Internal Function: CRC8 Calculation ---
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

// --- Internal Function: Scale Channel Value ---
static uint16_t crsf_scale_channel(uint16_t raw) {
    // Linear mapping: 11bit (0-2047) -> PWM (1000-2000)
    // CRSF Protocol: 172 -> 988us, 1811 -> 2012us
    return (uint16_t)((raw * 0.62477120195241f) + 881);
}

// --- External Interface Implementation ---

void crsf_init(void) {
    crsfFramePosition = 0;
    memset(crsfChannels, 0, sizeof(crsfChannels));
}

void crsf_process_byte(uint8_t c, uint32_t time_us) {
    // 1. Timeout detection: if byte interval is too long, reset receive index
    if ((time_us - crsfFrameStartAtUs) > CRSF_TIME_NEEDED_PER_FRAME_US) {
        crsfFramePosition = 0;
    }

    if (crsfFramePosition == 0) {
        crsfFrameStartAtUs = time_us;
    }

    // 2. Prevent buffer overflow
    if (crsfFramePosition < CRSF_FRAME_SIZE_MAX) {
        crsfFrame.bytes[crsfFramePosition++] = c;

        // 3. Only determine frame length after receiving at least 3 bytes (Addr, Len, Type)
        if (crsfFramePosition > 2) {
            // frameLength field value does not include Address(1) and FrameLength(1) itself
            uint8_t totalFrameLen = crsfFrame.frame.frameLength + 2;

            // 4. Receive complete frame
            if (crsfFramePosition >= totalFrameLen) {
                crsfFramePosition = 0; // Reset, prepare for next frame

                // 5. CRC Check
                // CRC Range: From Type to end of Payload
                uint8_t calculatedCRC = 0;
                calculatedCRC = crc8_calc(calculatedCRC, crsfFrame.frame.type);

                // Payload Length = Total Length - Addr(1) - Len(1) - Type(1) - CRC(1) = frameLength - 2
                int payloadLen = crsfFrame.frame.frameLength - 2;
                for (int i = 0; i < payloadLen; i++) {
                    calculatedCRC = crc8_calc(calculatedCRC, crsfFrame.frame.payload[i]);
                }

                // Compare CRC
                if (calculatedCRC == crsfFrame.bytes[totalFrameLen - 1]) {
                    // 6. Check passed, parse RC packet
                    if (crsfFrame.frame.deviceAddress == CRSF_ADDRESS_FLIGHT_CONTROLLER &&
                        crsfFrame.frame.type == CRSF_FRAMETYPE_RC_CHANNELS_PACKED) {

                        lastFrameTimeUs = time_us; // Update last active time

                        const crsfPayloadRcChannelsPacked_t* rc = (const crsfPayloadRcChannelsPacked_t*)crsfFrame.frame.payload;

                        crsfChannels[0] = crsf_scale_channel(rc->chan0);
                        crsfChannels[1] = crsf_scale_channel(rc->chan1);
                        crsfChannels[2] = crsf_scale_channel(rc->chan2);
                        crsfChannels[3] = crsf_scale_channel(rc->chan3);
                        crsfChannels[4] = crsf_scale_channel(rc->chan4);
                        crsfChannels[5] = crsf_scale_channel(rc->chan5);
                        crsfChannels[6] = crsf_scale_channel(rc->chan6);
                        crsfChannels[7] = crsf_scale_channel(rc->chan7);
                        // ... other channels similarly
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
    // Assuming disconnected if no data received for 500ms, you need to provide a function to get current time
    // Or check lastFrameTimeUs externally
    return 1; // Simply return 1, actual usage suggests checking time difference
}