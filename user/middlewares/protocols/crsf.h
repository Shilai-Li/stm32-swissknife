#ifndef CRSF_H
#define CRSF_H

#include <stdint.h>

// Include protocol definition. If you have a separate crsf_protocol.h, uncomment the next line.
// #include "crsf_protocol.h"

// If you don't want to use a separate crsf_protocol.h, you can define core constants here.
#ifndef CRSF_PROTOCOL_H
#define CRSF_FRAME_SIZE_MAX 64
#define CRSF_CHANNEL_COUNT 16
#endif

// --- External Interface Functions ---

/**
 * @brief  Initialize CRSF (if variable initialization is needed)
 */
void crsf_init(void);

/**
 * @brief  Process received byte (call this in UART interrupt or polling loop)
 * @param  c: Received byte
 * @param  time_us: Current system time (microseconds), used for frame timeout judgment
 */
void crsf_process_byte(uint8_t c, uint32_t time_us);

/**
 * @brief  Get channel value
 * @param  channel: Channel index (0-15)
 * @return PWM value (usually between 1000 - 2000)
 */
uint16_t crsf_get_channel(int channel);

/**
 * @brief  Check if connected (whether a valid frame has been received recently)
 * @return 1: Connected, 0: Disconnected
 */
int crsf_is_connected(void);

#endif // CRSF_H
