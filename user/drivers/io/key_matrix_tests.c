/**
 * @file key_matrix_tests.c
 * @brief Test for 4x4 Matrix Keypad
 */

#include "io/key_matrix.h"
#include "usart.h"
#include "uart.h"
#include "usb_cdc.h"
#include <stdio.h>

#define CH_DEBUG 2

// 4x4 Keypad Map
// 1 2 3 A
// 4 5 6 B
// 7 8 9 C
// * 0 # D
static const char key_map[] = {
    '1', '2', '3', 'A',
    '4', '5', '6', 'B',
    '7', '8', '9', 'C',
    '*', '0', '#', 'D'
};

void app_main(void) {
    UART_Register(CH_DEBUG, &huart2);
    // UART_Init(); 

    UART_SendString(CH_DEBUG, "\r\n=== Matrix Keypad Test (4x4) ===\r\n");
    
    // Define HW Configuration
    // Rows: PC0, PC1, PC2, PC3
    static GPIO_TypeDef* row_ports[] = {GPIOC, GPIOC, GPIOC, GPIOC};
    static uint16_t      row_pins[]  = {GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_2, GPIO_PIN_3};
    
    // Cols: PC4, PC5, PC6, PC7
    static GPIO_TypeDef* col_ports[] = {GPIOC, GPIOC, GPIOC, GPIOC};
    static uint16_t      col_pins[]  = {GPIO_PIN_4, GPIO_PIN_5, GPIO_PIN_6, GPIO_PIN_7};
    
    // Initialize
    KeyMatrix_Init(row_ports, row_pins, 4,
                   col_ports, col_pins, 4);
                   
    UART_SendString(CH_DEBUG, "Scanning...\r\n");

    while(1) {
        if (KeyMatrix_Scan()) {
            MatrixEvent_t evt = KeyMatrix_GetEvent();
            
            if (evt.pressed) {
                char c = KeyMatrix_MapChar(key_map, evt);
                
                char buf[32];
                // Note: using sprintf requires stdio linkage, 
                // better to use pure string sends if stack is tight.
                // But for tests it's fine.
                snprintf(buf, 32, "Key Pressed: %c [%d,%d]\r\n", c, evt.row, evt.col);
                UART_SendString(CH_DEBUG, buf);
            }
        }
        
        HAL_Delay(20); // 50Hz Scan Rate
    }
}
