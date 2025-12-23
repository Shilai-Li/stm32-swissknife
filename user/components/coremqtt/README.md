# coreMQTT Component

AWS IoT coreMQTT library port for STM32 microcontrollers.

## Overview

coreMQTT is a lightweight, MQTT 3.1.1 compliant client library optimized for embedded systems. This port integrates coreMQTT into the STM32 Swiss Knife framework.

**Version:** v2.3.1

**Features:**
- ✅ MQTT 3.1.1 compliant
- ✅ Low memory footprint
- ✅ QoS 0, 1, and 2 support
- ✅ Clean and persistent sessions
- ✅ Last Will and Testament (LWT)
- ✅ No external dependencies (except transport layer)
- ✅ EasyLogger integration for debugging

## Directory Structure

```
coremqtt/
├── coremqtt.h              # Main wrapper header
├── coremqtt_port.h         # Platform abstraction layer header
├── coremqtt_port.c         # Platform implementation
├── update_coremqtt.ps1     # Update script from official repo
└── csrc/                   # Core library source files (官方库，保持原名)
    ├── core_mqtt.c
    ├── core_mqtt.h
    ├── core_mqtt_config.h  # Configuration file
    ├── core_mqtt_serializer.c
    ├── core_mqtt_state.c
    └── transport_interface.h
```

## Prerequisites

⚠️ **IMPORTANT:** coreMQTT requires a **transport layer** implementation. You must provide:

1. **TCP/IP Stack** (one of):
   - ESP8266 AT commands (already available in this project)
   - W5500 Ethernet module (driver available)
   - LwIP TCP/IP stack
   - FreeRTOS+TCP
   - Custom socket implementation

2. **Transport Interface Functions:**
   - `TransportSend_t` - Send data over network
   - `TransportRecv_t` - Receive data from network

## Quick Start

### 1. Build Configuration

In `CMakeLists.txt`, set:

```cmake
set(TEST_CASE "coremqtt" CACHE STRING "Select which test to run" FORCE)
```

### 2. Basic Example

```c
#include "coremqtt.h"

// 1. Define network context (example for custom TCP/IP)
struct NetworkContext {
    int socketDescriptor;
    // Add TLS context if needed
};

// 2. Implement transport functions
int32_t MyTransport_Send(NetworkContext_t* ctx, const void* buf, size_t len) {
    return send(ctx->socketDescriptor, buf, len, 0);
}

int32_t MyTransport_Recv(NetworkContext_t* ctx, void* buf, size_t len) {
    return recv(ctx->socketDescriptor, buf, len, 0);
}

// 3. Setup MQTT
void MQTT_Setup(void) {
    MQTTContext_t mqttContext;
    NetworkContext_t networkContext;
    TransportInterface_t transport;
    MQTTFixedBuffer_t buffer;
    uint8_t mqttBuffer[2048];
    
    // Setup transport
    transport.pNetworkContext = &networkContext;
    transport.send = MyTransport_Send;
    transport.recv = MyTransport_Recv;
    transport.writev = NULL;  // Optional
    
    // Setup buffer
    buffer.pBuffer = mqttBuffer;
    buffer.size = sizeof(mqttBuffer);
    
    // Initialize MQTT
    MQTT_Init(&mqttContext, &transport, coremqtt_get_time_ms, 
              coremqtt_event_callback, &buffer);
    
    // Connect to broker
    MQTTConnectInfo_t connectInfo = {0};
    bool sessionPresent;
    
    connectInfo.cleanSession = true;
    connectInfo.pClientIdentifier = "MySTM32Client";
    connectInfo.clientIdentifierLength = strlen(connectInfo.pClientIdentifier);
    connectInfo.keepAliveSeconds = 60;
    
    MQTT_Connect(&mqttContext, &connectInfo, NULL, 5000, &sessionPresent);
    
    // Subscribe to topic
    MQTTSubscribeInfo_t subscription = {0};
    subscription.qos = MQTTQoS0;
    subscription.pTopicFilter = "stm32/sensors/#";
    subscription.topicFilterLength = strlen(subscription.pTopicFilter);
    
    uint16_t packetId = MQTT_GetPacketId(&mqttContext);
    MQTT_Subscribe(&mqttContext, &subscription, 1, packetId);
    
    // Publish message
    MQTTPublishInfo_t publishInfo = {0};
    publishInfo.qos = MQTTQoS0;
    publishInfo.retain = false;
    publishInfo.pTopicName = "stm32/status";
    publishInfo.topicNameLength = strlen(publishInfo.pTopicName);
    publishInfo.pPayload = "Hello from STM32!";
    publishInfo.payloadLength = strlen(publishInfo.pPayload);
    
    MQTT_Publish(&mqttContext, &publishInfo, 0);
    
    // Process loop (call periodically)
    MQTT_ProcessLoop(&mqttContext, 100);
}
```

## Using with ESP8266

If you're using the ESP8266 WiFi module (already integrated):

See `drivers/tests/esp8266_mqtt_tests.c` for a complete working example.

The ESP8266 driver provides MQTT client functions directly:
```c
#include "esp8266.h"

ESP8266_MQTT_SetUserConfig(0, 1, "MyClient", "user", "pass", 0, 0, "");
ESP8266_MQTT_Connect("broker.hivemq.com", 1883, 0);
ESP8266_MQTT_Subscribe("test/topic", 0);
ESP8266_MQTT_Publish("test/topic", "Hello!", 0, 0);
```

## Configuration

Edit `csrc/core_mqtt_config.h` to adjust:

- `MQTT_RECV_POLLING_TIMEOUT_MS` - Receive timeout
- `MQTT_STATE_ARRAY_MAX_COUNT` - Max pending QoS messages
- `MQTT_PINGRESP_TIMEOUT_MS` - Ping response timeout
- `MQTT_MAX_CLIENT_ID_LEN` - Client ID length limit
- `MQTT_MAX_TOPIC_LEN` - Topic name length limit
- `MQTT_MAX_PAYLOAD_LEN` - Payload size limit

## Testing

Run the test suite:

```bash
# Build with coreMQTT tests
cmake -DTEST_CASE=coremqtt ...
make
```

The tests demonstrate:
1. ✅ Library initialization
2. ✅ Packet serialization
3. ✅ Topic matching (wildcards)
4. ✅ Packet ID generation
5. ✅ Version information

## API Reference

### Core Functions

- `MQTT_Init()` - Initialize MQTT context
- `MQTT_Connect()` - Connect to MQTT broker
- `MQTT_Subscribe()` - Subscribe to topics
- `MQTT_Unsubscribe()` - Unsubscribe from topics
- `MQTT_Publish()` - Publish messages
- `MQTT_ProcessLoop()` - Process MQTT events
- `MQTT_Disconnect()` - Disconnect from broker

### Helper Functions

- `MQTT_GetPacketId()` - Generate unique packet ID
- `MQTT_MatchTopic()` - Match topic with filter (wildcards)
- `coremqtt_get_version()` - Get library version

## Updating

To update to the latest coreMQTT version:

```powershell
cd HAL/user/components/coremqtt
.\update_coremqtt.ps1
```

This will clone the latest version from the official AWS repository.

## Memory Usage

Typical memory footprint (STM32F103):
- **Code (Flash):** ~15-20 KB
- **RAM (per context):** ~2-4 KB (depends on buffer size)
- **Network Buffer:** User-defined (recommended: 2048 bytes)

## References

- [Official coreMQTT Repository](https://github.com/FreeRTOS/coreMQTT)
- [MQTT 3.1.1 Specification](https://docs.oasis-open.org/mqtt/mqtt/v3.1.1/mqtt-v3.1.1.html)
- [AWS IoT Core Documentation](https://docs.aws.amazon.com/iot/)
- [ESP8266 MQTT Example](../../drivers/tests/esp8266_mqtt_tests.c)

## License

MIT License (same as original coreMQTT library)

## Support

For issues specific to this STM32 port, consult:
- `coremqtt_port.h` - Platform implementation notes
- `components/tests/coremqtt_tests.c` - Test examples
- `drivers/tests/esp8266_mqtt_tests.c` - Real-world example

For coreMQTT library issues:
- [GitHub Issues](https://github.com/FreeRTOS/coreMQTT/issues)
- [AWS Forums](https://forums.aws.amazon.com/forum.jspa?forumID=210)
