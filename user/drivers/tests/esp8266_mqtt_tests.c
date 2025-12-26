/**
 * @file esp8266_mqtt_tests.c
 * @brief ESP8266 MQTT AT Command Test Cases
 * 
 * CubeMX Configuration Required:
 * ================================
 * 
 * 1. USART1 (ESP8266 Communication):
 *    - Mode: Asynchronous
 *    - Baud Rate: 115200
 *    - Word Length: 8 Bits
 *    - Parity: None
 *    - Stop Bits: 1
 *    - Enable USART1 global interrupt
 * 
 * 2. USART2 (Debug Output):
 *    - Same configuration as USART1
 * 
 * Wiring:
 * =======
 * ESP-12E:
 *    - STM32 TX (USART1) → ESP-12E RX
 *    - STM32 RX (USART1) ← ESP-12E TX
 *    - ESP-12E CH_PD (EN) → 3.3V
 *    - ESP-12E VCC → 3.3V (>300mA current!)
 *    - ESP-12E GND → GND
 * 
 * Test Modes:
 * ===========
 * - TEST_MQTT_PUBLISH: Publish sensor data to MQTT broker
 * - TEST_MQTT_SUBSCRIBE: Subscribe and receive messages
 */

#include "stm32f1xx_hal.h"
#include "esp8266.h"
#include "uart.h"
#include "delay.h"
#include <stdio.h>
#include <string.h>

/* Test mode selection (set ONE to 1) */
#define TEST_MQTT_PUBLISH   1
#define TEST_MQTT_SUBSCRIBE 0

/* External UART handles */
extern UART_HandleTypeDef huart1;  // ESP8266
extern UART_HandleTypeDef huart2;  // Debug

/* MQTT Configuration */
#define WIFI_SSID           "YourWiFiSSID"      // <-- 修改为你的WiFi名
#define WIFI_PASSWORD       "YourPassword"      // <-- 修改为你的WiFi密码
#define MQTT_BROKER         "broker.hivemq.com" // 免费公共MQTT Broker
#define MQTT_PORT           1883
#define MQTT_CLIENT_ID      "STM32_ESP8266_Test"
#define MQTT_USERNAME       NULL                // HiveMQ不需要认证
#define MQTT_PASSWORD       NULL

/* Print helper */
#define PRINT(fmt, ...) do { \
    char buf[128]; \
    snprintf(buf, sizeof(buf), fmt "\r\n", ##__VA_ARGS__); \
    UART_Send(UART_DEBUG_CHANNEL, (uint8_t*)buf, strlen(buf)); \
} while(0)

static ESP8266_Handle_t esp;

/* ========================================================================
 * Helper: Initialize ESP8266 and Connect to WiFi + MQTT
 * ======================================================================== */
static bool ESP_MQTT_Setup(void) {
    PRINT("=== ESP8266 MQTT Setup ===");
    
    // 1. Initialize ESP8266
    ESP8266_Config_t cfg = {
        .cmd_uart = UART_CHANNEL_2,      // USART2 for ESP8266
        .debug_uart = UART_DEBUG_CHANNEL,    // Debug output
        .echo_off = true,
        .timeout_ms = 5000
    };
    
    if (ESP8266_Init(&esp, &cfg) != ESP8266_OK) {
        PRINT("ERROR: ESP8266 Init Failed!");
        return false;
    }
    PRINT("✓ ESP8266 Initialized");
    
    // 2. Set Station Mode
    if (ESP8266_SetMode(&esp, 1) != ESP8266_OK) {
        PRINT("ERROR: Failed to set Station mode");
        return false;
    }
    PRINT("✓ Station Mode Set");
    
    // 3. Connect to WiFi
   PRINT("Connecting to WiFi: %s...", WIFI_SSID);
    if (ESP8266_JoinAP(&esp, WIFI_SSID, WIFI_PASSWORD) != ESP8266_OK) {
        PRINT("ERROR: WiFi Connection Failed!");
        PRINT("Check SSID/Password in code");
        return false;
    }
    PRINT("✓ WiFi Connected!");
    
    // 4. Configure MQTT User
    if (ESP8266_MQTT_UserConfig(&esp, MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD) != ESP8266_OK) {
        PRINT("ERROR: MQTT User Config Failed!");
        PRINT("Your ESP8266 firmware may not support MQTT");
        PRINT("Check firmware version with: AT+GMR");
        return false;
    }
    PRINT("✓ MQTT User Configured");
    
    // 5. Connect to MQTT Broker
    PRINT("Connecting to MQTT Broker: %s:%d...", MQTT_BROKER, MQTT_PORT);
    if (ESP8266_MQTT_Connect(&esp, MQTT_BROKER, MQTT_PORT) != ESP8266_OK) {
        PRINT("ERROR: MQTT Connection Failed!");
        return false;
    }
    PRINT("✓ MQTT Connected!");
    PRINT("");
    
    return true;
}

/* ========================================================================
 * Test Case 1: MQTT Publish (发布数据到云端)
 * ======================================================================== */
#if TEST_MQTT_PUBLISH

void app_main(void) {
    HAL_Delay(100);
    
    PRINT("\r\n\r\n");
    PRINT("╔════════════════════════════════════════╗");
    PRINT("║   ESP8266 MQTT Publish Test           ║");
    PRINT("╚════════════════════════════════════════╝");
    PRINT("");
    
    // Setup WiFi and MQTT
    if (!ESP_MQTT_Setup()) {
        PRINT("\r\n❌ Setup Failed! Check your configuration.");
        while (1) HAL_Delay(1000);
    }
    
    PRINT("=== Starting Publish Loop ===");
    PRINT("Publishing to topics:");
    PRINT("  - stm32/test/counter");
    PRINT("  - stm32/test/temperature");
    PRINT("  - stm32/test/status");
    PRINT("");
    PRINT("Use MQTT client to monitor:");
    PRINT("  - MQTTX:   http://mqttx.app");
    PRINT("  - HiveMQ:  http://www.hivemq.com/demos/websocket-client/");
    PRINT("");
    
    uint32_t counter = 0;
    float temperature = 25.0f;
    
    while (1) {
        counter++;
        temperature += 0.1f;
        if (temperature > 30.0f) temperature = 25.0f;
        
        char payload[64];
        
        // Publish counter
        snprintf(payload, sizeof(payload), "%lu", counter);
        PRINT("[%lu] Publishing counter: %s", counter, payload);
        if (ESP8266_MQTT_Publish(&esp, "stm32/test/counter", payload, 0) != ESP8266_OK) {
            PRINT("  ⚠️ Publish failed!");
        }
        
        HAL_Delay(1000);
        
        // Publish temperature  
        snprintf(payload, sizeof(payload), "%.1f°C", temperature);
        PRINT("[%lu] Publishing temperature: %s", counter, payload);
        if (ESP8266_MQTT_Publish(&esp, "stm32/test/temperature", payload, 0) != ESP8266_OK) {
            PRINT("  ⚠️ Publish failed!");
        }
        
        HAL_Delay(1000);
        
        // Publish status
        PRINT("[%lu] Publishing status: online", counter);
        if (ESP8266_MQTT_Publish(&esp, "stm32/test/status", "online", 0) != ESP8266_OK) {
            PRINT("  ⚠️ Publish failed!");
        }
        
        PRINT("");
        HAL_Delay(3000);  // Wait 3s before next cycle
    }
}

#endif /* TEST_MQTT_PUBLISH */

/* ========================================================================
 * Test Case 2: MQTT Subscribe (订阅云端命令)
 * ======================================================================== */
#if TEST_MQTT_SUBSCRIBE

void User_Entry(void) {
    HAL_Delay(100);
    
    PRINT("\r\n\r\n");
    PRINT("╔════════════════════════════════════════╗");
    PRINT("║   ESP8266 MQTT Subscribe Test          ║");
    PRINT("╚════════════════════════════════════════╝");
    PRINT("");
    
    // Setup WiFi and MQTT
    if (!ESP_MQTT_Setup()) {
        PRINT("\r\n❌ Setup Failed! Check your configuration.");
        while (1) HAL_Delay(1000);
    }
    
    // Subscribe to command topic
    PRINT("=== Subscribing to Topics ===");
    if (ESP8266_MQTT_Subscribe(&esp, "stm32/test/command", 0) != ESP8266_OK) {
        PRINT("ERROR: Subscribe failed!");
        while (1) HAL_Delay(1000);
    }
    PRINT("✓ Subscribed to: stm32/test/command");
    PRINT("");
    PRINT("Waiting for messages...");
    PRINT("Send messages using MQTT client:");
    PRINT("  Topic: stm32/test/command");
    PRINT("  Payload: LED_ON, LED_OFF, RESET, etc.");
    PRINT("");
    
    // Note: Receiving messages requires parsing +MQTTSUBRECV
    // This is an async event from ESP8266
    // For now, just loop (full implementation needs async handling)
    
    while (1) {
        // TODO: Poll UART for +MQTTSUBRECV messages
        // Format: +MQTTSUBRECV:0,"topic",len,"data"
        HAL_Delay(100);
    }
}

#endif /* TEST_MQTT_SUBSCRIBE */
