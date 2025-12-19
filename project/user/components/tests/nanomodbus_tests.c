/**
 * @file nanomodbus_tests.c
 * @brief nanoMODBUS Component Test Cases
 * 
 * This file contains test cases for the nanoMODBUS Modbus RTU library.
 * 
 * CubeMX Configuration Required:
 * ================================
 * 
 * 1. USART Configuration (for Modbus RTU):
 *    - Peripheral: USART1 (or USART2)
 *    - Mode: Asynchronous
 *    - Baud Rate: 9600 (or 19200/115200 for higher speeds)
 *    - Word Length: 8 Bits
 *    - Parity: Even (or None, depending on Modbus requirements)
 *    - Stop Bits: 1
 *    - Data Direction: Receive and Transmit
 *    - Over Sampling: 16 Samples
 * 
 * 2. GPIO Configuration:
 *    - Optional: RS485 DE/RE control pin (if using RS485 transceiver)
 * 
 * 3. NVIC Settings:
 *    - USART1 global interrupt: Enabled (if using interrupts, optional for this test)
 * 
 * Wiring:
 * =======
 * For loopback testing (client/server on same board):
 *    - TX → RX (connect UART TX to RX directly)
 *    - Requires two UART peripherals, or use external Modbus device
 * 
 * For external Modbus device:
 *    - TX → Modbus RX
 *    - RX → Modbus TX
 *    - GND → GND
 * 
 * Test Modes:
 * ===========
 * - TEST_SERVER: Run as Modbus RTU server (slave)
 * - TEST_CLIENT: Run as Modbus RTU client (master)
 */

#include "stm32f1xx_hal.h"
#include "nanomodbus.h"
#include "uart.h"
#include <stdio.h>
#include <string.h>

/* Test mode selection (set only ONE to 1) */
#define TEST_SERVER 1
#define TEST_CLIENT 0

/* External UART handle (from main.c or CubeMX generated code) */
extern UART_HandleTypeDef huart2;

/* Print helper */
#define PRINT(fmt, ...) do { \
    char buf[128]; \
    snprintf(buf, sizeof(buf), fmt "\r\n", ##__VA_ARGS__); \
    UART_Send(UART_DEBUG_CHANNEL, (uint8_t*)buf, strlen(buf)); \
} while(0)

/* ========================================================================
 * Test Case 1: Modbus RTU Server (Slave)
 * ======================================================================== */
#if TEST_SERVER

static nmbs_t modbus_server;
static nmbs_server_data_t server_data;

void Test_ModbusRTU_Server(void) {
    PRINT("=== nanoMODBUS RTU Server Test ===");
    PRINT("Unit ID: 1");
    PRINT("UART: 9600 8E1");
    PRINT("Waiting for Modbus requests...\r\n");
    
    /* Initialize Modbus server */
    nmbs_error err = nmbs_server_init_rtu(&modbus_server, &server_data, 0x01, &huart2);
    if (err != NMBS_ERROR_NONE) {
        PRINT("ERROR: Server init failed: %s", nmbs_strerror(err));
        return;
    }
    
    /* Initialize some test data */
    server_data.regs[0] = 0x1234;  /* Register 0 */
    server_data.regs[1] = 0x5678;  /* Register 1 */
    server_data.regs[2] = 0xABCD;  /* Register 2 */
    nmbs_bitfield_write(server_data.coils, 0, 1);  /* Coil 0 = ON */
    nmbs_bitfield_write(server_data.coils, 1, 0);  /* Coil 1 = OFF */
    
    PRINT("Initial data:");
    PRINT("  Reg[0] = 0x%04X", server_data.regs[0]);
    PRINT("  Reg[1] = 0x%04X", server_data.regs[1]);
    PRINT("  Reg[2] = 0x%04X", server_data.regs[2]);
    PRINT("  Coil[0] = %d", nmbs_bitfield_read(server_data.coils, 0));
    PRINT("  Coil[1] = %d\r\n", nmbs_bitfield_read(server_data.coils, 1));
    
    uint32_t request_count = 0;
    
    /* Main server loop */
    while (1) {
        /* Poll for incoming Modbus requests */
        err = nmbs_server_poll(&modbus_server);
        
        if (err == NMBS_ERROR_NONE) {
            request_count++;
            PRINT("[%lu] Request processed successfully", request_count);
            
            /* Show updated data */
            PRINT("  Current Reg[0] = 0x%04X", server_data.regs[0]);
            PRINT("  Current Coil[0] = %d", nmbs_bitfield_read(server_data.coils, 0));
            
        } else if (err == NMBS_ERROR_TIMEOUT) {
            /* Timeout is normal when no requests */
            HAL_Delay(10);
            
        } else {
            /* Error occurred */
            PRINT("ERROR: %s", nmbs_strerror(err));
            HAL_Delay(100);
        }
    }
}

#endif /* TEST_SERVER */

/* ========================================================================
 * Test Case 2: Modbus RTU Client (Master)
 * ======================================================================== */
#if TEST_CLIENT

static nmbs_t modbus_client;

void Test_ModbusRTU_Client(void) {
    PRINT("=== nanoMODBUS RTU Client Test ===");
    PRINT("Target Unit ID: 1");
    PRINT("UART: 9600 8E1\r\n");
    
    /* Initialize Modbus client */
    nmbs_error err = nmbs_client_init_rtu(&modbus_client, &huart2);
    if (err != NMBS_ERROR_NONE) {
        PRINT("ERROR: Client init failed: %s", nmbs_strerror(err));
        return;
    }
    
    /* Set target server address */
    nmbs_set_destination_rtu_address(&modbus_client, 0x01);
    
    uint16_t regs[10];
    uint8_t coils[32];
    uint32_t test_count = 0;
    
    HAL_Delay(1000);  /* Wait for server to be ready */
    
    /* Main client test loop */
    while (1) {
        test_count++;
        PRINT("\r\n--- Test Cycle %lu ---", test_count);
        
        /* Test 1: Read Holding Registers (FC 03) */
        PRINT("1. Read Holding Registers [0-2]...");
        err = nmbs_read_holding_registers(&modbus_client, 0, 3, regs);
        if (err == NMBS_ERROR_NONE) {
            PRINT("   SUCCESS: Reg[0]=0x%04X, Reg[1]=0x%04X, Reg[2]=0x%04X", 
                   regs[0], regs[1], regs[2]);
        } else {
            PRINT("   ERROR: %s", nmbs_strerror(err));
        }
        HAL_Delay(500);
        
        /* Test 2: Write Single Register (FC 06) */
        uint16_t test_value = 0x1000 + test_count;
        PRINT("2. Write Single Register [0] = 0x%04X...", test_value);
        err = nmbs_write_single_register(&modbus_client, 0, test_value);
        if (err == NMBS_ERROR_NONE) {
            PRINT("   SUCCESS");
        } else {
            PRINT("   ERROR: %s", nmbs_strerror(err));
        }
        HAL_Delay(500);
        
        /* Test 3: Read back to verify */
        PRINT("3. Read back Register [0]...");
        err = nmbs_read_holding_registers(&modbus_client, 0, 1, regs);
        if (err == NMBS_ERROR_NONE) {
            if (regs[0] == test_value) {
                PRINT("   SUCCESS: Verified = 0x%04X", regs[0]);
            } else {
                PRINT("   MISMATCH: Got 0x%04X, Expected 0x%04X", regs[0], test_value);
            }
        } else {
            PRINT("   ERROR: %s", nmbs_strerror(err));
        }
        HAL_Delay(500);
        
        /* Test 4: Write Multiple Registers (FC 16) */
        regs[0] = 0xAAAA;
        regs[1] = 0xBBBB;
        regs[2] = 0xCCCC;
        PRINT("4. Write Multiple Registers [0-2]...");
        err = nmbs_write_multiple_registers(&modbus_client, 0, 3, regs);
        if (err == NMBS_ERROR_NONE) {
            PRINT("   SUCCESS");
        } else {
            PRINT("   ERROR: %s", nmbs_strerror(err));
        }
        HAL_Delay(500);
        
        /* Test 5: Read Coils (FC 01) */
        PRINT("5. Read Coils [0-7]...");
        err = nmbs_read_coils(&modbus_client, 0, 8, coils);
        if (err == NMBS_ERROR_NONE) {
            PRINT("   SUCCESS: Coil[0]=%d, Coil[1]=%d", 
                   nmbs_bitfield_read(coils, 0), nmbs_bitfield_read(coils, 1));
        } else {
            PRINT("   ERROR: %s", nmbs_strerror(err));
        }
        HAL_Delay(500);
        
        /* Test 6: Write Single Coil (FC 05) */
        bool coil_value = (test_count % 2);  /* Toggle */
        PRINT("6. Write Single Coil [0] = %d...", coil_value);
        err = nmbs_write_single_coil(&modbus_client, 0, coil_value);
        if (err == NMBS_ERROR_NONE) {
            PRINT("   SUCCESS");
        } else {
            PRINT("   ERROR: %s", nmbs_strerror(err));
        }
        
        HAL_Delay(2000);  /* Wait before next cycle */
    }
}

#endif /* TEST_CLIENT */

/* ========================================================================
 * Main Test Entry Point
 * ======================================================================== */

void User_Entry(void) {
    /* Wait for UART to be ready */
    HAL_Delay(100);
    
    PRINT("\r\n\r\n");
    PRINT("╔════════════════════════════════════════╗");
    PRINT("║   nanoMODBUS RTU Component Test       ║");
    PRINT("╚════════════════════════════════════════╝");
    PRINT("");
    
#if TEST_SERVER
    Test_ModbusRTU_Server();
#elif TEST_CLIENT
    Test_ModbusRTU_Client();
#else
    #error "Please select TEST_SERVER or TEST_CLIENT"
#endif
    
    /* Should never reach here */
    while (1) {
        HAL_Delay(1000);
    }
}
