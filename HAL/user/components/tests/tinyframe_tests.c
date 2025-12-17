/**
 * @file tinyframe_tests.c
 * @brief TinyFrame component test suite
 * 
 * This test demonstrates TinyFrame usage for serial protocol parsing:
 * - Echo server: Receives frames and sends them back
 * - Command handler: Processes different frame types
 * - Periodic sender: Sends frames at regular intervals
 * 
 * CubeMX Configuration:
 * ====================
 * See uart.h for UART configuration requirements.
 * 
 * Ensure USE_UART2 is enabled (or change TINYFRAME_UART_CHANNEL).
 * 
 * How to Test:
 * ============
 * 1. Connect UART2 to PC via USB-Serial adapter
 * 2. Use a TinyFrame client (Python, C++, etc.) to send frames
 * 3. Or connect two boards and run this test on both
 * 4. Monitor the serial output to see frame exchanges
 * 
 * Example Python client:
 * ======================
 * pip install pyTinyFrame
 * 
 * from TinyFrame import TinyFrame
 * import serial
 * 
 * ser = serial.Serial('/dev/ttyUSB0', 115200)
 * tf = TinyFrame()
 * 
 * # Send a frame
 * msg = tf.build_frame(type=0x01, data=b'Hello STM32!')
 * ser.write(msg)
 * 
 * # Read response
 * while True:
 *     data = ser.read(1)
 *     if data:
 *         tf.accept(data)
 */

#include "tinyframe.h"
#include "tinyframe_port.h"
#include "uart.h"
#include "stm32f1xx_hal.h"
#include <stdio.h>
#include <string.h>

/* ============================================================================
 * Test Configuration
 * ========================================================================= */

#define ENABLE_ECHO_SERVER    1  // Echo back received frames
#define ENABLE_PERIODIC_SEND  1  // Send test frames periodically
#define PERIODIC_INTERVAL_MS  5000  // Send interval in milliseconds

/* ============================================================================
 * Frame Type Definitions
 * ========================================================================= */

typedef enum {
    FRAME_TYPE_ECHO       = 0x01,  // Echo request
    FRAME_TYPE_COMMAND    = 0x02,  // Command frame
    FRAME_TYPE_DATA       = 0x03,  // Data frame
    FRAME_TYPE_HEARTBEAT  = 0x10,  // Heartbeat/ping
} TF_FrameType;

/* ============================================================================
 * Global Variables
 * ========================================================================= */

static TinyFrame *tf = NULL;
static uint32_t last_send_time = 0;
static uint32_t rx_frame_count = 0;
static uint32_t tx_frame_count = 0;

/* ============================================================================
 * Frame Listeners (Callbacks)
 * ========================================================================= */

/**
 * @brief Echo listener - sends back what it receives
 */
TF_Result echo_listener(TinyFrame *tf, TF_Msg *msg)
{
    UART_Debug_Printf("[TF] Echo request received, len=%d\r\n", msg->len);
    rx_frame_count++;
    
    #if ENABLE_ECHO_SERVER
    // Echo back the frame
    TF_Msg response;
    TF_ClearMsg(&response);
    response.type = FRAME_TYPE_ECHO;
    response.data = msg->data;
    response.len = msg->len;
    response.is_response = true;
    response.frame_id = msg->frame_id;
    
    if (TF_Send(tf, &response)) {
        tx_frame_count++;
        UART_Debug_Printf("[TF] Echo response sent\r\n");
    } else {
        UART_Debug_Printf("[TinyFrame] Failed to send echo response\r\n");
    }
    #endif
    
    return TF_STAY;  // Keep this listener active
}

/**
 * @brief Command listener - processes commands
 */
TF_Result command_listener(TinyFrame *tf, TF_Msg *msg)
{
    UART_Debug_Printf("[TF] Command received, len=%d\r\n", msg->len);
    rx_frame_count++;
    
    // Parse command (example: first byte is command code)
    if (msg->len > 0) {
        uint8_t cmd = msg->data[0];
        UART_Debug_Printf("[TF] Command code: 0x%02X\r\n", cmd);
        
        switch (cmd) {
            case 0x01:  // Get status
                {
                    char status_msg[64];
                    int len = snprintf(status_msg, sizeof(status_msg),
                                     "RX:%lu TX:%lu", rx_frame_count, tx_frame_count);
                    
                    TF_Msg response;
                    TF_ClearMsg(&response);
                    response.type = FRAME_TYPE_COMMAND;
                    response.data = (uint8_t*)status_msg;
                    response.len = (TF_LEN)len;
                    response.is_response = true;
                    response.frame_id = msg->frame_id;
                    
                    TF_Send(tf, &response);
                    tx_frame_count++;
                }
                break;
                
            case 0x02:  // Reset counters
                rx_frame_count = 0;
                tx_frame_count = 0;
                UART_Debug_Printf("[TF] Counters reset\r\n");
                break;
                
            default:
                UART_Debug_Printf("[TF] Unknown command: 0x%02X\r\n", cmd);
                break;
        }
    }
    
    return TF_STAY;
}

/**
 * @brief Data listener - handles general data frames
 */
TF_Result data_listener(TinyFrame *tf, TF_Msg *msg)
{
    UART_Debug_Printf("[TF] Data frame received, len=%d\r\n", msg->len);
    rx_frame_count++;
    
    // Print data as hex
    UART_Debug_Printf("[TF] Data: ");
    for (TF_LEN i = 0; i < msg->len && i < 16; i++) {
        UART_Debug_Printf("%02X ", msg->data[i]);
    }
    if (msg->len > 16) {
        UART_Debug_Printf("... (%d bytes total)", msg->len);
    }
    UART_Debug_Printf("\r\n");
    
    return TF_STAY;
}

/**
 * @brief Generic fallback listener - catches unhandled frames
 */
TF_Result generic_listener(TinyFrame *tf, TF_Msg *msg)
{
    UART_Debug_Printf("[TF] Unhandled frame type 0x%02X, len=%d\r\n", msg->type, msg->len);
    rx_frame_count++;
    return TF_STAY;
}

/* ============================================================================
 * Helper Functions
 * ========================================================================= */

/**
 * @brief Send a periodic heartbeat frame
 */
void send_heartbeat(void)
{
    static uint32_t heartbeat_counter = 0;
    
    TF_Msg msg;
    TF_ClearMsg(&msg);
    msg.type = FRAME_TYPE_HEARTBEAT;
    msg.data = (uint8_t*)&heartbeat_counter;
    msg.len = sizeof(heartbeat_counter);
    
    if (TF_Send(tf, &msg)) {
        tx_frame_count++;
        UART_Debug_Printf("[TF] Heartbeat sent: %lu\r\n", heartbeat_counter);
        heartbeat_counter++;
    } else {
        UART_Debug_Printf("[TF] Failed to send heartbeat\r\n");
    }
}

/* ============================================================================
 * Test Entry Point
 * ========================================================================= */

void User_Entry(void)
{
    // Initialize UART
    UART_Init();
    HAL_Delay(100);  // Give UART time to stabilize
    
    UART_Debug_Printf("\r\n");
    UART_Debug_Printf("================================\r\n");
    UART_Debug_Printf("  TinyFrame Test Suite\r\n");
    UART_Debug_Printf("================================\r\n");
    UART_Debug_Printf("Version: TinyFrame v3.x\r\n");
    UART_Debug_Printf("UART: Channel %d\r\n", TINYFRAME_UART_CHANNEL);
    UART_Debug_Printf("Max RX Payload: %d bytes\r\n", TF_MAX_PAYLOAD_RX);
    UART_Debug_Printf("Max TX Payload: %d bytes\r\n", TF_MAX_PAYLOAD_TX);
    UART_Debug_Printf("Checksum: CRC%d\r\n", TF_CKSUM_TYPE);
    UART_Debug_Printf("\r\n");
    
    // Initialize TinyFrame
    tf = TinyFrame_Init();
    if (tf == NULL) {
        UART_Debug_Printf("[ERROR] TinyFrame initialization failed!\r\n");
        while (1);
    }
    
    UART_Debug_Printf("[OK] TinyFrame initialized\r\n");
    
    // Register frame listeners
    TF_AddTypeListener(tf, FRAME_TYPE_ECHO, echo_listener);
    TF_AddTypeListener(tf, FRAME_TYPE_COMMAND, command_listener);
    TF_AddTypeListener(tf, FRAME_TYPE_DATA, data_listener);
    TF_AddGenericListener(tf, generic_listener);
    
    UART_Debug_Printf("[OK] Frame listeners registered\r\n");
    UART_Debug_Printf("\r\n");
    UART_Debug_Printf("Ready! Waiting for frames...\r\n");
    UART_Debug_Printf("Tip: Send frame type 0x01 for echo test\r\n");
    UART_Debug_Printf("     Send frame type 0x02 with cmd 0x01 for status\r\n");
    UART_Debug_Printf("\r\n");
    
    last_send_time = HAL_GetTick();
    
    // Main loop
    while (1) {
        // Poll UART driver (for DMA processing and error recovery)
        UART_Poll();
        
        // Process incoming TinyFrame data
        TinyFrame_Process(tf);
        
        #if ENABLE_PERIODIC_SEND
        // Send periodic heartbeat
        uint32_t now = HAL_GetTick();
        if (now - last_send_time >= PERIODIC_INTERVAL_MS) {
            send_heartbeat();
            last_send_time = now;
        }
        #endif
        
        // Small delay to prevent busy-waiting
        HAL_Delay(10);
    }
}
