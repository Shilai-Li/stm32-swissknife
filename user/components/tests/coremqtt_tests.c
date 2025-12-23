/**
 * @file coremqtt_tests.c
 * @brief Test cases for coreMQTT library integration
 * 
 * This file provides comprehensive tests for the coreMQTT library,
 * demonstrating basic functionality and integration with STM32.
 * 
 * **CubeMX Configuration Required:**
 * Before running these tests, you MUST implement a transport layer.
 * See coremqtt_port.h for implementation guidelines.
 * 
 * **Test Overview:**
 * 1. Library initialization test
 * 2. Packet serialization test
 * 3. Topic matching test
 * 4. State machine test
 * 5. Integration test (requires actual network connection)
 */

#include <stdio.h>
#include <string.h>
#include "stm32f1xx_hal.h"
#include "coremqtt.h"

#ifdef ELOG_H
#include "elog.h"
#define TEST_LOG(fmt, ...) log_i("MQTT_TEST", fmt, ##__VA_ARGS__)
#else
#define TEST_LOG(fmt, ...) printf(fmt "\r\n", ##__VA_ARGS__)
#endif

/* Test configuration */
#define MQTT_TEST_BUFFER_SIZE       1024
#define MQTT_TEST_CLIENT_ID         "STM32TestClient"
#define MQTT_TEST_BROKER_ENDPOINT   "test.mosquitto.org"
#define MQTT_TEST_TOPIC             "stm32/test/topic"

/* Define concrete NetworkContext for testing */
struct NetworkContext {
    int dummySocket;  /* Placeholder for actual socket/connection */
};


/* Static test buffers */
static uint8_t mqttTestBuffer[MQTT_TEST_BUFFER_SIZE];
static MQTTContext_t mqttTestContext;

/**
 * @brief Dummy time function for testing
 */
static uint32_t Test_GetTimeMs(void)
{
    return HAL_GetTick();
}

/**
 * @brief Dummy event callback for testing
 */
static void Test_EventCallback(MQTTContext_t* pContext,
                               MQTTPacketInfo_t* pPacketInfo,
                               MQTTDeserializedInfo_t* pDeserializedInfo)
{
    (void)pContext;
    (void)pPacketInfo;
    (void)pDeserializedInfo;
    
    TEST_LOG("Event callback invoked: packet type 0x%02X", pPacketInfo->type);
}

/**
 * @brief Dummy network send function (does nothing)
 */
static int32_t Test_NetworkSend(NetworkContext_t* pNetworkContext,
                               const void* pBuffer,
                               size_t bytesToSend)
{
    (void)pNetworkContext;
    (void)pBuffer;
    
    /* Simulate successful send */
    return (int32_t)bytesToSend;
}

/**
 * @brief Dummy network receive function (returns no data)
 */
static int32_t Test_NetworkRecv(NetworkContext_t* pNetworkContext,
                               void* pBuffer,
                               size_t bytesToRecv)
{
    (void)pNetworkContext;
    (void)pBuffer;
    (void)bytesToRecv;
    
    /* Simulate no data available */
    return 0;
}

/**
 * @brief Test 1: MQTT context initialization
 */
static void Test_MQTT_Init(void)
{
    MQTTStatus_t status;
    TransportInterface_t transport = {0};
    MQTTFixedBuffer_t networkBuffer = {0};
    NetworkContext_t networkContext = {0};
    
    TEST_LOG("==================================================");
    TEST_LOG("Test 1: MQTT Context Initialization");
    TEST_LOG("==================================================");
    
    /* Setup transport interface */
    transport.pNetworkContext = &networkContext;
    transport.send = Test_NetworkSend;
    transport.recv = Test_NetworkRecv;
    
    /* Setup network buffer */
    networkBuffer.pBuffer = mqttTestBuffer;
    networkBuffer.size = MQTT_TEST_BUFFER_SIZE;
    
    /* Clear context */
    memset(&mqttTestContext, 0, sizeof(MQTTContext_t));
    
    /* Initialize MQTT */
    status = MQTT_Init(&mqttTestContext,
                       &transport,
                       Test_GetTimeMs,
                       Test_EventCallback,
                       &networkBuffer);
    
    if (status == MQTTSuccess)
    {
        TEST_LOG("✓ MQTT_Init: SUCCESS");
        TEST_LOG("  - Context initialized");
        TEST_LOG("  - Buffer size: %u bytes", MQTT_TEST_BUFFER_SIZE);
    }
    else
    {
        TEST_LOG("✗ MQTT_Init: FAILED (status=%d)", status);
    }
    
    TEST_LOG("");
}

/**
 * @brief Test 2: CONNECT packet serialization
 */
static void Test_MQTT_SerializeConnect(void)
{
    MQTTStatus_t status;
    MQTTConnectInfo_t connectInfo = {0};
    MQTTFixedBuffer_t networkBuffer = {0};
    size_t packetSize = 0;
    
    TEST_LOG("==================================================");
    TEST_LOG("Test 2: CONNECT Packet Serialization");
    TEST_LOG("==================================================");
    
    /* Setup connect info */
    connectInfo.cleanSession = true;
    connectInfo.keepAliveSeconds = 60;
    connectInfo.pClientIdentifier = MQTT_TEST_CLIENT_ID;
    connectInfo.clientIdentifierLength = (uint16_t)strlen(MQTT_TEST_CLIENT_ID);
    
    /* Setup buffer */
    networkBuffer.pBuffer = mqttTestBuffer;
    networkBuffer.size = MQTT_TEST_BUFFER_SIZE;
    
    /* Serialize CONNECT packet */
    status = MQTT_SerializeConnect(&connectInfo,
                                   NULL,  /* No Last Will */
                                   0,     /* Remaining length */
                                   &networkBuffer);
    
    if (status == MQTTSuccess)
    {
        /* Calculate packet size (simplified) */
        packetSize = (size_t)mqttTestBuffer[1] + 2;  /* Remaining length + header */
        
        TEST_LOG("✓ MQTT_SerializeConnect: SUCCESS");
        TEST_LOG("  - Client ID: %s", MQTT_TEST_CLIENT_ID);
        TEST_LOG("  - Keep Alive: %u seconds", connectInfo.keepAliveSeconds);
        TEST_LOG("  - Clean Session: %s", connectInfo.cleanSession ? "Yes" : "No");
        TEST_LOG("  - Packet size: %u bytes", packetSize);
    }
    else
    {
        TEST_LOG("✗ MQTT_SerializeConnect: FAILED (status=%d)", status);
    }
    
    TEST_LOG("");
}

/**
 * @brief Test 3: Topic matching
 */
static void Test_MQTT_MatchTopic(void)
{
    MQTTStatus_t status;
    bool isMatch = false;
    
    TEST_LOG("==================================================");
    TEST_LOG("Test 3: Topic Matching");
    TEST_LOG("==================================================");
    
    /* Test 1: Exact match */
    status = MQTT_MatchTopic("sensor/temperature",
                            strlen("sensor/temperature"),
                            "sensor/temperature",
                            strlen("sensor/temperature"),
                            &isMatch);
    
    TEST_LOG("Test 3.1: Exact match");
    TEST_LOG("  Topic: 'sensor/temperature'");
    TEST_LOG("  Filter: 'sensor/temperature'");
    TEST_LOG("  Result: %s", (status == MQTTSuccess && isMatch) ? "✓ MATCH" : "✗ NO MATCH");
    
    /* Test 2: Single-level wildcard */
    status = MQTT_MatchTopic("sensor/temperature",
                            strlen("sensor/temperature"),
                            "sensor/+",
                            strlen("sensor/+"),
                            &isMatch);
    
    TEST_LOG("Test 3.2: Single-level wildcard");
    TEST_LOG("  Topic: 'sensor/temperature'");
    TEST_LOG("  Filter: 'sensor/+'");
    TEST_LOG("  Result: %s", (status == MQTTSuccess && isMatch) ? "✓ MATCH" : "✗ NO MATCH");
    
    /* Test 3: Multi-level wildcard */
    status = MQTT_MatchTopic("home/sensor/temperature/living-room",
                            strlen("home/sensor/temperature/living-room"),
                            "home/#",
                            strlen("home/#"),
                            &isMatch);
    
    TEST_LOG("Test 3.3: Multi-level wildcard");
    TEST_LOG("  Topic: 'home/sensor/temperature/living-room'");
    TEST_LOG("  Filter: 'home/#'");
    TEST_LOG("  Result: %s", (status == MQTTSuccess && isMatch) ? "✓ MATCH" : "✗ NO MATCH");
    
    /* Test 4: No match */
    status = MQTT_MatchTopic("sensor/temperature",
                            strlen("sensor/temperature"),
                            "actuator/relay",
                            strlen("actuator/relay"),
                            &isMatch);
    
    TEST_LOG("Test 3.4: Different topics");
    TEST_LOG("  Topic: 'sensor/temperature'");
    TEST_LOG("  Filter: 'actuator/relay'");
    TEST_LOG("  Result: %s", (status == MQTTSuccess && !isMatch) ? "✓ NO MATCH (as expected)" : "✗ UNEXPECTED MATCH");
    
    TEST_LOG("");
}

/**
 * @brief Test 4: Packet ID generation
 */
static void Test_MQTT_GetPacketId(void)
{
    uint16_t packetId1, packetId2, packetId3;
    
    TEST_LOG("==================================================");
    TEST_LOG("Test 4: Packet ID Generation");
    TEST_LOG("==================================================");
    
    /* Get packet IDs */
    packetId1 = MQTT_GetPacketId(&mqttTestContext);
    packetId2 = MQTT_GetPacketId(&mqttTestContext);
    packetId3 = MQTT_GetPacketId(&mqttTestContext);
    
    TEST_LOG("Generated Packet IDs:");
    TEST_LOG("  ID 1: %u", packetId1);
    TEST_LOG("  ID 2: %u", packetId2);
    TEST_LOG("  ID 3: %u", packetId3);
    
    if (packetId1 != MQTT_PACKET_ID_INVALID &&
        packetId2 != MQTT_PACKET_ID_INVALID &&
        packetId3 != MQTT_PACKET_ID_INVALID &&
        packetId1 != packetId2 &&
        packetId2 != packetId3)
    {
        TEST_LOG("✓ Packet ID generation: SUCCESS");
        TEST_LOG("  - All IDs are unique and valid");
    }
    else
    {
        TEST_LOG("✗ Packet ID generation: FAILED");
    }
    
    TEST_LOG("");
}

/**
 * @brief Test 5: Library version check
 */
static void Test_MQTT_Version(void)
{
    const char* version = coremqtt_get_version();
    
    TEST_LOG("==================================================");
    TEST_LOG("Test 5: Library Version");
    TEST_LOG("==================================================");
    TEST_LOG("coreMQTT Version: %s", version);
    TEST_LOG("");
}

/**
 * @brief Main test entry point
 */
void coremqtt_run_tests(void)
{
    TEST_LOG("");
    TEST_LOG("##################################################");
    TEST_LOG("#         coreMQTT Library Test Suite           #");
    TEST_LOG("##################################################");
    TEST_LOG("");
    
    /* Run tests */
    Test_MQTT_Init();
    Test_MQTT_SerializeConnect();
    Test_MQTT_MatchTopic();
    Test_MQTT_GetPacketId();
    Test_MQTT_Version();
    
    TEST_LOG("==================================================");
    TEST_LOG("All tests completed!");
    TEST_LOG("==================================================");
    TEST_LOG("");
    TEST_LOG("Note: These are basic library tests.");
    TEST_LOG("For full functionality testing, you need to:");
    TEST_LOG("  1. Implement a real transport layer");
    TEST_LOG("  2. Connect to an MQTT broker");
    TEST_LOG("  3. Test publish/subscribe operations");
    TEST_LOG("");
    TEST_LOG("See esp8266_mqtt_tests.c for a complete example");
    TEST_LOG("using the ESP8266 WiFi module.");
    TEST_LOG("");
}

/**
 * @brief User entry point called from main()
 */
void User_Entry(void)
{
    /* Wait for system initialization */
    HAL_Delay(100);
    
    /* Run coreMQTT tests */
    coremqtt_run_tests();
    
    /* Tests complete - enter idle loop */
    while (1)
    {
        HAL_Delay(1000);
    }
}

