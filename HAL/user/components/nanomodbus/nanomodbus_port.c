/**
 * @file nanomodbus_port.c
 * @brief nanoMODBUS STM32 HAL Porting Layer Implementation (Bare-Metal)
 */

#include "nanomodbus_port.h"
#include "stm32f1xx_hal.h"
#include <string.h>

/* Global server data pointer for callbacks */
static nmbs_server_data_t* g_server_data = NULL;

/* UART handle for transport */
static UART_HandleTypeDef* g_huart = NULL;

/* ========================================================================
 * Platform Transport Functions (UART RTU)
 * ======================================================================== */

/**
 * @brief UART read function for nanoMODBUS
 * @param buf Buffer to store received data
 * @param count Number of bytes to read
 * @param byte_timeout_ms Timeout in milliseconds
 * @param arg User argument (UART handle)
 * @return Number of bytes actually read, or <0 on error
 */
static int32_t platform_read_uart(uint8_t* buf, uint16_t count, int32_t byte_timeout_ms, void* arg) {
    UART_HandleTypeDef* huart = (UART_HandleTypeDef*)arg;
    
    /* Handle infinite timeout */
    uint32_t timeout = (byte_timeout_ms < 0) ? HAL_MAX_DELAY : (uint32_t)byte_timeout_ms;
    
    /* Blocking receive */
    HAL_StatusTypeDef status = HAL_UART_Receive(huart, buf, count, timeout);
    
    if (status == HAL_OK) {
        return count;  /* All bytes received */
    } else if (status == HAL_TIMEOUT) {
        return 0;  /* Timeout, partial data */
    } else {
        return -1;  /* Transport error */
    }
}

/**
 * @brief UART write function for nanoMODBUS
 * @param buf Buffer containing data to send
 * @param count Number of bytes to write
 * @param byte_timeout_ms Timeout in milliseconds
 * @param arg User argument (UART handle)
 * @return Number of bytes actually written, or <0 on error
 */
static int32_t platform_write_uart(const uint8_t* buf, uint16_t count, int32_t byte_timeout_ms, void* arg) {
    UART_HandleTypeDef* huart = (UART_HandleTypeDef*)arg;
    
    /* Handle infinite timeout */
    uint32_t timeout = (byte_timeout_ms < 0) ? HAL_MAX_DELAY : (uint32_t)byte_timeout_ms;
    
    /* Blocking transmit */
    HAL_StatusTypeDef status = HAL_UART_Transmit(huart, (uint8_t*)buf, count, timeout);
    
    if (status == HAL_OK) {
        return count;  /* All bytes sent */
    } else if (status == HAL_TIMEOUT) {
        return 0;  /* Timeout, partial data */
    } else {
        return -1;  /* Transport error */
    }
}

/* ========================================================================
 * Server Callbacks (Data Access Functions)
 * ======================================================================== */

/**
 * @brief Server callback: Read Coils (FC 01)
 */
static nmbs_error server_read_coils(uint16_t address, uint16_t quantity, 
                                     nmbs_bitfield coils_out, uint8_t unit_id, void* arg) {
    if (!g_server_data || unit_id != g_server_data->unit_id) {
        return NMBS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
    }
    
    /* Check address bounds */
    if (address + quantity > NMBS_COIL_BUF_SIZE) {
        return NMBS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
    }
    
    /* Copy coils to output */
    for (uint16_t i = 0; i < quantity; i++) {
        bool bit = nmbs_bitfield_read(g_server_data->coils, address + i);
        nmbs_bitfield_write(coils_out, i, bit);
    }
    
    return NMBS_ERROR_NONE;
}

/**
 * @brief Server callback: Read Holding Registers (FC 03)
 */
static nmbs_error server_read_holding_registers(uint16_t address, uint16_t quantity, 
                                                  uint16_t* registers_out, uint8_t unit_id, void* arg) {
    if (!g_server_data || unit_id != g_server_data->unit_id) {
        return NMBS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
    }
    
    /* Check address bounds */
    if (address + quantity > NMBS_REG_BUF_SIZE) {
        return NMBS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
    }
    
    /* Copy registers to output */
    memcpy(registers_out, &g_server_data->regs[address], quantity * sizeof(uint16_t));
    
    return NMBS_ERROR_NONE;
}

/**
 * @brief Server callback: Write Single Coil (FC 05)
 */
static nmbs_error server_write_single_coil(uint16_t address, bool value, uint8_t unit_id, void* arg) {
    if (!g_server_data || unit_id != g_server_data->unit_id) {
        return NMBS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
    }
    
    /* Check address bounds */
    if (address >= NMBS_COIL_BUF_SIZE) {
        return NMBS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
    }
    
    /* Write coil */
    nmbs_bitfield_write(g_server_data->coils, address, value);
    
    return NMBS_ERROR_NONE;
}

/**
 * @brief Server callback: Write Single Register (FC 06)
 */
static nmbs_error server_write_single_register(uint16_t address, uint16_t value, uint8_t unit_id, void* arg) {
    if (!g_server_data || unit_id != g_server_data->unit_id) {
        return NMBS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
    }
    
    /* Check address bounds */
    if (address >= NMBS_REG_BUF_SIZE) {
        return NMBS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
    }
    
    /* Write register */
    g_server_data->regs[address] = value;
    
    return NMBS_ERROR_NONE;
}

/**
 * @brief Server callback: Write Multiple Coils (FC 15)
 */
static nmbs_error server_write_multiple_coils(uint16_t address, uint16_t quantity, 
                                                const nmbs_bitfield coils, uint8_t unit_id, void* arg) {
    if (!g_server_data || unit_id != g_server_data->unit_id) {
        return NMBS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
    }
    
    /* Check address bounds */
    if (address + quantity > NMBS_COIL_BUF_SIZE) {
        return NMBS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
    }
    
    /* Write coils */
    for (uint16_t i = 0; i < quantity; i++) {
        bool bit = nmbs_bitfield_read(coils, i);
        nmbs_bitfield_write(g_server_data->coils, address + i, bit);
    }
    
    return NMBS_ERROR_NONE;
}

/**
 * @brief Server callback: Write Multiple Registers (FC 16)
 */
static nmbs_error server_write_multiple_registers(uint16_t address, uint16_t quantity, 
                                                    const uint16_t* registers, uint8_t unit_id, void* arg) {
    if (!g_server_data || unit_id != g_server_data->unit_id) {
        return NMBS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
    }
    
    /* Check address bounds */
    if (address + quantity > NMBS_REG_BUF_SIZE) {
        return NMBS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
    }
    
    /* Write registers */
    memcpy(&g_server_data->regs[address], registers, quantity * sizeof(uint16_t));
    
    return NMBS_ERROR_NONE;
}

/* ========================================================================
 * Public Initialization Functions
 * ======================================================================== */

nmbs_error nmbs_server_init_rtu(nmbs_t* nmbs, nmbs_server_data_t* server_data, 
                                  uint8_t unit_id, void* huart) {
    if (!nmbs || !server_data || !huart) {
        return NMBS_ERROR_INVALID_ARGUMENT;
    }
    
    /* Store global references */
    g_server_data = server_data;
    g_huart = (UART_HandleTypeDef*)huart;
    
    /* Initialize server data */
    memset(server_data, 0, sizeof(nmbs_server_data_t));
    server_data->unit_id = unit_id;
    
    /* Configure platform */
    nmbs_platform_conf platform;
    nmbs_platform_conf_create(&platform);
    platform.transport = NMBS_TRANSPORT_RTU;
    platform.read = platform_read_uart;
    platform.write = platform_write_uart;
    platform.arg = huart;
    
    /* Configure callbacks */
    nmbs_callbacks callbacks;
    nmbs_callbacks_create(&callbacks);
    callbacks.read_coils = server_read_coils;
    callbacks.read_holding_registers = server_read_holding_registers;
    callbacks.write_single_coil = server_write_single_coil;
    callbacks.write_single_register = server_write_single_register;
    callbacks.write_multiple_coils = server_write_multiple_coils;
    callbacks.write_multiple_registers = server_write_multiple_registers;
    
    /* Create server */
    nmbs_error err = nmbs_server_create(nmbs, unit_id, &platform, &callbacks);
    if (err != NMBS_ERROR_NONE) {
        return err;
    }
    
    /* Set timeouts (can be adjusted) */
    nmbs_set_byte_timeout(nmbs, 100);   /* 100ms byte timeout */
    nmbs_set_read_timeout(nmbs, 1000);  /* 1s read timeout */
    
    return NMBS_ERROR_NONE;
}

nmbs_error nmbs_client_init_rtu(nmbs_t* nmbs, void* huart) {
    if (!nmbs || !huart) {
        return NMBS_ERROR_INVALID_ARGUMENT;
    }
    
    /* Store UART handle */
    g_huart = (UART_HandleTypeDef*)huart;
    
    /* Configure platform */
    nmbs_platform_conf platform;
    nmbs_platform_conf_create(&platform);
    platform.transport = NMBS_TRANSPORT_RTU;
    platform.read = platform_read_uart;
    platform.write = platform_write_uart;
    platform.arg = huart;
    
    /* Create client */
    nmbs_error err = nmbs_client_create(nmbs, &platform);
    if (err != NMBS_ERROR_NONE) {
        return err;
    }
    
    /* Set timeouts (can be adjusted) */
    nmbs_set_byte_timeout(nmbs, 100);   /* 100ms byte timeout */
    nmbs_set_read_timeout(nmbs, 1000);  /* 1s read timeout */
    
    return NMBS_ERROR_NONE;
}
