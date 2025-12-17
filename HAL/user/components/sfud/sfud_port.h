/**
 * @file sfud_port.h
 * @brief SFUD Port Layer Header for STM32 HAL
 * @author stm32-swissknife Project
 * @date 2025
 * 
 * This file provides the port layer interface for SFUD on STM32.
 */

#ifndef SFUD_PORT_H
#define SFUD_PORT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "csrc/sfud.h"
#include "main.h"

/**
 * @brief Initialize SFUD with default hardware configuration
 * 
 * This is a convenience function that initializes SFUD with the
 * default Flash device defined in sfud_cfg.h
 * 
 * @return sfud_err SFUD_SUCCESS on success
 */
sfud_err SFUD_Port_Init(void);

/**
 * @brief Get default Flash device handle
 * 
 * @return const sfud_flash* Pointer to default Flash device
 */
const sfud_flash *SFUD_Port_GetDefaultFlash(void);

#ifdef __cplusplus
}
#endif

#endif /* SFUD_PORT_H */
