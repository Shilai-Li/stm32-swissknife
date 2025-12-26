#include "io/led.h"

#include "main.h"

void user_main(void)
{
    LED_Register(LED_1, GPIOB, GPIO_PIN_2, LED_ACTIVE_LOW);

    while (1)
    {
        // 2. Test Toggle
        LED_Toggle(LED_1);
        HAL_Delay(200);
        LED_Toggle(LED_1);
        HAL_Delay(200);
        
        // 3. Test On/Off explicitly
        LED_On(LED_1);
        HAL_Delay(1000);
        
        LED_Off(LED_1);
        HAL_Delay(1000);
    }
}
