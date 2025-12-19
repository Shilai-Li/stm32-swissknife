/**
 * @file TF_Config.h
 * @brief Configuration file for TinyFrame library
 * 
 * This file is required by TinyFrame to define its configuration.
 * It is included by TinyFrame.h.
 */

#ifndef TF_CONFIG_H
#define TF_CONFIG_H

#include <stdint.h>
#include <stdio.h>

//---------------------------------------------------------------------------
//  TinyFrame Data Types & Sizes
//---------------------------------------------------------------------------

// Field sizes (in bytes)
#define TF_ID_BYTES     1   // 1, 2, or 4
#define TF_LEN_BYTES    2   // 1, 2, or 4 (Use 2 for frames > 255 bytes)
#define TF_TYPE_BYTES   1   // 1, 2, or 4

// Checksum type (0 = NONE, 8 = XOR, 16 = CRC16, 32 = CRC32)
#define TF_CKSUM_TYPE   TF_CKSUM_CRC16

// Start of Frame byte
#define TF_SOF_BYTE     0x01

// Max listener counts
#define TF_MAX_ID_LST       5
#define TF_MAX_TYPE_LST     5
#define TF_MAX_GEN_LST      2

// Ticks and Counters
typedef uint32_t TF_TICKS;  // Type for timeouts
typedef uint8_t  TF_COUNT;  // Type for listener counters

// Parser timeout (how long to wait for the rest of frame)
#define TF_PARSER_TIMEOUT_TICKS  100 // ms if tick is 1ms

//---------------------------------------------------------------------------
//  Buffers
//---------------------------------------------------------------------------

// Max payloads
#define TF_MAX_PAYLOAD_TX   256
#define TF_MAX_PAYLOAD_RX   256

// Receive buffer size (internal to TinyFrame struct)
// Note: TF_MAX_PAYLOAD_RX is used for the buffer size in the struct

// Send buffer size
// This buffer is used to compose the frame before sending
// It must be large enough to hold:
// SOF(1) + ID(TF_ID_BYTES) + LEN(TF_LEN_BYTES) + TYPE(TF_TYPE_BYTES) + HEAD_CKSUM(1 or 2) + DATA(TF_MAX_PAYLOAD_TX) + DATA_CKSUM(1 or 2)
#define TF_SENDBUF_LEN (1 + TF_ID_BYTES + TF_LEN_BYTES + TF_TYPE_BYTES + 2 + TF_MAX_PAYLOAD_TX + 2)

//---------------------------------------------------------------------------
//  Features
//---------------------------------------------------------------------------

// Peer bit (for master/slave distinction)
#define TF_PEER     TF_MASTER // or TF_SLAVE

#define TF_USE_MUTEX    0
#define TF_ERROR_CALLBACKS 0

#endif // TF_CONFIG_H
