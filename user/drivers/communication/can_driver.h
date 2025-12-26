/**
 * @file can_driver.h
 * @brief CAN Driver Wrapper for STM32 HAL
 * 
 * =================================================================================
 *                       >>> INTEGRATION GUIDE <<<
 * =================================================================================
 * 1. CubeMX Config (Connectivity -> CAN):
 *    - Mode: Master (or Normal)
 *    - Bit Timing Parameters (Important!):
 *      * Prescaler: Depends on APB1 Clock. 
 *        Example (36MHz APB1 -> 500kbps): Prescaler=4, BS1=15, BS2=2. (Sample Point 87.5%)
 *      * Calculate: Baud = APB1 / (Prescaler * (1 + BS1 + BS2))
 *    - NVIC Settings:
 *      * Enable "CAN1 RX0 interrupt" (Essential for Receive)
 * 
 * 2. Code Integration:
 *    - Ensure 'CAN_Init(&hcan1)' is called.
 *    - The driver automatically sets up a default filter to Accept All IDs.
 *    - To receive, you must implement HAL_CAN_RxFifo0MsgPendingCallback (or let driver do it if handled).
 * =================================================================================
 */

#ifndef __CAN_DRIVER_H
#define __CAN_DRIVER_H

#include "main.h"

#ifndef __STM32F1xx_HAL_CAN_H
#include "main.h"
#endif

// --- Configuration ---
// Define Rx Buffer Size (Simple software FIFO)
#define CAN_RX_BUFFER_SIZE 16

typedef struct {
    CAN_RxHeaderTypeDef Header;
    uint8_t             Data[8];
} CAN_Frame_t;

typedef struct {
    CAN_HandleTypeDef *hcan;
    uint32_t           TxMailbox;
    // Simple Circular Buffer for Rx
    CAN_Frame_t        RxBuffer[CAN_RX_BUFFER_SIZE];
    volatile uint8_t   RxHead;
    volatile uint8_t   RxTail;
    uint8_t            FilterConfigured;
} CAN_Driver_HandleTypeDef;

/* Function Prototypes */

/**
 * @brief Initialize the CAN Driver Wrapper
 * @param hdriver Driver Handle
 * @param hcan HAL CAN Handle (already initialized by CubeMX)
 */
void CAN_Driver_Init(CAN_Driver_HandleTypeDef *hdriver, CAN_HandleTypeDef *hcan);

/**
 * @brief Start the CAN module and interrupts
 * @return 0 on success
 */
uint8_t CAN_Driver_Start(CAN_Driver_HandleTypeDef *hdriver);

/**
 * @brief Setup a default "Accept All" filter
 */
void CAN_Driver_ConfigFilter_AcceptAll(CAN_Driver_HandleTypeDef *hdriver);

/**
 * @brief Setup a specific ID filter
 * @param bank Filter bank number (0-13 or 0-27 depending on MCU)
 * @param id Standard ID to accept
 */
void CAN_Driver_ConfigFilter_ID(CAN_Driver_HandleTypeDef *hdriver, uint32_t bank, uint32_t id);

/**
 * @brief Send a standard data frame
 * @param id Standard ID (11-bit)
 * @param pData POinter to data (max 8 bytes)
 * @param len Length (0-8)
 * @return 0 on success
 */
uint8_t CAN_Driver_Send(CAN_Driver_HandleTypeDef *hdriver, uint32_t id, uint8_t *pData, uint8_t len);

/**
 * @brief Check if data is available in Rx FIFO
 */
uint8_t CAN_Driver_Available(CAN_Driver_HandleTypeDef *hdriver);

/**
 * @brief Read a frame from Rx FIFO
 * @param frame Pointer to frame structure to fill
 * @return 0 on success, 1 on empty
 */
uint8_t CAN_Driver_Read(CAN_Driver_HandleTypeDef *hdriver, CAN_Frame_t *frame);

// --- Interrupt Callback Handler ---
// Call this from HAL_CAN_RxFifo0MsgPendingCallback in main.c or call CAN_Driver_ProcessRx in it
void CAN_Driver_RxCpltCallback(CAN_Driver_HandleTypeDef *hdriver);

#endif // __CAN_DRIVER_H
