/**
 * @file nanomodbus_port.h
 * @brief nanoMODBUS STM32 HAL Porting Layer
 * 
 * This file provides the STM32 HAL porting layer for nanoMODBUS.
 * Supports both RTU (UART) and TCP transports.
 */

#ifndef NANOMODBUS_PORT_H
#define NANOMODBUS_PORT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "csrc/nanomodbus.h"

/* Configuration */
#define NMBS_COIL_BUF_SIZE 256       /* Max coils storage (bits) */
#define NMBS_REG_BUF_SIZE  256       /* Max registers storage (16-bit) */

/**
 * @brief Modbus server data storage structure
 */
typedef struct {
    uint8_t unit_id;                       /* RTU Unit ID (1-247) */
    uint8_t coils[NMBS_COIL_BUF_SIZE / 8]; /* Coils storage */
    uint16_t regs[NMBS_REG_BUF_SIZE];      /* Registers storage */
} nmbs_server_data_t;

/**
 * @brief Initialize nanoMODBUS server instance (RTU mode, bare-metal)
 * 
 * @param nmbs Pointer to nmbs_t instance
 * @param server_data Pointer to server data storage
 * @param unit_id RTU unit ID (1-247)
 * @param huart Pointer to UART handle for RTU communication
 * @return nmbs_error NMBS_ERROR_NONE if successful
 */
nmbs_error nmbs_server_init_rtu(nmbs_t* nmbs, nmbs_server_data_t* server_data, 
                                 uint8_t unit_id, void* huart);

/**
 * @brief Initialize nanoMODBUS client instance (RTU mode, bare-metal)
 * 
 * @param nmbs Pointer to nmbs_t instance
 * @param huart Pointer to UART handle for RTU communication
 * @return nmbs_error NMBS_ERROR_NONE if successful
 */
nmbs_error nmbs_client_init_rtu(nmbs_t* nmbs, void* huart);

#ifdef __cplusplus
}
#endif

#endif /* NANOMODBUS_PORT_H */
