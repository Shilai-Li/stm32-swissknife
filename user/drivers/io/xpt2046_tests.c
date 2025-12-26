/**
 * @file xpt2046_tests.c
 * @brief Test XPT2046 Touch Screen (Dual Mode SPI)
 */

#include "main.h"
#include "xpt2046.h"
#include "soft_spi.h"
#include "usart.h"
#include "uart.h"
#include "usb_cdc.h"
#include <stdio.h>

#define CH_DEBUG 2

//#define USE_HARDWARE_SPI 0
#define USE_HARDWARE_SPI 1

#if USE_HARDWARE_SPI
extern SPI_HandleTypeDef hspi1; 
#endif

// XPT2046_HandleTypeDef touch; // Define later

XPT2046_HandleTypeDef touch;
Soft_SPI_HandleTypeDef soft_spi;

// Wrapper for Hardware SPI to match interface
uint8_t HAL_SPI_Wrapper(void *handle, uint8_t *tx, uint8_t *rx, uint16_t size, uint32_t timeout) {
    // HAL returns HAL_StatusTypeDef (0=OK)
#if USE_HARDWARE_SPI
    return (uint8_t)HAL_SPI_TransmitReceive((SPI_HandleTypeDef*)handle, tx, rx, size, timeout);
#else
    return 1; // Error
#endif
}

// Wrapper for Software SPI to match interface
uint8_t Soft_SPI_Wrapper(void *handle, uint8_t *tx, uint8_t *rx, uint16_t size, uint32_t timeout) {
    // Soft SPI returns uint8_t
    return Soft_SPI_TransmitReceive((Soft_SPI_HandleTypeDef*)handle, tx, rx, size, timeout);
}

void app_main(void) {
    UART_Register(CH_DEBUG, &huart2);
    // UART_Init();
    
    UART_SendString(CH_DEBUG, "\r\n=== XPT2046 Touch Test (Decoupled) ===\r\n");

#if USE_HARDWARE_SPI
    UART_SendString(CH_DEBUG, "Mode: Hardware SPI\r\n");
    // Init Hardware SPI
    XPT2046_Init(&touch, &hspi1, HAL_SPI_Wrapper, 
                 GPIOA, GPIO_PIN_4, GPIOA, GPIO_PIN_1);

#else
    UART_SendString(CH_DEBUG, "Mode: Software SPI\r\n");
    // Init Software SPI
    Soft_SPI_Init(&soft_spi, 
                  GPIOA, GPIO_PIN_5, // SCK
                  GPIOA, GPIO_PIN_7, // MOSI
                  GPIOA, GPIO_PIN_6, // MISO
                  SOFT_SPI_MODE_0);  
                  
    // Init Touch with Soft SPI Wrapper
    XPT2046_Init(&touch, &soft_spi, Soft_SPI_Wrapper, 
                 GPIOA, GPIO_PIN_4, GPIOA, GPIO_PIN_1);
#endif
    
    // ... Rest same ...

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
