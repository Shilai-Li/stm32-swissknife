/**
 * @file esp8266.h
 * @brief ESP8266 Wi-Fi Module Driver
 * 
 * =================================================================================
 *                       >>> INTEGRATION GUIDE <<<
 * =================================================================================
 * 1. CubeMX Config (Connectivity -> USARTx):
 *    - Mode: Asynchronous
 *    - Baud Rate: 115200 (Default for ESP8266)
 *    - NVIC: Enable "USARTx global interrupt" (Critical for RingBuffer)
 * 
 * 2. Wiring:
 *    - STM32 TX -> ESP8266 RX
 *    - STM32 RX -> ESP8266 TX
 *    - ESP8266 CH_PD (EN) -> 3.3V
 *    - ESP8266 VCC -> 3.3V (Must provide >300mA current!)
 * 
 * 3. Driver Logic:
 *    This driver relies on 'uart.c' which provides RingBuffer.
 *    Ensure UART_Init() is called for both ESP channel and Debug channel.
 * =================================================================================
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

/* ============================================================================
 * MQTT API (Requires ESP8266 AT Firmware 2.0+)
 * ========================================================================= */

/**
 * @brief  Configure MQTT User Parameters
 * @param  client_id  MQTT Client ID (unique identifier)
 * @param  username   MQTT Username (NULL if not required)
 * @param  password   MQTT Password (NULL if not required)
 * @return ESP8266_OK on success
 */
ESP8266_Status_t ESP8266_MQTT_UserConfig(ESP8266_Handle_t *handle, 
                                          const char *client_id,
                                          const char *username,
                                          const char *password);

/**
 * @brief  Connect to MQTT Broker
 * @param  host  Broker hostname or IP address
 * @param  port  Broker port (usually 1883 for non-TLS)
 * @return ESP8266_OK on success
 */
ESP8266_Status_t ESP8266_MQTT_Connect(ESP8266_Handle_t *handle,
                                       const char *host,
                                       uint16_t port);

/**
 * @brief  Disconnect from MQTT Broker
 * @return ESP8266_OK on success
 */
ESP8266_Status_t ESP8266_MQTT_Disconnect(ESP8266_Handle_t *handle);

/**
 * @brief  Publish MQTT Message
 * @param  topic  MQTT Topic string
 * @param  data   Message payload
 * @param  qos    Quality of Service (0, 1, or 2)
 * @return ESP8266_OK on success
 */
ESP8266_Status_t ESP8266_MQTT_Publish(ESP8266_Handle_t *handle,
                                       const char *topic,
                                       const char *data,
                                       uint8_t qos);

/**
 * @brief  Subscribe to MQTT Topic
 * @param  topic  MQTT Topic string (supports wildcards: +, #)
 * @param  qos    Quality of Service (0, 1, or 2)
 * @return ESP8266_OK on success
 */
ESP8266_Status_t ESP8266_MQTT_Subscribe(ESP8266_Handle_t *handle,
                                         const char *topic,
                                         uint8_t qos);

/**
 * @brief  Unsubscribe from MQTT Topic
 * @param  topic  MQTT Topic string
 * @return ESP8266_OK on success
 */
ESP8266_Status_t ESP8266_MQTT_Unsubscribe(ESP8266_Handle_t *handle,
                                           const char *topic);

#ifdef __cplusplus
}
#endif

#endif // ESP8266_H
