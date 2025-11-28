#include "gpio.h"

void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Enable GPIOA, GPIOB, GPIOC and GPIOD clocks */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD, ENABLE);

    /* Configure PC13 (LED) as Output Push-Pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; // Low speed equivalent
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    /* Configure PA2 (Reset), PA3 (CS), PA4 (DC) as Output Push-Pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // High speed for SPI control
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* Set initial states */
    GPIO_ResetBits(GPIOC, GPIO_Pin_13); // LED On (assuming active low)
    
    GPIO_SetBits(GPIOA, GPIO_Pin_3 | GPIO_Pin_2); // CS High, Reset High
    GPIO_ResetBits(GPIOA, GPIO_Pin_4); // DC Low
}
