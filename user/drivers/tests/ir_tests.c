/**
 * @file ir_test.c
 * @brief Test both IR drivers
 */

#include "ir_nec_exti.h"
// #include "ir_nec_tim.h" // Uncomment if using TIM
#include "uart.h"
#include <stdio.h>

void Test_IR_Entry(void) {
    UART_Init();
    UART_Debug_Printf("\r\n=== IR Remote Test (EXTI Mode) ===\r\n");
    UART_Debug_Printf("Please press buttons on remote...\r\n");
    
    // EXTI Mode Example
    // Assume PB0 is connected to IR Receiver
    // CubeMX: GPIO_EXTI0, Falling Edge, NVIC Enabled.
    static IR_NEC_EXTI_Handle_t ir_exti;
    IR_NEC_EXTI_Init(&ir_exti, GPIO_PIN_0); 
    
    /* 
       Note: You must manually add call to IR_NEC_EXTI_Callback 
       in stm32f1xx_it.c -> HAL_GPIO_EXTI_Callback!
    */
    
    while(1) {
        if (IR_NEC_EXTI_Available(&ir_exti)) {
            uint16_t cmd = IR_NEC_EXTI_GetCommand(&ir_exti);
            UART_Debug_Printf("IR CMD: 0x%02X\r\n", cmd);
        }
    }
}
