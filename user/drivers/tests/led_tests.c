#include "io/led.h"

#include "main.h"

void user_main(void)
{
    LED_Init();

    while (1)
    {
        LED_On(LED_1);
        HAL_Delay(500);

        LED_Off(LED_1);
        HAL_Delay(500);
    }
}
