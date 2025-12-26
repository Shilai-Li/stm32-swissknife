#include "potentiometer.h"
#include "uart.h"
#include "adc.h"
#include <stdio.h>

#define UART_CH 0

static Pot_Handle_t knob;
static char msg[128];

void knob_error_callback(Pot_Handle_t *dev) {
    snprintf(msg, sizeof(msg), "[POT] ADC Error! Total: %lu\r\n", dev->error_cnt);
    UART_SendString(UART_CH, msg);
}

void app_main(void) {
    UART_SendString(UART_CH, "\r\n===== Potentiometer Test =====\r\n");
    
    Pot_Config_t config = {
        .hadc = &hadc1,
        .deadzone_low = 50,
        .deadzone_high = 4045,
        .inverse = false
    };
    
    Pot_Init(&knob, &config);
    Pot_SetErrorCallback(&knob, knob_error_callback);
    
    UART_SendString(UART_CH, "Reading knob position...\r\n");
    UART_SendString(UART_CH, "Commands: [i]nvert [d]eadzone test [m]ap test\r\n\r\n");
    
    uint8_t cmd;
    
    while (1) {
        uint16_t raw = Pot_Update(&knob);
        
        if (UART_Read(UART_CH, &cmd)) {
            switch (cmd) {
                case 'i':
                case 'I':
                    knob.config.inverse = !knob.config.inverse;
                    snprintf(msg, sizeof(msg), "Inverse: %s\r\n", 
                            knob.config.inverse ? "ON" : "OFF");
                    UART_SendString(UART_CH, msg);
                    break;
                    
                case 'd':
                case 'D':
                    snprintf(msg, sizeof(msg), "Raw: %u | Filtered: %u\r\n",
                            knob.last_raw, knob.last_filtered);
                    UART_SendString(UART_CH, msg);
                    break;
                    
                case 'm':
                case 'M': {
                    int32_t motor_speed = Pot_Map(&knob, -100, 100);
                    int32_t servo_angle = Pot_Map(&knob, 0, 180);
                    int32_t pwm_duty = Pot_Map(&knob, 0, 1000);
                    
                    snprintf(msg, sizeof(msg),
                            "Motor: %ld | Servo: %ld° | PWM: %ld/1000\r\n",
                            motor_speed, servo_angle, pwm_duty);
                    UART_SendString(UART_CH, msg);
                    break;
                }
                    
                default:
                    break;
            }
        }
        
        uint8_t percent = Pot_GetPercent(&knob);
        float ratio = Pot_GetRatio(&knob);
        
        snprintf(msg, sizeof(msg), "Knob: %u%% (%.2f) | Success: %lu\r\n",
                percent, ratio, knob.success_cnt);
        UART_SendString(UART_CH, msg);
        
        HAL_Delay(200);
    }
}
