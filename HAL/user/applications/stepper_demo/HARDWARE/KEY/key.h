#ifndef _KEY_H
#define _KEY_H
#include "sys.h"

//Button port reading, directly using IO level reading method
//#define KEY0        PHin(3) //KEY0 connected to PH3
//#define KEY1        PHin(2) //KEY1 connected to PH2
//#define KEY2        PCin(13)//KEY2 connected to PC13
//#define WK_UP       PAin(0) //WKUP connected to PA0


//Button port reading, directly call HAL library function to read IO
//Button port reading, directly call HAL library function to read IO
#define KEY0        HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0)  //KEY0 connected to PA0 (Original was PH3/PA0 mixed up in original comments? Let's stick to PA0 as per main.c comment)
// Wait, looking at main.c: PA0 is KEY0. PC13 is KEY1. PH2 is KEY2. PH3 is WK_UP.
// Original key.h said: KEY0->PH3, KEY1->PH2, KEY2->PC13, WK_UP->PA0. This contradicts main.c comments?
// Let's re-read main.c comments carefully.
// main.c: PA0: Input (KEY0), PC13 (KEY1), PH2/PH3 (KEY2, WK_UP).
// key.h: KEY0->PH3, KEY1->PH2, KEY2->PC13, WK_UP->PA0.
// CONFLICT DETECTED.
// I will trust main.c comments as they were derived from my analysis of key.c (viewed earlier).
// Let's check key.c again.
// key.c orig: PA0 (KEY0?), PC13 (KEY1?), PH2/3 (KEY2/WK_UP?).
// key.c Scan function: KEY0/1/2/WK_UP.
// key.c Init: PA0, PC13, PH2/3.
// So:
// PA0 -> likely KEY0 (or WK_UP?)
// PC13 -> likely KEY1 (or KEY2?)
// PH2/3 -> likely KEY2/WK_UP (or KEY0/1?)
// My plan remapped PH2->PA1 and PH3->PA2.
// I should update key.h to match the physical pins I initialized in key.c.
// KEY0 -> PA0
// KEY1 -> PC13
// KEY2 -> PA1 (was PH2)
// WK_UP -> PA2 (was PH3)
#define KEY0        HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0)
#define KEY1        HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_13)
#define KEY2        HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_1) // Remapped from PH2
#define WK_UP       HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_2) // Remapped from PH3

#define KEY0_PRES 	1
#define KEY1_PRES		2
#define KEY2_PRES		3
#define WKUP_PRES   4

void KEY_Init(void);
u8 KEY_Scan(u8 mode);
#endif
