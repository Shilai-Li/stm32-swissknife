/*
 * CubeMX Configuration for Stepper Motor Example (Ported to STM32F103C8T6)
 * -------------------------------------------
 * 1. RCC Configuration:
 *    - High Speed Clock (HSE): Crystal/Ceramic Resonator (8 MHz)
 *    - System Clock: 72 MHz (PLL: Mul=9)
 *    - APB1 Prescaler: /2 (36 MHz) -> Timer Clock: 72 MHz
 *    - APB2 Prescaler: /1 (72 MHz)
 *
 * 2. SYS Configuration:
 *    - Debug: Serial Wire
 *    - Timebase Source: SysTick
 *
 * 3. Connectivity:
 *    - USART1 (Asynchronous):
 *      - Baud Rate: 115200
 *      - Word Length: 8 Bits
 *      - Parity: None
 *      - Stop Bits: 1
 *      - Pins: PA9 (TX), PA10 (RX)
 *
 * 4. Timers:
 *    - TIM3 (Internal Clock):
 *      - Prescaler: 57 (Count Freq: 72MHz / 58 ~= 1.25 MHz)
 *      - Period: 5000  (Update Freq: 1.25MHz / 5000 = 250 Hz)
 *      - NVIC: TIM3 global interrupt enabled
 *
 * 5. GPIO Configuration:
 *    - PB0: Output Push Pull, Pull-Up, High Speed (PWM/Pulse for Motor)
 *    - PB1: Output Push Pull, Pull-Up, High Speed (LED0 / Dir for Motor)
 *    - PA0: Input, Pull-Down (KEY0)
 *    - PC13: Input, Pull-Up (KEY1)
 *    - PA1: Input, Pull-Up (KEY2) - [Remapped from PH2]
 *    - PA2: Input, Pull-Up (WK_UP) - [Remapped from PH3]
 *
 * 6. Critical Integration Steps:
 *    a. Include Paths: Add these directories to your IDE's Include Path:
 *       - HAL/user/applications/stepper_demo/SYSTEM/sys
 *       - HAL/user/applications/stepper_demo/SYSTEM/delay
 *       - HAL/user/applications/stepper_demo/SYSTEM/usart
 *       - HAL/user/applications/stepper_demo/HARDWARE/LED
 *       - HAL/user/applications/stepper_demo/HARDWARE/KEY
 *       - HAL/user/applications/stepper_demo/HARDWARE/TIMER
 *
 *    b. Interrupt Configuration:
 *       - EXCLUDE or COMMENT OUT `TIM3_IRQHandler` and `USART1_IRQHandler` 
 *         in `stm32f4xx_it.c`. (They are defined in timer.c and usart.c)
 *
 *    c. STM32F103C8T6 Portability Notes:
 *       1. Hardware Incompatibility: 
 *          - F103C8T6 (48-pin) has NO Port H.
 *          - Action: Remap KEY2 (PH2) and WK_UP (PH3) to available pins (e.g., PA1, PA2) in key.c.
 *       2. Software Configuration:
 *          - Include Files: Change "stm32f4xx.h" to "stm32f1xx.h" in sys.h.
 *          - Timer Clock: F103 TIM3 runs at 72MHz (F4 was 90MHz).
 *            - F4: 90MHz / 71 = 1.25MHz.
 *            - F1: 72MHz / 57 ~= 1.25MHz.
 *            - Action: Change TIM3_Init prescaler in main.c logic.
 *       3. Code Cleanup:
 *          - Ensure sys.c avoids F4-specific registers.
 */
#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "key.h"
#include "timer.h"

#include "stm32f1xx_hal.h"

void User_Entry(void)
{
    // HAL_Init();                     //Initialize HAL (Handled by main.c)
    // Stm32_Clock_Init(360,25,2,8);   //Set Clock (Handled by main.c / SystemClock_Config)
    delay_init(180);                //Initialize Delay (Ensure SysTick is available)
    // uart_init(115200);              //Initialize USART (Handled by MX_USART1_UART_Init)
    LED_Init();                     //Initialize LED
    KEY_Init();                     //Initialize buttons
    TIM3_Init(5000,57);       //2KHz (72MHz/57+1 ~= 1.25MHz, similar to 90MHz/71+1)
    dir=0;//     Direction                          
    while(1)
    {
        LED0=!LED0;                 //LED0翻转
        delay_ms(200);              //延时200ms
    }
}

