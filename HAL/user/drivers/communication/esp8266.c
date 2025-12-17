/**
 * @file esp8266.c
 * @brief ESP8266 Wi-Fi Module Driver Implementation
 * @details Implements AT command interface with ring-buffer based UART
 */

#include "esp8266.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "delay.h" // Requires delay_ms implementation

#define ESP_RX_BUFFER_SIZE 256
#define CMD_DELAY_MS 50 

/* Internal Helper to Log Debug Messages */
static void ESP_Log(ESP8266_Handle_t *h, const char *fmt, ...) {
    if (h->config.debug_uart < UART_CHANNEL_MAX) {
        char buf[128];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);
        UART_SendString(h->config.debug_uart, buf);
    }
}

/**
 * @brief  Wait for a specific string sequence from UART
 * @details Reads char-by-char into a sliding window buffer to find match
 */
static ESP8266_Status_t WaitFor(ESP8266_Handle_t *h, const char *expected, uint32_t timeout) {
    uint32_t start = HAL_GetTick();
    uint32_t matched_len = 0;
    uint32_t expected_len = strlen(expected);
    
    // Simple state machine to match string on the fly
    // Not using a huge buffer to save RAM, just tracking progress
    
    // We also need to capture logs if debug is on, but that's hard without buffering whole line.
    // For now, just simplistic matching.
    
    while ((HAL_GetTick() - start) < timeout) {
        uint8_t c;
        if (UART_Read(h->config.cmd_uart, &c)) {
            // Echo to debug if enabled
            if (h->config.debug_uart < UART_CHANNEL_MAX) {
                 UART_Send(h->config.debug_uart, &c, 1);
            }

            if (c == expected[matched_len]) {
                matched_len++;
                if (matched_len == expected_len) {
                    return ESP8266_OK;
                }
            } else {
                // Mismatch, reset match index
                // Optimization: In a strict implementation KMP algo is better,
                // but for "OK" or "ERROR" simple reset is usually fine enough for AT
                // unless the pattern is redundant like "OOOK".
                if (c == expected[0]) matched_len = 1; // Retry start 
                else matched_len = 0;
            }
        }
    }
    
    return ESP8266_TIMEOUT;
}

/**
 * @brief Flush Input Buffer
 */
static void FlushRx(ESP8266_Handle_t *h) {
    uint8_t dummy;
    while(UART_Read(h->config.cmd_uart, &dummy));
}

ESP8266_Status_t ESP8266_Init(ESP8266_Handle_t *handle, const ESP8266_Config_t *config) {
    if (!handle || !config) return ESP8266_INVALID_ARGS;
    
    handle->config = *config;
    handle->initialized = false;

    // Flush any garbage
    FlushRx(handle);

    // 1. Check Communication (AT)
    if (ESP8266_SendCmd(handle, "AT\r\n", "OK", 1000) != ESP8266_OK) {
        ESP_Log(handle, "[ESP] AT Check Failed\r\n");
        return ESP8266_ERROR;
    }

    // 2. Disable Echo if requested
    if (config->echo_off) {
        ESP8266_SendCmd(handle, "ATE0\r\n", "OK", 500);
    }
    
    // 3. Set Station Mode Default
    // ESP8266_SetMode(handle, 1); 

    handle->initialized = true;
    ESP_Log(handle, "[ESP] Init Success\r\n");
    return ESP8266_OK;
}

ESP8266_Status_t ESP8266_Reset(ESP8266_Handle_t *handle) {
    ESP8266_SendCmd(handle, "AT+RST\r\n", "ready", 2000); // Wait for ready text
    // Sometimes ready doesn't come if echo is off? usually it prints junk then ready.
    delay_ms(500); 
    return ESP8266_OK;
}

ESP8266_Status_t ESP8266_SetMode(ESP8266_Handle_t *handle, uint8_t mode) {
    char cmd[16];
    sprintf(cmd, "AT+CWMODE=%d\r\n", mode);
    return ESP8266_SendCmd(handle, cmd, "OK", 1000);
}

ESP8266_Status_t ESP8266_JoinAP(ESP8266_Handle_t *handle, const char *ssid, const char *pwd) {
    char cmd[128];
    // AT+CWJAP="SSID","PWD"
    snprintf(cmd, sizeof(cmd), "AT+CWJAP=\"%s\",\"%s\"\r\n", ssid, pwd);
    
    // Connecting can take time
    ESP_Log(handle, "[ESP] Joining AP... %s\r\n", ssid);
    ESP8266_Status_t status = ESP8266_SendCmd(handle, cmd, "OK", 10000); // 10s timeout
    
    if (status != ESP8266_OK) {
        ESP_Log(handle, "[ESP] Join Failed\r\n");
    }
    return status;
}

ESP8266_Status_t ESP8266_ConnectTCP(ESP8266_Handle_t *handle, const char *ip, uint16_t port) {
    char cmd[64];
    // AT+CIPSTART="TCP","192.168.1.1",8080
    snprintf(cmd, sizeof(cmd), "AT+CIPSTART=\"TCP\",\"%s\",%d\r\n", ip, port);
    
    return ESP8266_SendCmd(handle, cmd, "OK", 5000);
}

ESP8266_Status_t ESP8266_Send(ESP8266_Handle_t *handle, const uint8_t *data, uint16_t len) {
    char cmd[32];
    // AT+CIPSEND=<len>
    snprintf(cmd, sizeof(cmd), "AT+CIPSEND=%d\r\n", len);
    
    if (ESP8266_SendCmd(handle, cmd, ">", 2000) != ESP8266_OK) {
        return ESP8266_ERROR; // Failed to enter send mode
    }
    
    // Now disable irq briefly? No, UART_Send is safe.
    UART_Send(handle->config.cmd_uart, data, len);
    
    // Wait for "SEND OK"
    return WaitFor(handle, "SEND OK", 3000);
}

ESP8266_Status_t ESP8266_SendCmd(ESP8266_Handle_t *handle, const char *cmd, const char *expected, uint32_t timeout_ms) {
    if (!handle) return ESP8266_INVALID_ARGS;
    
    FlushRx(handle); // Clear previous buffer
    UART_SendString(handle->config.cmd_uart, cmd);
    
    return WaitFor(handle, expected, timeout_ms);
}
