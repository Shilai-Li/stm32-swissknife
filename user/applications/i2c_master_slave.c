#include "main.h"
#include "drivers/communication/uart.h"
#include "drivers/system/delay.h" // For Delay_ms
#include <stdio.h>
#include <string.h>

/* 
 * I2C Role Configuration
 * Passed via CMake: -DDEMO_ROLE=1 (Master) or -DDEMO_ROLE=0 (Slave)
 */
#ifndef DEMO_ROLE
  #warning "No I2C Role defined! Defaulting to MASTER (1)."
  #define DEMO_ROLE 1
#endif

// --- Hardware Configuration ---
// Assuming I2C1 (PB6/PB7 on many STM32 boards)
// Ensure I2C1 is enabled in CubeMX/main.c
extern I2C_HandleTypeDef hi2c1;
#define DEMO_I2C_HANDLE &hi2c1

// Slave Address (7-bit: 0x20) -> Left Shifted for HAL (0x40)
#define SLAVE_ADDR_7BIT 0x20
#define SLAVE_ADDR_HAL  (SLAVE_ADDR_7BIT << 1)

void user_main(void)
{
    UART_Init();
    // Delay_Init(); 
    
    // --- FORCE PIN PULL-UP (If no external resistors) ---
    // Assuming I2C1 is on PB6 (SCL) and PB7 (SDA)
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOB_CLK_ENABLE();
    
    // We only modify the PULL setting, keeping AF and Speed (assuming CubeMX set them right)
    // Actually, HAL_GPIO_Init overwrites everything. So we must fully spec it.
    // Standard I2C: AF OD, Pull-Up, High Speed.
    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP; 
    
#if defined(STM32F1)
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    // F1 does not use .Alternate field
#else
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH; // F4
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1; // F4
#endif

    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    // ----------------------------------------------------
    
    UART_Debug_Printf("\r\n=================================\r\n");
    UART_Debug_Printf("    I2C Master/Slave Demo        \r\n");
    UART_Debug_Printf("=================================\r\n");
    
#if DEMO_ROLE == 1
    // ================= MASTER MODE =================
    UART_Debug_Printf("Role: MASTER (DEMO_ROLE=1)\r\n");
    UART_Debug_Printf("Target Slave Addr: 0x%02X\r\n", SLAVE_ADDR_HAL);
    
    uint8_t tx_count = 0;
    uint8_t buffer[32];
    
    while (1)
    {
        // 1. Prepare Data
        sprintf((char*)buffer, "Hello #%d", tx_count++);
        
        // 2. Transmit
        UART_Debug_Printf("Master TX: '%s' ... ", buffer);
        
        HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(
            DEMO_I2C_HANDLE, 
            SLAVE_ADDR_HAL, 
            buffer, 
            strlen((char*)buffer) + 1, // Include null terminator
            1000 // Timeout 1s
        );
        
        if (status == HAL_OK) {
            UART_Debug_Printf("ACK\r\n");
        } else {
            UART_Debug_Printf("NACK/ERR (%d)\r\n", HAL_I2C_GetError(DEMO_I2C_HANDLE));
        }
        
        HAL_Delay(1000); 
    }
    
#else
    // ================= SLAVE MODE =================
    UART_Debug_Printf("Role: SLAVE (DEMO_ROLE=0)\r\n");
    UART_Debug_Printf("Listening on Addr: 0x%02X\r\n", SLAVE_ADDR_HAL);
    
    // Check Address
    if (hi2c1.Init.OwnAddress1 != SLAVE_ADDR_HAL) {
        UART_Debug_Printf("Warning: I2C1 OwnAddr is 0x%X, overwriting to 0x%X...\r\n", hi2c1.Init.OwnAddress1, SLAVE_ADDR_HAL);
        
        HAL_I2C_DeInit(&hi2c1);
        hi2c1.Init.OwnAddress1 = SLAVE_ADDR_HAL;
        if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
            UART_Debug_Printf("I2C Re-Init Failed!\r\n");
            Error_Handler();
        }
    }

    uint8_t rx_buffer[32];
    
    while (1)
    {
        // UART_Debug_Printf("Slave: Waiting...\r\n");
        
        // Blocking Receive
        memset(rx_buffer, 0, sizeof(rx_buffer));
        
        // Reading 10 bytes for demo. 
        // Note: HAL_I2C_Slave_Receive blocks until EXACTLY Size bytes are received or Timeout.
        // If master sends fewer bytes, this will timeout!
        HAL_StatusTypeDef status = HAL_I2C_Slave_Receive(DEMO_I2C_HANDLE, rx_buffer, 10, 2000); 
        
        if (status == HAL_OK) {
            UART_Debug_Printf("Slave RX: '%s'\r\n", rx_buffer);
        } else if (status == HAL_TIMEOUT) {
            // Normal if no data
        } else {
             UART_Debug_Printf("Slave Err: %d\r\n", HAL_I2C_GetError(DEMO_I2C_HANDLE));
             // If AF/BERR error, resetting handle might be needed
             // HAL_I2C_Init(&hi2c1);
        }
    }
#endif
}
