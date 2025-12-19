/**
 * @file u8g2_port.h
 * @brief U8G2 Hardware Abstraction Layer for STM32 HAL
 */

#ifndef U8G2_PORT_H
#define U8G2_PORT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "u8g2.h"

/* 
 * Hardware Handles (External)
 * You must define these in your main.c or pass them appropriately
 */
extern SPI_HandleTypeDef hspi1; // Or hspi2, adjust as needed
extern I2C_HandleTypeDef hi2c1; // Or hi2c2

/*
 * GPIO/Delay Callback
 * Handles Reset, DC, CS pins and delays.
 */
uint8_t u8x8_gpio_and_delay_stm32(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);

/*
 * Hardware SPI Byte Callback
 * Handles actual SPI transmission.
 */
uint8_t u8x8_byte_stm32_hw_spi(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);

/*
 * Hardware I2C Byte Callback
 * Handles actual I2C transmission.
 */
uint8_t u8x8_byte_stm32_hw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);

/**
 * @brief Helper to Init U8G2 for SSD1306 I2C (128x64 Noname)
 * @param u8g2 Pointer to u8g2 struct
 */
void U8G2_Init_SSD1306_I2C(u8g2_t *u8g2);

/**
 * @brief Helper to Init U8G2 for SSD1306 SPI (128x64 Noname)
 * @param u8g2 Pointer to u8g2 struct
 * @param cs_port GPIO Port for CS
 * @param cs_pin  GPIO Pin for CS
 * @param dc_port GPIO Port for DC
 * @param dc_pin  GPIO Pin for DC
 * @param rst_port GPIO Port for RST
 * @param rst_pin  GPIO Pin for RST
 */
void U8G2_Init_SSD1306_SPI(u8g2_t *u8g2, 
                           GPIO_TypeDef* cs_port, uint16_t cs_pin,
                           GPIO_TypeDef* dc_port, uint16_t dc_pin,
                           GPIO_TypeDef* rst_port, uint16_t rst_pin);

#ifdef __cplusplus
}
#endif

#endif
