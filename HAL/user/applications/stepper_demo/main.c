/*
 * CubeMX Configuration for Stepper Motor Example
 * -------------------------------------------
 * 1. RCC Configuration:
 *    - High Speed Clock (HSE): Crystal/Ceramic Resonator (25 MHz)
 *    - System Clock: 180 MHz (PLL: M=25, N=360, P=2, Q=8)
 *    - APB1 Prescaler: /4 (45 MHz) -> Timer Clock: 90 MHz
 *    - APB2 Prescaler: /2 (90 MHz)
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
 *      - Prescaler: 71 (Count Freq: 90MHz / 72 = 1.25 MHz)
 *      - Period: 5000  (Update Freq: 1.25MHz / 5000 = 250 Hz)
 *      - NVIC: TIM3 global interrupt enabled
 *
 * 5. GPIO Configuration:
 *    - PB0: Output Push Pull, Pull-Up, High Speed (PWM/Pulse for Motor)
 *    - PB1: Output Push Pull, Pull-Up, High Speed (LED0 / Dir for Motor)
 *    - PA0: Input, Pull-Down (KEY0)
 *    - PC13: Input, Pull-Up (KEY1)
 *    - PH2, PH3: Input, Pull-Up (KEY2, WK_UP)
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
    TIM3_Init(5000,71);       //2KHz
    dir=0;//     Direction                          
    while(1)
    {
        LED0=!LED0;                 //LED0翻转
        delay_ms(200);              //延时200ms
    }
}

