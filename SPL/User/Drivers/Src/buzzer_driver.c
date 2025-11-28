#include "buzzer_driver.h"

#ifdef USE_STDPERIPH_DRIVER

#include "delay_driver.h"

/* ============================================================
 * Static Buzzer Table (Edit according to your board)
 * ============================================================ */
static BUZZER buzzers[BUZZER_MAX_COUNT] = {
    {GPIOC, GPIO_Pin_13, false, BUZZER_ACTIVE_LOW},   /* Buzzer 1 */
};


/* ============================================================
 * Private Helper Functions
 * ============================================================ */

/**
 * @brief Enable GPIO clock based on port
 */
static void BUZZER_EnableClock(GPIO_TypeDef *port)
{
    if (port == GPIOA) RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    else if (port == GPIOB) RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    else if (port == GPIOC) RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    else if (port == GPIOD) RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
    else if (port == GPIOE) RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
}


/* ============================================================
 * Public Functions
 * ============================================================ */

/**
 * @brief Initialize all buzzer GPIO pins
 */
void BUZZER_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    for (uint8_t i = 0; i < BUZZER_MAX_COUNT; i++) {

        /* Enable clock */
        BUZZER_EnableClock(buzzers[i].port);

        /* Configure GPIO */
        GPIO_InitStructure.GPIO_Pin = buzzers[i].pin;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
        GPIO_Init(buzzers[i].port, &GPIO_InitStructure);

        /* Default OFF */
        BUZZER_Off(i);
    }
}


/**
 * @brief Turn ON a specific buzzer
 */
void BUZZER_On(uint8_t buzzer_id)
{
    if (buzzer_id < BUZZER_MAX_COUNT) {

        if (buzzers[buzzer_id].level == BUZZER_ACTIVE_HIGH)
            GPIO_SetBits(buzzers[buzzer_id].port, buzzers[buzzer_id].pin);
        else
            GPIO_ResetBits(buzzers[buzzer_id].port, buzzers[buzzer_id].pin);

        buzzers[buzzer_id].is_on = true;
    }
}


/**
 * @brief Turn OFF a specific buzzer
 */
void BUZZER_Off(uint8_t buzzer_id)
{
    if (buzzer_id < BUZZER_MAX_COUNT) {

        if (buzzers[buzzer_id].level == BUZZER_ACTIVE_HIGH)
            GPIO_ResetBits(buzzers[buzzer_id].port, buzzers[buzzer_id].pin);
        else
            GPIO_SetBits(buzzers[buzzer_id].port, buzzers[buzzer_id].pin);

        buzzers[buzzer_id].is_on = false;
    }
}


/**
 * @brief Toggle buzzer state
 */
void BUZZER_Toggle(uint8_t buzzer_id)
{
    if (buzzer_id < BUZZER_MAX_COUNT) {

        if (buzzers[buzzer_id].is_on)
            BUZZER_Off(buzzer_id);
        else
            BUZZER_On(buzzer_id);
    }
}


/**
 * @brief Set buzzer state (true = ON / false = OFF)
 */
void BUZZER_SetState(uint8_t buzzer_id, bool state)
{
    if (state)
        BUZZER_On(buzzer_id);
    else
        BUZZER_Off(buzzer_id);
}


/**
 * @brief Simple test function (blocking)
 */
void BUZZER_Test(void)
{
    BUZZER_Init();
		Delay_Init();

    while (1) {

        for (uint8_t i = 0; i < BUZZER_MAX_COUNT; i++) {
            BUZZER_On(i);
            Delay_ms(1000);
            BUZZER_Off(i);
					  Delay_ms(1000);
        }
    }
}

#endif