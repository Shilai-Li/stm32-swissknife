#include "timer.h"
#include "led.h"



TIM_HandleTypeDef TIM3_Handler;      //Timer handle 

//General timer 3 interrupt initialization
//arr: auto reload value
//psc: clock prescaler
//Timer overflow time calculation: Tout=((arr+1)*(psc+1))/Ft us.
//Ft=Timer clock frequency, unit: Mhz
//Here we use timer 3! (Timer 3 is on APB1, clock is HCLK/2)
void TIM3_Init(u16 arr,u16 psc)
{  
    TIM3_Handler.Instance=TIM3;                          //General timer 3
    TIM3_Handler.Init.Prescaler=psc;                     //Prescaler
    TIM3_Handler.Init.CounterMode=TIM_COUNTERMODE_UP;    //Up counting mode
    TIM3_Handler.Init.Period=arr;                        //Auto reload value
    TIM3_Handler.Init.ClockDivision=TIM_CLOCKDIVISION_DIV1;//Clock division
    HAL_TIM_Base_Init(&TIM3_Handler);
    
    HAL_TIM_Base_Start_IT(&TIM3_Handler); //Enable timer 3 and timer 3 update interrupt, TIM_IT_UPDATE   
}

//Timer bottom driver initialization interrupt priority
//This function will be called by HAL_TIM_Base_Init()
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
    if(htim->Instance==TIM3)
	{
		__HAL_RCC_TIM3_CLK_ENABLE();            //Enable TIM3 clock
		HAL_NVIC_SetPriority(TIM3_IRQn,1,3);    //Set interrupt priority, preemption priority 1, sub priority 3
		HAL_NVIC_EnableIRQ(TIM3_IRQn);          //Enable ITM3 interrupt   
	}
}

//Timer 3 interrupt service function
void TIM3_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&TIM3_Handler);
}

//Callback function, timer interrupt call function
int val=0;
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim==(&TIM3_Handler))
    {
			if(val<10000)//5 seconds
			{
				val++;			
        pwm=!pwm;        //pwm flip signal
			}
    }
}
