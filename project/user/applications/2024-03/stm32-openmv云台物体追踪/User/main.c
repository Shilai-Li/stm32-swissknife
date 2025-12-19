/*
 * CubeMX Configuration Instructions for stm32-openmv云台物体追踪
 * ==========================================================
 *
 * 1. System Clock (RCC)
 *    ------------------
 *    - High Speed Clock (HSE): Crystal/Ceramic Resonator
 *    - Clock Configuration: Set System Clock (SYSCLK) to 72MHz
 *
 * 2. GPIO Configuration
 *    ------------------
 *    - PC13: Output Push Pull, Label: LED0
 *    - PD2:  Output Push Pull, Label: LED1
 *
 * 3. Connectivity (USART1)
 *    ---------------------
 *    - Mode: Asynchronous
 *    - Baud Rate: 115200 Bits/s
 *    - Word Length: 8 Bits
 *    - Parity: None
 *    - Stop Bits: 1
 *    - NVIC Settings: **Enable USART1 global interrupt** (Critical for reception)
 *
 * 4. Timers (TIM2) - Servo Control
 *    -----------------------------
 *    - Clock Source: Internal Clock
 *    - Channel 1: PWM Generation CH1
 *    - Channel 2: PWM Generation CH2
 *    - Prescaler (PSC): 719  (72MHz / 720 = 100kHz)
 *    - Counter Period (ARR): 1999 (100kHz / 2000 = 50Hz / 20ms)
 *    - Pulse: 0 (Initial)
 *
 * 5. NVIC (Interrupts)
 *    -----------------
 *    - Priority Group: 2 bits (or default)
 *    - Ensure USART1 Global Interrupt is Enabled
 *
 * 6. Code Generation & User Code
 *    ---------------------------
 *    - Generate peripheral initialization as .c/.h files (recommended)
 *    - **CRITICAL**: In main.c USER CODE BEGIN 2, you MUST manually start PWM:
 *      HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
 *      HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
 */
#include "main.h"
#include "stm32f1xx_hal.h"
#include "control.h"

/* External Handles */
extern TIM_HandleTypeDef htim2;
extern UART_HandleTypeDef huart1;

/* Global Variables */
uint8_t rx_data[1];

u8 flag = 0;

/* UART Receive Callback (Interrupt) */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        uint8_t cmd = rx_data[0];
        
        switch (cmd)
        {
            case '1': // Bottom-Left: Move Left (0) + Down (0) -> Check Variable_X/Y logic
                // Assuming Variable_X(0) means increase angle (Right?), 1 means decrease (Left?)
                // Based on traces: '1': Variable_X(0), Variable_Y(0)
                Control_X(Variable_X(0));
                Control_Y(Variable_Y(0));
                HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET); // LED control
                break;
                
            case '2': // Bottom-Right
                // Trace: '2': Variable_X(1), Variable_Y(0)
                Control_X(Variable_X(1));
                Control_Y(Variable_Y(0));
                HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
                break;
                
            case '3': // Top-Left
                // Trace: '3': Variable_X(0), Variable_Y(1)
                Control_X(Variable_X(0));
                Control_Y(Variable_Y(1));
                HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
                break;
                
            case '4': // Top-Right
                // Trace: '4': Variable_X(1), Variable_Y(1)
                Control_X(Variable_X(1));
                Control_Y(Variable_Y(1));
                HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
                break;
                
            case '5': // Center Target
                // Do nothing, or maybe stop moving
                HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
                break;
                
            default:
                break;
        }
        
        // Restart Interrupt Receive
        HAL_UART_Receive_IT(&huart1, rx_data, 1);
    }
}

int main()
{
    /* 
     * NOTE: System Clock, GPIO, and Peripheral initialization 
     * are assumed to be handled by the CubeMX generated SystemClock_Config() 
     * and MX_..._Init() functions called in main.c (generated part)
     */
    
    /* Initialize PWM Channels */
    // Note: You must ensure MX_TIM2_Init() calls HAL_TIM_PWM_Start() or call it here:
    // HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
    // HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
    
    // Initial Position (Center)
    Control_X(90);
    Control_Y(120);
    
    HAL_Delay(2500);

    /* Start Receiving UART Data */
    HAL_UART_Receive_IT(&huart1, rx_data, 1);

    while(1)
    {
        HAL_Delay(500);
        // Toggle LED to show aliveness if needed, or simple heartbeat
        // HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_2); 
    }
}
