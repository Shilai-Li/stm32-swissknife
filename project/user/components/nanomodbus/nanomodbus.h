/**
 * @file nanomodbus.h
 * @brief nanoMODBUS Component Wrapper
 * 
 * This is a wrapper header for the nanoMODBUS library.
 * nanoMODBUS is a compact MODBUS RTU/TCP C library for microcontrollers.
 * 
 * GitHub: https://github.com/debevv/nanoMODBUS
 * License: MIT
 * 
 * Usage:
 *   #include "nanomodbus.h"
 */

#ifndef NANOMODBUS_WRAPPER_H
#define NANOMODBUS_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Include the core nanoMODBUS library */
#include "csrc/nanomodbus.h"

/* Include the STM32 HAL port layer */
#include "nanomodbus_port.h"

#ifdef __cplusplus
}
#endif

#endif /* NANOMODBUS_WRAPPER_H */
