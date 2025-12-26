/**
 * @file can_tests.c
 * @brief CAN Driver Test Code
 */

#include "can_driver.h"
#include "uart.h"

// --- Configuration ---
// Requires CAN1 initialized in main.c
extern CAN_HandleTypeDef hcan1;

CAN_Driver_HandleTypeDef hcan_drv;

// Important: You must glue the interrupt to the callback in main.c or stm32f1xx_it.c
// void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
//     if (hcan == &hcan1) {
//         CAN_Driver_RxCpltCallback(&hcan_drv);
//     }
// }
// For this test to work without modifying main.c/it.c, we can poll in the loop or use the callback if user adds it.
// We will use polling logic in the test loop for simplicity if interrupts aren't hooked up,
// BUT the driver is designed for Interrupts.
// To make it run standalone here without IT glue, we can manually call polling.

void app_main(void)
{
    UART_Init();
    UART_Debug_Printf("\r\n--- CAN Driver Test Start ---\r\n");

    // 1. Initialize
    CAN_Driver_Init(&hcan_drv, &hcan1);
    
    // 2. Configure Filter (Accept All)
    // CAN_Driver_ConfigFilter_AcceptAll(&hcan_drv); // Start() below calls this automatically if not set
    
    // 3. Start
    if (CAN_Driver_Start(&hcan_drv) == 0) {
        UART_Debug_Printf("CAN Started Successfully.\r\n");
    } else {
        UART_Debug_Printf("CAN Start Failed!\r\n");
        while(1) HAL_Delay(1000);
    }
    
    // 4. Send Test
    uint8_t payload[] = {0xDE, 0xAD, 0xBE, 0xEF};
    UART_Debug_Printf("Sending Frame ID 0x123...\r\n");
    
    if (CAN_Driver_Send(&hcan_drv, 0x123, payload, 4) == 0) {
        UART_Debug_Printf("Send OK.\r\n");
    } else {
        UART_Debug_Printf("Send Failed (Check Bus/Termination).\r\n");
    }

    UART_Debug_Printf("Entering Loopback/Monitor Mode...\r\n");

    CAN_Frame_t frame;
    while (1) {
        // Poll HW for this test context if interrupts aren't firing
        // Manually trigger the "Callback" logic to pull from HW FIFO to SW FIFO
        CAN_Driver_RxCpltCallback(&hcan_drv); 
        
        if (CAN_Driver_Available(&hcan_drv)) {
            if (CAN_Driver_Read(&hcan_drv, &frame) == 0) {
                UART_Debug_Printf("Rx ID: 0x%X DLC: %d Data: %02X %02X...\r\n", 
                    frame.Header.StdId, frame.Header.DLC, frame.Data[0], frame.Data[1]);
                
                // Echo back with ID + 1
                CAN_Driver_Send(&hcan_drv, frame.Header.StdId + 1, frame.Data, frame.Header.DLC);
            }
        }
        
        // Periodic heartbeat send
        static uint32_t last_tick = 0;
        if (HAL_GetTick() - last_tick > 1000) {
             uint8_t hb[] = {0x11};
             CAN_Driver_Send(&hcan_drv, 0x555, hb, 1);
             last_tick = HAL_GetTick();
             // UART_Debug_Printf("Heartbeat Sent.\r\n");
        }
    }
}
