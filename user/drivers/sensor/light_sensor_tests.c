#include "light_sensor.h"
#include "uart.h"
#include "adc.h"
#include <stdio.h>

#define UART_CH 0

static LightSensor_Handle_t ldr;
static char msg[128];

void light_error_callback(LightSensor_Handle_t *dev) {
    snprintf(msg, sizeof(msg), "[LDR] ADC Error! Total: %lu\r\n", dev->error_cnt);
    UART_SendString(UART_CH, msg);
}

void app_main(void) {
    UART_SendString(UART_CH, "\r\n===== Light Sensor Test =====\r\n");
    
    LightSensor_Config_t config = {
        .hadc = &hadc1,
        .dark_threshold = 800,
        .light_threshold = 1200,
        .inverse_logic = false
    };
    
    LightSensor_Init(&ldr, &config);
    LightSensor_SetErrorCallback(&ldr, light_error_callback);
    
    UART_SendString(UART_CH, "Monitoring ambient light...\r\n");
    UART_SendString(UART_CH, "Commands: [c]alibrate [i]nverse\r\n\r\n");
    
    uint8_t cmd;
    bool led_state = false;
    
    while (1) {
        uint16_t raw = LightSensor_Update(&ldr);
        
        if (UART_Read(UART_CH, &cmd)) {
            switch (cmd) {
                case 'c':
                case 'C':
                    UART_SendString(UART_CH, "Calibration:\r\n");
                    UART_SendString(UART_CH, "1. Cover sensor completely\r\n");
                    HAL_Delay(3000);
                    LightSensor_Update(&ldr);
                    snprintf(msg, sizeof(msg), "  Dark value: %u\r\n", ldr.last_filtered);
                    UART_SendString(UART_CH, msg);
                    
                    UART_SendString(UART_CH, "2. Expose to ambient light\r\n");
                    HAL_Delay(3000);
                    LightSensor_Update(&ldr);
                    snprintf(msg, sizeof(msg), "  Light value: %u\r\n", ldr.last_filtered);
                    UART_SendString(UART_CH, msg);
                    break;
                    
                case 'i':
                case 'I':
                    ldr.config.inverse_logic = !ldr.config.inverse_logic;
                    snprintf(msg, sizeof(msg), "Inverse: %s\r\n",
                            ldr.config.inverse_logic ? "ON" : "OFF");
                    UART_SendString(UART_CH, msg);
                    break;
                    
                default:
                    break;
            }
        }
        
        bool is_dark = LightSensor_IsDark(&ldr);
        uint8_t intensity = LightSensor_GetIntensityPercentage(&ldr);
        
        if (is_dark && !led_state) {
            UART_SendString(UART_CH, ">>> Dark detected - Turn ON LED\r\n");
            led_state = true;
        } else if (!is_dark && led_state) {
            UART_SendString(UART_CH, ">>> Light detected - Turn OFF LED\r\n");
            led_state = false;
        }
        
        snprintf(msg, sizeof(msg), 
                "Raw: %u | Intensity: %u%% | State: %s | Success: %lu\r\n",
                ldr.last_raw, intensity, 
                is_dark ? "DARK" : "LIGHT",
                ldr.success_cnt);
        UART_SendString(UART_CH, msg);
        
        HAL_Delay(300);
    }
}
