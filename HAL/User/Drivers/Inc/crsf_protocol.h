#ifndef CRSF_PROTOCOL_H
#define CRSF_PROTOCOL_H

#include <stdint.h>

#define CRSF_BAUDRATE 420000
#define CRSF_SYNC_BYTE 0xC8
#define CRSF_FRAME_SIZE_MAX 64

// Frame Types
#define CRSF_FRAMETYPE_RC_CHANNELS_PACKED 0x16
#define CRSF_FRAMETYPE_LINK_STATISTICS 0x14

// Device Addresses
#define CRSF_ADDRESS_FLIGHT_CONTROLLER 0xC8

// Channel Definitions
#define CRSF_CHANNEL_COUNT 16

// Packed Channel Data Structure (11 bits per channel)
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

// General Frame Structure
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

#endif