#include "led.h"

void User_Entry(void)
{
    LED_Init();

    while (1)
    {
        LED_On(LED_1);
        LED_On(LED_2);

        HAL_Delay(500);
        LED_Off(LED_1);
        LED_Off(LED_2);

        HAL_Delay(500);
    }
}
