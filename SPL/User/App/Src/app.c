#include "app.h"

#include "uart_driver.h"
#include "delay_driver.h"
#include "led_driver.h"


int app_main(void)
{
		
	UART_Test();
	//LED_Test();
	//BUZZER_Test();
	//DHT11_Test();
	//ssd1306_TestAll();
	
    while (1)
    {

    }
}