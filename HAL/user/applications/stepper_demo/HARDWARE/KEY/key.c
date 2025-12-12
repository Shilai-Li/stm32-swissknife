#include "key.h"
#include "delay.h"

//Initialize buttons
void KEY_Init(void)
{
    GPIO_InitTypeDef GPIO_Initure;
    
    __HAL_RCC_GPIOA_CLK_ENABLE();           //Enable GPIOA clock
    __HAL_RCC_GPIOC_CLK_ENABLE();           //Enable GPIOC clock
    __HAL_RCC_GPIOH_CLK_ENABLE();           //Enable GPIOH clock
    
    GPIO_Initure.Pin=GPIO_PIN_0;            //PA0
    GPIO_Initure.Mode=GPIO_MODE_INPUT;      //Input mode
    GPIO_Initure.Pull=GPIO_PULLDOWN;        //Pull-down
    GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //High speed
    HAL_GPIO_Init(GPIOA,&GPIO_Initure);
    
    GPIO_Initure.Pin=GPIO_PIN_13;           //PC13
    GPIO_Initure.Mode=GPIO_MODE_INPUT;      //Input mode
    GPIO_Initure.Pull=GPIO_PULLUP;          //Pull-up
    GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //High speed
    HAL_GPIO_Init(GPIOC,&GPIO_Initure);
    
    GPIO_Initure.Pin=GPIO_PIN_2|GPIO_PIN_3; //PH2,3
    HAL_GPIO_Init(GPIOH,&GPIO_Initure);
}

//Button scan function
//Return button value
//mode:0, does not support continuous press; 1, supports continuous press;
//0 means no button is pressed
//1 means WKUP button WK_UP
//Note that this function responds to priority, KEY0>KEY1>KEY2>WK_UP!!
u8 KEY_Scan(u8 mode)
{
    static u8 key_up=1;     //Button release flag
    if(mode==1)key_up=1;    //Support continuous press
    if(key_up&&(KEY0==0||KEY1==0||KEY2==0||WK_UP==1))
    {
        delay_ms(10);
        key_up=0;
        if(KEY0==0)       return KEY0_PRES;
        else if(KEY1==0)  return KEY1_PRES;
        else if(KEY2==0)  return KEY2_PRES;
        else if(WK_UP==1) return WKUP_PRES;          
    }else if(KEY0==1&&KEY1==1&&KEY2==1&&WK_UP==0)key_up=1;
    return 0;   //No button operation
}
