#include "led.h"

//Initialize PB1 as output. Enable clock	    
//LED IO initialization
void LED_Init(void)
{
    GPIO_InitTypeDef GPIO_Initure;
    __HAL_RCC_GPIOB_CLK_ENABLE();           //Enable GPIOB clock
	
    GPIO_Initure.Pin=GPIO_PIN_0|GPIO_PIN_1; //PB1,0
    GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //Push-pull output
    GPIO_Initure.Pull=GPIO_PULLUP;          //Pull-up
    GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //High speed
    HAL_GPIO_Init(GPIOB,&GPIO_Initure);
	
    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);	//PB0 set to 1 
    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,GPIO_PIN_SET);	//PB1 set to 1  
}
