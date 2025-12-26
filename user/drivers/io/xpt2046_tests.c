/**
 * @file xpt2046_tests.c
 * @brief Test XPT2046 Touch Screen (Dual Mode SPI)
 */

#include "io/xpt2046.h"
#include "interface/soft_spi.h"
#include "uart.h"
#include "usb_cdc.h"
#include <stdio.h>

extern SPI_HandleTypeDef hspi1; // Adjust manually if testing Hard SPI

#define CH_DEBUG 2

// Choose Mode for Test
#define USE_HARDWARE_SPI 0

XPT2046_HandleTypeDef touch;
Soft_SPI_HandleTypeDef soft_spi;

void user_main(void) {
    UART_Register(CH_DEBUG, &huart2);
    // UART_Init();
    
    UART_SendString(CH_DEBUG, "\r\n=== XPT2046 Touch Test ===\r\n");

#if USE_HARDWARE_SPI
    UART_SendString(CH_DEBUG, "Mode: Hardware SPI\r\n");
    // Init Hardware SPI (Assume hspi1 is configured in CubeMX)
    // CS: PA4, IRQ: PA1 (Example)
    XPT2046_Init(&touch, &hspi1, GPIOA, GPIO_PIN_4, GPIOA, GPIO_PIN_1);

#else
    UART_SendString(CH_DEBUG, "Mode: Software SPI\r\n");
    // Init Software SPI
    // SCK: PA5, MOSI: PA7, MISO: PA6
    // Note: Soft_SPI_Init doesn't need a handle in struct v1, it initializes pins.
    // Let's check Soft_SPI_Init signature: 
    // void Soft_SPI_Init(Soft_SPI_HandleTypeDef *hspi, sck_port, sck_pin, mosi_port, mosi_pin, miso_port, miso_pin, mode);
    
    Soft_SPI_Init(&soft_spi, 
                  GPIOA, GPIO_PIN_5, // SCK
                  GPIOA, GPIO_PIN_7, // MOSI
                  GPIOA, GPIO_PIN_6, // MISO
                  SOFT_SPI_MODE_0);  
                  
    // Init Touch with Soft SPI
    // CS: PA4, IRQ: PA1
    XPT2046_Init_Soft(&touch, &soft_spi, GPIOA, GPIO_PIN_4, GPIOA, GPIO_PIN_1);
#endif

    // Calibration (Optional, use defaults)
    // XPT2046_SetCalibration(&touch, 320, 240, 200, 3900, 200, 3900);

    UART_SendString(CH_DEBUG, "Touch the screen...\r\n");

    while(1) {
        if (XPT2046_IsTouched(&touch)) {
            uint16_t x, y;
            if (XPT2046_GetCoordinates(&touch, &x, &y)) {
                char buf[64];
                snprintf(buf, 64, "Touch: X=%d, Y=%d\r\n", x, y);
                UART_SendString(CH_DEBUG, buf);
            }
            HAL_Delay(100); // Poll Rate
        }
        HAL_Delay(10);
    }
}
