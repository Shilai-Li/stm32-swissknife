/**
 * @file w5500.h
 * @brief W5500 Ethernet Controller Driver
 * 
 * =================================================================================
 *                       >>> INTEGRATION GUIDE <<<
 * =================================================================================
 * 1. CubeMX Config (SPI):
 *    - Enable SPI1 (or SPI2)
 *    - Mode: Full-Duplex Master
 *    - Hardware NSS Signal: Disable (use software GPIO)
 *    - Clock: Up to 42MHz (STM32F1: 36MHz max)
 *    - Data Size: 8 Bits
 *    - First Bit: MSB First
 *    - Prescaler: 2-4 (for high speed)
 *    - CPOL: Low, CPHA: 1 Edge
 * 
 * 2. GPIO Config:
 *    - W5500_CS (NSS): GPIO Output, High by default
 *    - W5500_RST (Optional): GPIO Output, High by default
 * 
 * 3. Wiring:
 *    STM32 SPI1      W5500 Module
 *    ─────────────────────────────
 *    SCK (PA5)   →   SCK
 *    MISO (PA6)  ←   MISO
 *    MOSI (PA7)  →   MOSI
 *    CS (PA4)    →   SCSn (Chip Select)
 *    RST (PA3)   →   RSTn (Reset, optional)
 *    3.3V        →   VCC
 *    GND         →   GND
 * 
 * 4. Ethernet Cable:
 *    - Connect RJ45 port to router/switch
 *    - Link LED should light up when connected
 * 
 * 5. Network Configuration Example:
 *    - IP: 192.168.1.100
 *    - Gateway: 192.168.1.1
 *    - Subnet: 255.255.255.0
 *    - MAC: 0x00, 0x08, 0xDC, 0xAB, 0xCD, 0xEF
 * =================================================================================
 */

#ifndef W5500_H
#define W5500_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "main.h"

/* ============================================================================
 * Configuration
 * ========================================================================= */

#define W5500_MAX_SOCKETS       8      /* W5500 has 8 hardware sockets */
#define W5500_TX_BUFFER_SIZE    2048   /* Per socket TX buffer */
#define W5500_RX_BUFFER_SIZE    2048   /* Per socket RX buffer */

/* ============================================================================
 * Socket Protocol Types
 * ========================================================================= */

typedef enum {
    W5500_PROTO_CLOSED = 0x00,
    W5500_PROTO_TCP    = 0x01,
    W5500_PROTO_UDP    = 0x02,
    W5500_PROTO_MACRAW = 0x04
} W5500_Protocol_t;

/* ============================================================================
 * Socket Status
 * ========================================================================= */

typedef enum {
    W5500_SOCK_CLOSED      = 0x00,
    W5500_SOCK_INIT        = 0x13,
    W5500_SOCK_LISTEN      = 0x14,
    W5500_SOCK_ESTABLISHED = 0x17,
    W5500_SOCK_CLOSE_WAIT  = 0x1C,
    W5500_SOCK_UDP         = 0x22,
    W5500_SOCK_MACRAW      = 0x42
} W5500_SockStatus_t;

/* ============================================================================
 * Return Status
 * ========================================================================= */

typedef enum {
    W5500_OK = 0,
    W5500_ERROR,
    W5500_TIMEOUT,
    W5500_BUSY,
    W5500_INVALID_SOCKET,
    W5500_NOT_CONNECTED
} W5500_Status_t;

/* ============================================================================
 * Network Configuration Structure
 * ========================================================================= */

typedef struct {
    uint8_t mac[6];         /* MAC Address */
    uint8_t ip[4];          /* IP Address */
    uint8_t gateway[4];     /* Gateway Address */
    uint8_t subnet[4];      /* Subnet Mask */
    uint8_t dns[4];         /* DNS Server (optional) */
} W5500_NetConfig_t;

/* ============================================================================
 * Hardware Configuration Structure
 * ========================================================================= */

typedef struct {
    SPI_HandleTypeDef *hspi;    /* SPI Handle */
    GPIO_TypeDef *cs_port;      /* Chip Select GPIO Port */
    uint16_t cs_pin;            /* Chip Select GPIO Pin */
    GPIO_TypeDef *rst_port;     /* Reset GPIO Port (optional, can be NULL) */
    uint16_t rst_pin;           /* Reset GPIO Pin */
} W5500_Config_t;

/* ============================================================================
 * Public API - Initialization
 * ========================================================================= */

/**
 * @brief Initialize W5500 with network configuration
 * @param config Hardware configuration (SPI, GPIO)
 * @param net_config Network configuration (IP, MAC, etc.)
 * @return W5500_OK on success
 */
W5500_Status_t W5500_Init(const W5500_Config_t *config, const W5500_NetConfig_t *net_config);

/**
 * @brief Hardware Reset W5500 chip
 * @return W5500_OK on success
 */
W5500_Status_t W5500_Reset(void);

/**
 * @brief Check if W5500 is responding
 * @return W5500_OK if chip detected
 */
W5500_Status_t W5500_Check(void);

/* ============================================================================
 * Public API - Socket Management
 * ========================================================================= */

/**
 * @brief Open a socket with specified protocol and port
 * @param socket Socket number (0-7)
 * @param protocol TCP, UDP, or MACRAW
 * @param port Local port number (0 for auto-assign)
 * @return W5500_OK on success
 */
W5500_Status_t W5500_Socket_Open(uint8_t socket, W5500_Protocol_t protocol, uint16_t port);

/**
 * @brief Close a socket
 * @param socket Socket number (0-7)
 * @return W5500_OK on success
 */
W5500_Status_t W5500_Socket_Close(uint8_t socket);

/**
 * @brief Get current socket status
 * @param socket Socket number (0-7)
 * @return Socket status
 */
W5500_SockStatus_t W5500_Socket_GetStatus(uint8_t socket);

/* ============================================================================
 * Public API - TCP Client
 * ========================================================================= */

/**
 * @brief Connect to TCP server
 * @param socket Socket number (0-7)
 * @param dest_ip Destination IP address (4 bytes)
 * @param dest_port Destination port
 * @param timeout_ms Connection timeout in milliseconds
 * @return W5500_OK on success
 */
W5500_Status_t W5500_TCP_Connect(uint8_t socket, const uint8_t *dest_ip, uint16_t dest_port, uint32_t timeout_ms);

/**
 * @brief Send data over TCP connection
 * @param socket Socket number (0-7)
 * @param data Data buffer to send
 * @param len Data length
 * @return Number of bytes sent, or <0 on error
 */
int32_t W5500_TCP_Send(uint8_t socket, const uint8_t *data, uint16_t len);

/**
 * @brief Receive data from TCP connection
 * @param socket Socket number (0-7)
 * @param buffer Buffer to store received data
 * @param max_len Maximum bytes to receive
 * @return Number of bytes received, 0 if no data, <0 on error
 */
int32_t W5500_TCP_Recv(uint8_t socket, uint8_t *buffer, uint16_t max_len);

/**
 * @brief Check how many bytes are available to read
 * @param socket Socket number (0-7)
 * @return Number of bytes available
 */
uint16_t W5500_TCP_Available(uint8_t socket);

/**
 * @brief Disconnect TCP connection
 * @param socket Socket number (0-7)
 * @return W5500_OK on success
 */
W5500_Status_t W5500_TCP_Disconnect(uint8_t socket);

/* ============================================================================
 * Public API - TCP Server
 * ========================================================================= */

/**
 * @brief Start TCP server listening on port
 * @param socket Socket number (0-7)
 * @return W5500_OK on success
 */
W5500_Status_t W5500_TCP_Listen(uint8_t socket);

/* ============================================================================
 * Public API - UDP
 * ========================================================================= */

/**
 * @brief Send UDP packet
 * @param socket Socket number (0-7)
 * @param dest_ip Destination IP address (4 bytes)
 * @param dest_port Destination port
 * @param data Data buffer to send
 * @param len Data length
 * @return Number of bytes sent, or <0 on error
 */
int32_t W5500_UDP_Send(uint8_t socket, const uint8_t *dest_ip, uint16_t dest_port, const uint8_t *data, uint16_t len);

/**
 * @brief Receive UDP packet
 * @param socket Socket number (0-7)
 * @param buffer Buffer to store received data
 * @param max_len Maximum bytes to receive
 * @param src_ip Buffer to store source IP (4 bytes, can be NULL)
 * @param src_port Pointer to store source port (can be NULL)
 * @return Number of bytes received, 0 if no data, <0 on error
 */
int32_t W5500_UDP_Recv(uint8_t socket, uint8_t *buffer, uint16_t max_len, uint8_t *src_ip, uint16_t *src_port);

/* ============================================================================
 * Public API - Utilities
 * ========================================================================= */

/**
 * @brief Get current link status
 * @return true if Ethernet cable is connected
 */
bool W5500_IsLinkUp(void);

/**
 * @brief Get W5500 chip version
 * @return Version number (should be 0x04 for W5500)
 */
uint8_t W5500_GetVersion(void);

#ifdef __cplusplus
}
#endif

#endif /* W5500_H */
