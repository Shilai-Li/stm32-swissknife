/**
 * @file core_mqtt_config.h
 * @brief coreMQTT library configuration for STM32 bare-metal environment
 * 
 * This configuration file defines settings for the coreMQTT library optimized
 * for resource-constrained STM32 microcontrollers.
 */

#ifndef CORE_MQTT_CONFIG_H
#define CORE_MQTT_CONFIG_H

/**************************************************/
/******* DO NOT CHANGE the following order ********/
/**************************************************/

/* Logging configuration for coreMQTT library. */
#ifndef LIBRARY_LOG_NAME
    #define LIBRARY_LOG_NAME     "MQTT"
#endif

#ifndef LIBRARY_LOG_LEVEL
    #define LIBRARY_LOG_LEVEL    LOG_INFO
#endif

/* Logging header include (uses EasyLogger if available) */
#if defined(ELOG_H)
    #define LogError(message)    log_e(LIBRARY_LOG_NAME, message)
    #define LogWarn(message)     log_w(LIBRARY_LOG_NAME, message)
    #define LogInfo(message)     log_i(LIBRARY_LOG_NAME, message)
    #define LogDebug(message)    log_d(LIBRARY_LOG_NAME, message)
#else
    /* Fallback to no logging if EasyLogger is not available */
    #define LogError(message)
    #define LogWarn(message)
    #define LogInfo(message)
    #define LogDebug(message)
#endif

/**
 * @brief Timeout for receiving CONNACK packet in milliseconds.
 *
 * When receiving the CONNACK packet, the RECV function may be called multiple
 * times until the full packet is received. The timeout will
 * apply to each call to the transport receive function.
 */
#define MQTT_RECV_POLLING_TIMEOUT_MS                 ( 100U )

/**
 * @brief Maximum number of MQTT PUBLISH messages that may be pending
 * acknowledgement at any time.
 *
 * QoS 1 and 2 MQTT PUBLISHes require acknowledgement from the server before
 * they can be completed. While they are awaiting the acknowledgement, the
 * client must maintain information about their state. The value of this
 * macro sets the limit on how many simultaneous PUBLISH states an MQTT
 * context maintains.
 */
#define MQTT_STATE_ARRAY_MAX_COUNT                   ( 10U )

/**
 * @brief The maximum number of retries for receiving CONNACK.
 *
 * The MQTT_Connect function attempts to receive the CONNACK packet from the
 * broker. This macro determines how many times the API will attempt to receive
 * the CONNACK when timeoutMs in the input is 0.
 *
 * @note The default value is 2 times. Setting it to 0 means there will be
 * no retries.
 */
#define MQTT_MAX_CONNACK_RECEIVE_RETRY_COUNT         ( 2U )

/**
 * @brief Number of milliseconds to wait for a ping response to a ping
 * request as part of the keep-alive mechanism.
 *
 * If a ping response is not received before this timeout, then
 * #MQTT_ProcessLoop will return #MQTTKeepAliveTimeout.
 */
#define MQTT_PINGRESP_TIMEOUT_MS                     ( 5000U )

/**
 * @brief The maximum time to wait for a PUBACK for a QoS 1 PUBLISH.
 *
 * If a PUBACK is not received before this timeout, then #MQTT_ProcessLoop
 * will return #MQTTPubAckTimeout. If 0, then the timeout will be disabled.
 */
#define MQTT_SEND_TIMEOUT_MS                         ( 100U )

/**
 * @brief Macro to enable/disable assertion in the MQTT library.
 *
 * Assertions are by default disabled to optimize memory usage in embedded
 * systems. Set this to 1 to enable assertions during development/debugging.
 */
#define MQTT_DO_NOT_USE_CUSTOM_ASSERT                ( 1 )

/**
 * @brief Macro to enable/disable metrics reporting to AWS IoT.
 *
 * When enabled, the library will include device information in the username
 * field of CONNECT packet. This is useful when connecting to AWS IoT.
 * Set to 0 for bare-metal STM32 applications.
 */
#define MQTT_REPORT_METRICS                          ( 0 )

/**
 * @brief Maximum length of client identifier for MQTT connection.
 *
 * The MQTT v3.1.1 specification states that client identifiers should not
 * exceed 23 characters. AWS IoT allows up to 128. Adjust as needed.
 */
#define MQTT_MAX_CLIENT_ID_LEN                       ( 64U )

/**
 * @brief Maximum length of topic name/filter.
 *
 * MQTT v3.1.1 allows topic names and filters up to 65535 bytes.
 * This value can be reduced to save memory in resource-constrained systems.
 */
#define MQTT_MAX_TOPIC_LEN                           ( 256U )

/**
 * @brief Maximum length of MQTT payload.
 *
 * Defines the maximum size for a single MQTT message payload.
 * Adjust based on your application requirements and available RAM.
 */
#define MQTT_MAX_PAYLOAD_LEN                         ( 2048U )

#endif /* CORE_MQTT_CONFIG_H */
