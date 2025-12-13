#include "control.h"

/* External Handles */
extern TIM_HandleTypeDef htim2;

unsigned int pwm_x = 100; // Initial Position
unsigned int pwm_y = 100;

void Control_X (unsigned int x)
{
    // TIM_SetCompare2(TIM2,50+(int)((float)x*1.115));
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 50 + (int)((float)x * 1.115));
}

void Control_Y (unsigned int y)
{
    // TIM_SetCompare1(TIM2,50+(int)((float)y*1.115));
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 50 + (int)((float)y * 1.115));
}

unsigned int Variable_X(unsigned char flag)
{
    if (flag == 1)
    {
        pwm_x -= para;
        if (pwm_x < 5)
            pwm_x = 5;
    }
    else
    {
        pwm_x += para;
        if (pwm_x > 180)
            pwm_x = 180;
    }
    return pwm_x;
}

unsigned int Variable_Y(unsigned char flag)
{
    if (flag == 1)
    {
        pwm_y -= para;
        if (pwm_y < 31)
            pwm_y = 31;
    }
    else
    {
        pwm_y += para;
        if (pwm_y > 130)
            pwm_y = 130;
    }
    return pwm_y;
}

void Delay(unsigned int x)
{
    HAL_Delay(x);
}
