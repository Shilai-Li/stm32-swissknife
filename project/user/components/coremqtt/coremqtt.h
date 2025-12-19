/**
 * @file coremqtt.h
 * @brief Wrapper header for the coreMQTT library integration
 * @version v2.3.1
 * @date 2025-12-18
 * 
 * This wrapper provides the coreMQTT client library interface for STM32 applications.
 * coreMQTT is a lightweight MQTT 3.1.1 client library optimized for embedded systems.
 * 
 * **Features:**
 * - MQTT 3.1.1 compliant
 * - Low memory footprint
 * - No external dependencies (except transport layer)
 * - Support for QoS 0, 1, and 2
 * - Clean and persistent sessions
 * - Last Will and Testament (LWT)
 * 
 * **Basic Usage:**
 * 1. Initialize MQTT context with MQTT_Init()
 * 2. Implement transport interface (send/receive functions)
 * 3. Connect to broker with MQTT_Connect()
 * 4. Subscribe/Publish with MQTT_Subscribe()/MQTT_Publish()
 * 5. Call MQTT_ProcessLoop() or MQTT_ReceiveLoop() periodically
 * 6. Disconnect with MQTT_Disconnect()
 * 
 * **Example:**
 * @code
 * #include "coreMQTT.h"
 * 
 * // Define network context and transport functions
 * NetworkContext_t networkContext;
 * TransportInterface_t transport = {
 *     .send = myNetworkSend,
 *     .recv = myNetworkRecv,
 *     .pNetworkContext = &networkContext
 * };
 * 
 * // Initialize MQTT
 * MQTTContext_t mqttContext;
 * MQTTFixedBuffer_t buffer;
 * uint8_t mqttBuffer[2048];
 * buffer.pBuffer = mqttBuffer;
 * buffer.size = sizeof(mqttBuffer);
 * 
 * MQTT_Init(&mqttContext, &transport, getTimestampMs, eventCallback, &buffer);
 * 
 * // Connect
 * MQTTConnectInfo_t connectInfo = {0};
 * bool sessionPresent;
 * connectInfo.cleanSession = true;
 * connectInfo.pClientIdentifier = "myClientID";
 * connectInfo.clientIdentifierLength = strlen(connectInfo.pClientIdentifier);
 * 
 * MQTT_Connect(&mqttContext, &connectInfo, NULL, 5000, &sessionPresent);
 * @endcode
 * 
 * @note Requires implementation of transport layer (TCP/IP + optional TLS)
 * @note See core_mqtt_config.h for configuration options
 */

#ifndef COREMQTT_H
#define COREMQTT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Include the main coreMQTT headers */
#include "core_mqtt.h"
#include "core_mqtt_serializer.h"
#include "core_mqtt_state.h"
#include "transport_interface.h"

/* Include port-specific implementations */
#include "coremqtt_port.h"

/**
 * @brief Get coreMQTT library version string
 * @return Version string (e.g., "v2.3.1")
 */
static inline const char* coremqtt_get_version(void)
{
    return MQTT_LIBRARY_VERSION;
}

#ifdef __cplusplus
}
#endif

#endif /* COREMQTT_H */
