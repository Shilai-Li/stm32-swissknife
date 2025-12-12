#include "delay_driver.h"

void User_Entry(void)
{
    while (1)
    {
        HAL_Delay(1000);
    }
}