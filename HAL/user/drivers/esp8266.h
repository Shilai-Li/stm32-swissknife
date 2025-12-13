/**
 * @file esp8266.h
 * @brief ESP8266 Wi-Fi Module Driver
 * @author Standard Implementation
 * @date 2024
 * @note Requires `uart.h` driver for communication
 */

#ifndef ESP8266_H
#define ESP8266_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "uart.h"

/**
 * @brief ESP8266 Return Status
 */
typedef enum {
    ESP8266_OK = 0,
    ESP8266_ERROR,
    ESP8266_TIMEOUT,
    ESP8266_BUSY,
    ESP8266_INVALID_ARGS
} ESP8266_Status_t;

/**
 * @brief ESP8266 Configuration Structure
 */
typedef struct {
    UART_Channel cmd_uart;      /*!< UART Channel for AT Commands */
    UART_Channel debug_uart;    /*!< UART Channel for Driver Debug Logs (optional, set to -1 if unused) */
    uint32_t timeout_ms;        /*!< Default timeout for commands */
    bool echo_off;              /*!< If true, turns off echo (ATE0) during init */
} ESP8266_Config_t;

/**
 * @brief ESP8266 Handle Structure
 */
typedef struct {
    ESP8266_Config_t config;
    bool initialized;
} ESP8266_Handle_t;

/* ============================================================================
 * Public API
 * ========================================================================= */

/**
 * @brief  Initialize the ESP8266 Driver
 * @param  handle Pointer to handle structure
 * @param  config Pointer to configuration
 * @return ESP8266_OK on success
 */
ESP8266_Status_t ESP8266_Init(ESP8266_Handle_t *handle, const ESP8266_Config_t *config);

/**
 * @brief  Hardware Reset (rst pin) or Soft Reset (AT+RST)
 * @note   This driver currently implements Soft Reset.
 * @return ESP8266_OK on success
 */
ESP8266_Status_t ESP8266_Reset(ESP8266_Handle_t *handle);

/**
 * @brief  Set Wi-Fi Mode (Station, SoftAP, Station+SoftAP)
 * @param  mode 1=Station, 2=SoftAP, 3=Both
 * @return ESP8266_OK on success
 */
ESP8266_Status_t ESP8266_SetMode(ESP8266_Handle_t *handle, uint8_t mode);

/**
 * @brief  Join an Access Point
 * @param  ssid Network SSID
 * @param  pwd  Password
 * @return ESP8266_OK on success
 */
ESP8266_Status_t ESP8266_JoinAP(ESP8266_Handle_t *handle, const char *ssid, const char *pwd);

/**
 * @brief  Establish TCP Connection
 * @param  ip   Target IP String (e.g. "192.168.1.10")
 * @param  port Target Port
 * @return ESP8266_OK on success
 */
ESP8266_Status_t ESP8266_ConnectTCP(ESP8266_Handle_t *handle, const char *ip, uint16_t port);

/**
 * @brief  Send Data over active connection
 * @param  data Pointer to data
 * @param  len  Length of data
 * @return ESP8266_OK on success
 */
ESP8266_Status_t ESP8266_Send(ESP8266_Handle_t *handle, const uint8_t *data, uint16_t len);

/**
 * @brief  Send Raw AT Command and wait for expected response
 * @param  cmd Raw command string (including \r\n)
 * @param  expected Expected response substring (e.g. "OK")
 * @param  timeout_ms Override default timeout
 * @return ESP8266_OK on match, TIMEOUT otherwise
 */
ESP8266_Status_t ESP8266_SendCmd(ESP8266_Handle_t *handle, const char *cmd, const char *expected, uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif // ESP8266_H
