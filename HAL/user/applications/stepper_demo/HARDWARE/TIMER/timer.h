#ifndef _TIMER_H
#define _TIMER_H
#include "sys.h"

extern TIM_HandleTypeDef TIM3_Handler;      //Timer handle 

void TIM3_Init(u16 arr,u16 psc);
#endif

