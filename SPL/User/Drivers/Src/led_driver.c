#include "led_driver.h"

#ifdef USE_STDPERIPH_DRIVER

#include "uart_driver.h"
#include "delay_driver.h"
#include "tim.h"

/* ========== SPL: LED Configuration Array ========== */

/**
 * LED configuration array (SPL F1 format)
 * Edit this array to match your hardware configuration
 * Example:
 *   {GPIOA, GPIO_Pin_5,  RCC_APB2Periph_GPIOA,  false, LED_ACTIVE_HIGH}
 *   {GPIOB, GPIO_Pin_0,  RCC_APB2Periph_GPIOB,  false, LED_ACTIVE_LOW}
 *   {GPIOC, GPIO_Pin_13, RCC_APB2Periph_GPIOC,  false, LED_ACTIVE_LOW}
 */
static LED leds[LED_MAX_COUNT] = {
    {GPIOC, GPIO_Pin_13, false, LED_ACTIVE_LOW},
    {GPIOB, GPIO_Pin_9, false, LED_ACTIVE_HIGH},
};

/* ========== SPL: Helper Functions ========== */

/**
 * @brief Write GPIO state based on LED activation level (SPL)
 * @param led Pointer to LED object
 * @param state Desired state (true=ON, false=OFF)
 */
static void LED_WriteGPIO(LED *led, bool state)
{
    bool level = state;
    
    /* If LED is active low, invert logic */
    if (led->level == LED_ACTIVE_LOW)
        level = !state;
    
    if (level)
        GPIO_SetBits(led->port, led->pin);
    else
        GPIO_ResetBits(led->port, led->pin);
}

/* ========== SPL: Public API Implementation ========== */

static void LED_EnableClock(GPIO_TypeDef *port)
{
    /* Enable GPIO clock according to port */
    if (port == GPIOA)
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    else if (port == GPIOB)
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    else if (port == GPIOC)
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    else if (port == GPIOD)
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
    else if (port == GPIOE)
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
}

/**
 * @brief Initialize all LED pins (SPL)
 */
void LED_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    for (uint8_t i = 0; i < LED_MAX_COUNT; i++) 
    {
        /* Enable clock for this GPIO port */
        LED_EnableClock(leds[i].port);

        /* Configure pin as push-pull output */
        GPIO_InitStruct.GPIO_Pin = leds[i].pin;
        GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_Init(leds[i].port, &GPIO_InitStruct);

        /* Set initial state to OFF */
        LED_WriteGPIO(&leds[i], false);
        leds[i].is_on = false;
    }
}


/**
 * @brief Turn ON specified LED (SPL)
 * @param id LED index
 */
void LED_On(uint8_t id)
{
    if (id >= LED_MAX_COUNT)
        return;
    
    LED *led = &leds[id];
    LED_WriteGPIO(led, true);
    led->is_on = true;
}

/**
 * @brief Turn OFF specified LED (SPL)
 * @param id LED index
 */
void LED_Off(uint8_t id)
{
    if (id >= LED_MAX_COUNT)
        return;
    
    LED *led = &leds[id];
    LED_WriteGPIO(led, false);
    led->is_on = false;
}

/**
 * @brief Toggle LED state (SPL)
 * @param id LED index
 */
void LED_Toggle(uint8_t id)
{
    if (id < LED_MAX_COUNT)
    {
        bool new_state = !leds[id].is_on;
        LED_WriteGPIO(&leds[id], new_state);
        leds[id].is_on = new_state;
    }
}

/**
 * @brief Set LED state directly (SPL)
 * @param id LED index
 * @param state true=ON, false=OFF
 */
void LED_SetState(uint8_t id, bool state)
{
    if (id < LED_MAX_COUNT)
    {
        LED_WriteGPIO(&leds[id], state);
        leds[id].is_on = state;
    }
}

// 亮度0~100，period与初始化一致
void LED_SetBrightness_SPL(uint8_t brightness, uint16_t period)
{
    uint16_t pulse = (brightness * period) / 100; // 占空比计算
    TIM_SetCompare1(TIM3, pulse);
}

/**
 * @brief LED test function (SPL)
 * Demonstrates all LED control functions
 * @note SPL does not have HAL_Delay(), provide your own delay if needed
 */
void LED_Test(void)
{
    LED_Init();
		Delay_Init();
		TIM3_PWM_Init(71, 99);

    while (1)
    {
			#if 1
				LED_SetBrightness_SPL(10, 99);
				
			#endif
			
			#if 0
				/* Test LED_On */
        for (uint8_t i = 0; i < LED_MAX_COUNT; i++) {
            LED_On(i);
						Delay_ms(200);
        }
        Delay_ms(500);

        /* Test LED_Off */
        for (uint8_t i = 0; i < LED_MAX_COUNT; i++) {
            LED_Off(i);
            Delay_ms(200);
        }
        Delay_ms(500);

        /* Test LED_Toggle */
        for (uint8_t i = 0; i < LED_MAX_COUNT; i++) {
            LED_Toggle(i);
            Delay_ms(200);
        }
        Delay_ms(500);

        /* Test LED_SetState(true) */
        for (uint8_t i = 0; i < LED_MAX_COUNT; i++) {
            LED_SetState(i, true);
            Delay_ms(200);
        }
        Delay_ms(500);

        /* Test LED_SetState(false) */
        for (uint8_t i = 0; i < LED_MAX_COUNT; i++) {
            LED_SetState(i, false);
            Delay_ms(200);
        }
        Delay_ms(500);
				
			#endif
        
    }
}

#endif /* USE_HAL_DRIVER or USE_SPL_DRIVER */