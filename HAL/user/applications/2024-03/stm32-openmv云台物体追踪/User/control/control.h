#ifndef __CONTROL_H
#define __CONTROL_H

#include "main.h"
#include "stm32f1xx_hal.h"

#define   para     3

void Control_X (unsigned int x);
void Control_Y (unsigned int y);

unsigned int Variable_X(unsigned char flag);
unsigned int Variable_Y(unsigned char flag);


void Delay(unsigned int x);
#endif
