/**
 * @file coremqtt_port.h
 * @brief Hardware abstraction layer for coreMQTT on STM32
 * 
 * This file provides platform-specific implementations and utilities for
 * integrating coreMQTT with STM32 microcontrollers.
 * 
 * **CubeMX Configuration:**
 * 
 * coreMQTT requires a network transport layer (TCP/IP) which is NOT provided.
 * You must implement the transport layer using one of the following options:
 * 
 * 1. **ESP8266 WiFi Module (AT Commands)**:
 *    - Use the existing esp8266 driver
 *    - See esp8266_mqtt_tests.c for examples
 * 
 * 2. **W5500 Ethernet Module**:
 *    - Use the existing w5500 driver
 *    - Implement TCP socket functions
 * 
 * 3. **LwIP TCP/IP Stack**:
 *    - Enable LwIP in CubeMX
 *    - Implement TransportRecv_t and TransportSend_t using LwIP APIs
 * 
 * 4. **FreeRTOS+TCP**:
 *    - If using FreeRTOS
 *    - Implement transport interface using FreeRTOS+TCP sockets
 * 
 * **Minimal Transport Implementation Example:**
 * @code
 * // Define your network context
 * struct NetworkContext {
 *     int socketDescriptor;
 *     // Add TLS context here if using TLS
 * };
 * 
 * // Implement receive function
 * int32_t MyTransport_Recv(NetworkContext_t* pNetworkContext, 
 *                          void* pBuffer, size_t bytesToRecv) {
 *     // Return number of bytes received, 0 if no data, negative on error
 *     return recv(pNetworkContext->socketDescriptor, pBuffer, bytesToRecv, 0);
 * }
 * 
 * // Implement send function
 * int32_t MyTransport_Send(NetworkContext_t* pNetworkContext, 
 *                          const void* pBuffer, size_t bytesToSend) {
 *     // Return number of bytes sent, 0 if buffer full, negative on error
 *     return send(pNetworkContext->socketDescriptor, pBuffer, bytesToSend, 0);
 * }
 * @endcode
 */

#ifndef COREMQTT_PORT_H
#define COREMQTT_PORT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "transport_interface.h"

/**
 * @brief Get current time in milliseconds since boot
 * 
 * This function provides a monotonic millisecond counter for coreMQTT
 * to handle timeouts and keep-alive mechanisms.
 * 
 * @return Current time in milliseconds
 * 
 * @note This uses HAL_GetTick() which wraps around every ~49 days.
 *       coreMQTT handles the wrap-around correctly for durations < 50 days.
 */
uint32_t coremqtt_get_time_ms(void);

/**
 * @brief Example event callback for MQTT events
 * 
 * This callback is invoked by coreMQTT when packets are received.
 * Implement your application logic here to handle incoming publishes
 * and acknowledgements.
 * 
 * @param[in] pContext MQTT context
 * @param[in] pPacketInfo Packet information
 * @param[in] pDeserializedInfo Deserialized packet data
 */
void coremqtt_event_callback(MQTTContext_t* pContext,
                            MQTTPacketInfo_t* pPacketInfo,
                            MQTTDeserializedInfo_t* pDeserializedInfo);

/**
 * @brief Helper function to create a default transport interface
 * 
 * @param[out] pTransport Pointer to transport interface to initialize
 * @param[in] pNetworkContext Pointer to your network context
 * @param[in] sendFunc Your transport send function
 * @param[in] recvFunc Your transport receive function
 */
static inline void coremqtt_transport_interface_init(
    TransportInterface_t* pTransport,
    NetworkContext_t* pNetworkContext,
    TransportSend_t sendFunc,
    TransportRecv_t recvFunc)
{
    if (pTransport != NULL) {
        pTransport->pNetworkContext = pNetworkContext;
        pTransport->send = sendFunc;
        pTransport->recv = recvFunc;
        pTransport->writev = NULL;  /* Optional, can be NULL */
    }
}

#ifdef __cplusplus
}
#endif

#endif /* COREMQTT_PORT_H */
