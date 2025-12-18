/**
 * @file can_driver.c
 * @brief CAN Driver Wrapper Implementation
 * @author Standard Implementation
 * @date 2024
 */

#include "can_driver.h"
#include <string.h>

void CAN_Driver_Init(CAN_Driver_HandleTypeDef *hdriver, CAN_HandleTypeDef *hcan) {
    hdriver->hcan = hcan;
    hdriver->RxHead = 0;
    hdriver->RxTail = 0;
    hdriver->FilterConfigured = 0;
}

void CAN_Driver_ConfigFilter_AcceptAll(CAN_Driver_HandleTypeDef *hdriver) {
    CAN_FilterTypeDef sFilterConfig;

    sFilterConfig.FilterBank = 0;
    sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
    sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
    sFilterConfig.FilterIdHigh = 0x0000;
    sFilterConfig.FilterIdLow = 0x0000;
    sFilterConfig.FilterMaskIdHigh = 0x0000;
    sFilterConfig.FilterMaskIdLow = 0x0000;
    sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
    sFilterConfig.FilterActivation = ENABLE;
    sFilterConfig.SlaveStartFilterBank = 14;

    if (HAL_CAN_ConfigFilter(hdriver->hcan, &sFilterConfig) == HAL_OK) {
        hdriver->FilterConfigured = 1;
    }
}

void CAN_Driver_ConfigFilter_ID(CAN_Driver_HandleTypeDef *hdriver, uint32_t bank, uint32_t id) {
    CAN_FilterTypeDef sFilterConfig;

    // Standard ID shifted left by 5 to match register mapping for 32bit mode
    uint32_t regID = (id << 21); // STID[10:0] is bits 31:21 in 32-bit filter

    sFilterConfig.FilterBank = bank;
    sFilterConfig.FilterMode = CAN_FILTERMODE_IDLIST;
    sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
    
    // We can filter 2 IDs per bank in 32-bit list mode, or 4 in 16-bit. 
    // Simplified: Just Accept One ID, mask unused.
    sFilterConfig.FilterIdHigh = (regID >> 16) & 0xFFFF;
    sFilterConfig.FilterIdLow = regID & 0xFFFF;
    sFilterConfig.FilterMaskIdHigh = (regID >> 16) & 0xFFFF; // Second ID in List Mode
    sFilterConfig.FilterMaskIdLow = regID & 0xFFFF;
    
    sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
    sFilterConfig.FilterActivation = ENABLE;
    sFilterConfig.SlaveStartFilterBank = 14;

    if (HAL_CAN_ConfigFilter(hdriver->hcan, &sFilterConfig) == HAL_OK) {
        hdriver->FilterConfigured = 1;
    }
}

uint8_t CAN_Driver_Start(CAN_Driver_HandleTypeDef *hdriver) {
    // Ensure at least one filter is configured, otherwise traffic is blocked by default hardware logic
    if (!hdriver->FilterConfigured) {
        CAN_Driver_ConfigFilter_AcceptAll(hdriver);
    }

    if (HAL_CAN_Start(hdriver->hcan) != HAL_OK) return 1;
    
    // Activate Notification
    if (HAL_CAN_ActivateNotification(hdriver->hcan, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK) return 2;
    
    return 0;
}

uint8_t CAN_Driver_Send(CAN_Driver_HandleTypeDef *hdriver, uint32_t id, uint8_t *pData, uint8_t len) {
    CAN_TxHeaderTypeDef TxHeader;
    
    if (len > 8) len = 8;

    TxHeader.StdId = id;
    TxHeader.ExtId = 0x01;
    TxHeader.RTR = CAN_RTR_DATA;
    TxHeader.IDE = CAN_ID_STD;
    TxHeader.DLC = len;
    TxHeader.TransmitGlobalTime = DISABLE;
    
    // Wait if no mailbox available? Or return error?
    // Retry a few times
    uint32_t timeout = 1000;
    while (HAL_CAN_GetTxMailboxesFreeLevel(hdriver->hcan) == 0) {
        timeout--;
        if (timeout == 0) return 2; // Busy
    }

    if (HAL_CAN_AddTxMessage(hdriver->hcan, &TxHeader, pData, &hdriver->TxMailbox) != HAL_OK) {
        return 1; // Error
    }
    
    return 0; 
}

// --- Software FIFO Management ---

static uint8_t CAN_Buffer_Full(CAN_Driver_HandleTypeDef *h) {
    return ((h->RxHead + 1) % CAN_RX_BUFFER_SIZE) == h->RxTail;
}

static uint8_t CAN_Buffer_Empty(CAN_Driver_HandleTypeDef *h) {
    return h->RxHead == h->RxTail;
}

void CAN_Driver_RxCpltCallback(CAN_Driver_HandleTypeDef *hdriver) {
    CAN_RxHeaderTypeDef Header;
    uint8_t Data[8];

    // Read from Hardware FIFO
    while (HAL_CAN_GetRxFifoFillLevel(hdriver->hcan, CAN_RX_FIFO0) > 0) {
        if (HAL_CAN_GetRxMessage(hdriver->hcan, CAN_RX_FIFO0, &Header, Data) == HAL_OK) {
            // Push to Software FIFO
            if (!CAN_Buffer_Full(hdriver)) {
                hdriver->RxBuffer[hdriver->RxHead].Header = Header;
                memcpy(hdriver->RxBuffer[hdriver->RxHead].Data, Data, 8);
                
                hdriver->RxHead = (hdriver->RxHead + 1) % CAN_RX_BUFFER_SIZE;
            } else {
                // Buffer Overflow - Drop packet or flag error
            }
        }
    }
}

uint8_t CAN_Driver_Available(CAN_Driver_HandleTypeDef *hdriver) {
    // Check Software Buffer + Check Hardware Buffer (in case Interrupt disabled or polling mode)
    // Here we rely on Interrupt filling the SW Buffer
    return !CAN_Buffer_Empty(hdriver);
}

uint8_t CAN_Driver_Read(CAN_Driver_HandleTypeDef *hdriver, CAN_Frame_t *frame) {
    if (CAN_Buffer_Empty(hdriver)) return 1;
    
    // Pop from Software FIFO
    *frame = hdriver->RxBuffer[hdriver->RxTail];
    hdriver->RxTail = (hdriver->RxTail + 1) % CAN_RX_BUFFER_SIZE;
    
    return 0;
}
