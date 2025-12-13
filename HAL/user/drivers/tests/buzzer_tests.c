#include "buzzer.h"

void User_Entry(void)
{
    BUZZER_Init();

    while (1)
    {
        for (int i = 0; i < BUZZER_n; i++)
        {
            BUZZER_On((Buzzer_TypeDef)i);
            HAL_Delay(500);
            BUZZER_Off((Buzzer_TypeDef)i);
            HAL_Delay(500);
        }
    }
}