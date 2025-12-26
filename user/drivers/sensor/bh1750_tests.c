#include "bh1750.h"
#include "soft_i2c.h"
#include "uart.h"
#include <stdio.h>

#define UART_CH 0

static Soft_I2C_HandleTypeDef i2c;
static BH1750_Handle_t light_sensor;
static char msg[128];

void bh1750_error_callback(BH1750_Handle_t *dev) {
    snprintf(msg, sizeof(msg), "[BH1750] I2C Error! Total: %lu\r\n", dev->error_cnt);
    UART_SendString(UART_CH, msg);
}

void bh1750_tests_main(void) {
    UART_SendString(UART_CH, "\r\n===== BH1750 Light Sensor Test =====\r\n");
    
    Soft_I2C_Init(&i2c, GPIOB, GPIO_PIN_6, GPIOB, GPIO_PIN_7);
    
    BH1750_Init(&light_sensor, &i2c, 0);
    BH1750_SetErrorCallback(&light_sensor, bh1750_error_callback);
    
    UART_SendString(UART_CH, "Starting continuous measurement...\r\n");
    BH1750_Start(&light_sensor);
    
    UART_SendString(UART_CH, "Reading every 500ms\r\n\r\n");
    
    while (1) {
        float lux = BH1750_ReadLux(&light_sensor);
        
        if (lux >= 0) {
            const char *level;
            if (lux < 10) level = "Dark";
            else if (lux < 100) level = "Dim";
            else if (lux < 500) level = "Normal";
            else if (lux < 10000) level = "Bright";
            else level = "Very Bright";
            
            snprintf(msg, sizeof(msg), "Light: %.1f lux [%s] | Success: %lu\r\n",
                    lux, level, light_sensor.success_cnt);
        } else {
            snprintf(msg, sizeof(msg), "I2C Read Failed! Errors: %lu\r\n",
                    light_sensor.error_cnt);
        }
        
        UART_SendString(UART_CH, msg);
        HAL_Delay(500);
    }
}
