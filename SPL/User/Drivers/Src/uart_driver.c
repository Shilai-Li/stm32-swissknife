#include "uart_driver.h"

#ifdef USE_STDPERIPH_DRIVER

#include <string.h>
#include <stdarg.h>
#include "delay_driver.h"

/* Ring buffers for each UART channel */
static UART_RingBuf uart_rbuf[UART_CHANNEL_MAX];

/* ============================================================
 * Private UART Handle Mapping (SPL)
 * ============================================================ */

static USART_TypeDef* UART_GetPeripheral(UART_Channel ch)
{
    switch (ch) {
#if USE_UART1
        case UART_CHANNEL_1: return USART1;
#endif
#if USE_UART2
        case UART_CHANNEL_2: return USART2;
#endif
#if USE_UART3
        case UART_CHANNEL_3: return USART3;
#endif
#if USE_UART4
        case UART_CHANNEL_4: return UART4;
#endif
#if USE_UART5
        case UART_CHANNEL_5: return UART5;
#endif
#if USE_UART6
        case UART_CHANNEL_6: return USART6;
#endif
        default: return NULL;
    }
}

/* ============================================================
 * GPIO + Clock Initialization
 * ============================================================ */

static void UART_GPIO_Init(UART_Channel ch)
{
    GPIO_InitTypeDef GPIO_InitStruct;

#if USE_UART1
    if (ch == UART_CHANNEL_1)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

        /* PA9 = TX, PA10 = RX */
        GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIOA, &GPIO_InitStruct);

        GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(GPIOA, &GPIO_InitStruct);

        /* NVIC */
        NVIC_InitTypeDef NVIC_InitStruct;
        NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn;
        NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
        NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
        NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStruct);
    }
#endif

#if USE_UART2
    if (ch == UART_CHANNEL_2)
    {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

        /* PA2 = TX, PA3 = RX */
        GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2;
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIOA, &GPIO_InitStruct);

        GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(GPIOA, &GPIO_InitStruct);

        NVIC_InitTypeDef NVIC_InitStruct;
        NVIC_InitStruct.NVIC_IRQChannel = USART2_IRQn;
        NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
        NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
        NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStruct);
    }
#endif
}

/* ============================================================
 * UART Init
 * ============================================================ */
void UART_InitAll(void)
{
    USART_InitTypeDef USART_InitStruct;

    for (uint8_t ch = 0; ch < UART_CHANNEL_MAX; ch++)
    {
			 UART_Channel channel = (UART_Channel)ch;
			
				#if USE_UART1 || USE_UART2 || USE_UART3
						if (UART_GetPeripheral(channel) == NULL) continue;
				#endif

        UART_GPIO_Init(channel);

        USART_InitStruct.USART_BaudRate = UART_DEFAULT_BAUD;
        USART_InitStruct.USART_WordLength = USART_WordLength_8b;
        USART_InitStruct.USART_StopBits = USART_StopBits_1;
        USART_InitStruct.USART_Parity = USART_Parity_No;
        USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
        USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;

        USART_Init(UART_GetPeripheral(channel), &USART_InitStruct);

        /* Enable RX interrupt */
        USART_ITConfig(UART_GetPeripheral(channel), USART_IT_RXNE, ENABLE);

        /* Enable UART */
        USART_Cmd(UART_GetPeripheral(channel), ENABLE);
    }
}

/* ============================================================
 * Ring Buffer (same as HAL version)
 * ============================================================ */
static void UART_RingBuf_PushFromIRQ(UART_Channel ch, uint8_t byte)
{
    UART_RingBuf *rb = &uart_rbuf[ch];
    uint16_t next = (rb->head + 1) & (UART_RX_BUF_SIZE - 1);
    if (next == rb->tail) return;  /* full */
    rb->buf[rb->head] = byte;
    rb->head = next;
}

static bool UART_RingBuf_Pop(UART_Channel ch, uint8_t *out)
{
    UART_RingBuf *rb = &uart_rbuf[ch];
    if (rb->head == rb->tail) return false;
    *out = rb->buf[rb->tail];
    rb->tail = (rb->tail + 1) & (UART_RX_BUF_SIZE - 1);
    return true;
}

uint16_t UART_Available(UART_Channel ch)
{
    UART_RingBuf *rb = &uart_rbuf[ch];
    if (rb->head >= rb->tail) return rb->head - rb->tail;
    return UART_RX_BUF_SIZE - (rb->tail - rb->head);
}

/* ============================================================
 * Send
 * ============================================================ */
void UART_Send(UART_Channel ch, const uint8_t *data, uint16_t len)
{
    USART_TypeDef *uart = UART_GetPeripheral(ch);
    if (!uart || !data || !len) return;

    for (uint16_t i = 0; i < len; i++) {
        while (USART_GetFlagStatus(uart, USART_FLAG_TXE) == RESET);
        USART_SendData(uart, data[i]);
    }
}

/* String send */
void UART_SendString(UART_Channel ch, const char *str)
{
    UART_Send(ch, (uint8_t *)str, strlen(str));
}

/* ============================================================
 * Read
 * ============================================================ */
bool UART_Read(UART_Channel ch, uint8_t *out)
{
    __disable_irq();
    bool ok = UART_RingBuf_Pop(ch, out);
    __enable_irq();
    return ok;
}

bool UART_Receive(UART_Channel ch, uint8_t *out, uint32_t timeout_ms)
{
    uint32_t start = millis();  /* 你可以实现自己的计时函数 */

    while ((millis() - start) < timeout_ms)
    {
        if (UART_Read(ch, out))
            return true;
    }
    return false;
}

/* ============================================================
 * IRQ Handlers (similar to HAL callback)
 * ============================================================ */

#if USE_UART1
void USART1_IRQHandler(void)
{
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        uint8_t byte = USART_ReceiveData(USART1);
        UART_RingBuf_PushFromIRQ(UART_CHANNEL_1, byte);
    }
}
#endif

#if USE_UART2
void USART2_IRQHandler(void)
{
    if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {
        uint8_t byte = USART_ReceiveData(USART2);
        UART_RingBuf_PushFromIRQ(UART_CHANNEL_2, byte);
    }
}
#endif

void UART_Debug_Printf(const char *fmt, ...)
{
    char buffer[256];
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    if (len > 0)
        UART_Send(UART_DEBUG_CHANNEL, (uint8_t *)buffer, len);
}

void UART_Test(void)
{
    #if UART_DEBUG_ENABLE
        UART_InitAll();
    #endif
		
		Delay_Init();        // Configure TIM2 (1MHz)
    Delay_NVIC_Init();   // Enable TIM2 overflow interrupt

    UART_Debug_Printf("System initialized successfully.\r\n");

    uint8_t rx_data;

    while (1)
    {
        UART_Debug_Printf("Heartbeat: %lu ms\r\n", millis());
        Delay_ms(1000);

        if (UART_Available(UART_CHANNEL_1))
        {
            if (UART_Read(UART_CHANNEL_1, &rx_data))
            {
                UART_Send(UART_CHANNEL_1, &rx_data, 1); // 回显
                UART_Debug_Printf(" [Echoed: %c]\r\n", rx_data);
            }
        }
    }
}

#endif