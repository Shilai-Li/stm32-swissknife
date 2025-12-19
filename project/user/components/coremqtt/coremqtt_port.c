/**
 * @file coremqtt_port.c
 * @brief Platform-specific implementations for coreMQTT
 */

#include "coremqtt.h"
#include "stm32f1xx_hal.h"  /* Adjust to your STM32 family */

#ifdef ELOG_H
#include "elog.h"
#endif


/**
 * @brief Get current time in milliseconds
 */
uint32_t coremqtt_get_time_ms(void)
{
    return HAL_GetTick();
}

/**
 * @brief Default MQTT event callback implementation
 * 
 * This is a basic example. Replace with your application-specific logic.
 */
void coremqtt_event_callback(MQTTContext_t* pContext,
                            MQTTPacketInfo_t* pPacketInfo,
                            MQTTDeserializedInfo_t* pDeserializedInfo)
{
    uint16_t packetIdentifier;
    
    (void)pContext;
    
    /* Suppress unused variable warning if logging is disabled. */
    (void)packetIdentifier;

    /* Check if packet type is PUBLISH */
    if ((pPacketInfo->type & 0xF0U) == MQTT_PACKET_TYPE_PUBLISH)
    {
        packetIdentifier = pDeserializedInfo->packetIdentifier;
        
        /* Handle incoming PUBLISH */
        if (pDeserializedInfo->pPublishInfo != NULL)
        {
            MQTTPublishInfo_t* pPubInfo = pDeserializedInfo->pPublishInfo;
            
            #ifdef ELOG_H
            log_i("MQTT", "Incoming PUBLISH received:");
            log_i("MQTT", "  Topic: %.*s", 
                  pPubInfo->topicNameLength, 
                  pPubInfo->pTopicName);
            log_i("MQTT", "  Payload: %.*s", 
                  pPubInfo->payloadLength, 
                  (const char*)pPubInfo->pPayload);
            log_i("MQTT", "  QoS: %d", pPubInfo->qos);
            log_i("MQTT", "  PacketID: %u", packetIdentifier);
            #endif
            
            /* TODO: Add your application-specific PUBLISH handling here */
        }
    }
    else
    {
        /* Handle other packet types (ACKs, etc.) */
        switch (pPacketInfo->type)
        {
            case MQTT_PACKET_TYPE_PUBACK:
                #ifdef ELOG_H
                log_d("MQTT", "PUBACK received for packet %u", 
                      pDeserializedInfo->packetIdentifier);
                #endif
                break;
                
            case MQTT_PACKET_TYPE_SUBACK:
                #ifdef ELOG_H
                log_i("MQTT", "SUBACK received for packet %u", 
                      pDeserializedInfo->packetIdentifier);
                #endif
                break;
                
            case MQTT_PACKET_TYPE_UNSUBACK:
                #ifdef ELOG_H
                log_i("MQTT", "UNSUBACK received for packet %u", 
                      pDeserializedInfo->packetIdentifier);
                #endif
                break;
                
            case MQTT_PACKET_TYPE_PINGRESP:
                #ifdef ELOG_H
                log_d("MQTT", "PINGRESP received");
                #endif
                break;
                
            default:
                #ifdef ELOG_H
                log_w("MQTT", "Unknown packet type: 0x%02X", pPacketInfo->type);
                #endif
                break;
        }
    }
}
