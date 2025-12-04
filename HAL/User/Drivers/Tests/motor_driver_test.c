#include "motor_driver.h"
#include "tim.h"
#include "uart_driver.h"
#include "pid.h"
#include <stdio.h>
#include <string.h>
#include "usart.h"

/* PID Tuning Globals */
volatile float tune_Kp = 0.5f;   // 默认P参数
volatile float tune_Ki = 0.0f;
volatile float tune_Kd = 0.05f;  // 少量微分减少震荡
volatile int32_t tune_Target = 0;
uint8_t pid_rx_buf[64];

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart->Instance == USART2) {
        // Search for header 0xAA 0xBB
        // Packet: AA BB [Kp 4] [Ki 4] [Kd 4] [Target 4] [Sum 1] = 19 bytes
        for (int i = 0; i <= (int)Size - 19; i++) {
            if (pid_rx_buf[i] == 0xAA && pid_rx_buf[i+1] == 0xBB) {
                uint8_t *data = &pid_rx_buf[i+2];
                uint8_t checksum = 0;
                for (int j = 0; j < 16; j++) checksum += data[j];
                
                if (checksum == pid_rx_buf[i+18]) {
                    memcpy((void*)&tune_Kp, &data[0], 4);
                    memcpy((void*)&tune_Ki, &data[4], 4);
                    memcpy((void*)&tune_Kd, &data[8], 4);
                    memcpy((void*)&tune_Target, &data[12], 4);
                }
            }
        }
        HAL_UARTEx_ReceiveToIdle_DMA(huart, pid_rx_buf, sizeof(pid_rx_buf));
    }
}

/* 定义电机对象 */
Motor_Handle_t myMotor;

void User_Entry(void)
{
    // 初始化 UART 用于调试打印
    UART_Init();
    
    // Switch UART2 to DMA Idle Receive for Tuning
    extern UART_HandleTypeDef huart2;
    HAL_UART_AbortReceive(&huart2);
    HAL_UARTEx_ReceiveToIdle_DMA(&huart2, pid_rx_buf, sizeof(pid_rx_buf));

    UART_Debug_Printf("=== Step Response Test Mode ===\r\n");
    UART_Debug_Printf("Target: 0 <-> 500 (every 1 second)\r\n");

    // 强制开启 GPIO 时钟
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /* 填充电机配置 */
    myMotor.htim = &htim1;
    myMotor.channel = TIM_CHANNEL_1;
    myMotor.en_port = GPIOC;
    myMotor.en_pin = GPIO_PIN_15;
    myMotor.dir_port = GPIOB;
    myMotor.dir_pin = GPIO_PIN_13;
    myMotor.pwm_period = 99;

    // 编码器配置 (TIM2)
    extern TIM_HandleTypeDef htim2;
    myMotor.htim_enc = &htim2;

    /* 初始化并启动 */
    Motor_Init(&myMotor);
    Motor_Encoder_Init(&myMotor);
    Motor_ResetEncoderCount(&myMotor, 0); // 确保上电时为0
    Motor_Start(&myMotor);

    // PID 参数配置
    PID_Controller_t posPID;
    PID_Init(&posPID, tune_Kp, tune_Ki, tune_Kd, 95.0f, 100.0f, 10.0f);

    int32_t target_pos = 0; // 目标位置 (脉冲数)
    int32_t current_pos = 0;
    float control_output = 0.0f;
    uint32_t last_time = HAL_GetTick();
    uint32_t print_time = 0;

    // 阶跃测试
    uint32_t step_change_time = 0;

    while (1) {
        uint32_t now = HAL_GetTick();
        float dt = (now - last_time) / 1000.0f; // 转换为秒

        // 简单的任务调度：10ms 控制周期 (100Hz)
        if (now - last_time >= 10) {
            last_time = now;

            // Update PID params from tuning globals (可通过Python调参)
            posPID.Kp = tune_Kp;
            posPID.Ki = tune_Ki;
            posPID.Kd = tune_Kd;
            
            // 如果tune_Target不为0，使用Python设置的目标（手动模式）
            if (tune_Target != 0) {
                target_pos = tune_Target;
            }

            // 获取当前位置
            current_pos = Motor_GetEncoderCount(&myMotor);

            // 计算 PID
            control_output = PID_Compute(&posPID, (float)target_pos, (float)current_pos, dt);

            // 应用控制量
            // 方向映射：1=正转，0=反转
            if (control_output >= 0) {
                Motor_SetDirection(&myMotor, 1); // 正转
                Motor_SetSpeed(&myMotor, (uint8_t)control_output);
            } else {
                Motor_SetDirection(&myMotor, 0); // 反转
                Motor_SetSpeed(&myMotor, (uint8_t)(-control_output)); // 取绝对值
            }
        }

        // 打印调试信息 (50ms 一次)
        if (now - print_time >= 50) {
            print_time = now;
            
            uint32_t raw_cnt = __HAL_TIM_GET_COUNTER(&htim2);
            
            char buf[96];
            // 修复负数打印格式
            int val_int = (int)control_output;
            int val_dec = (int)((control_output - val_int) * 100);
            if (val_dec < 0) val_dec = -val_dec;

            snprintf(buf, sizeof(buf), "T:%ld C:%ld RAW:%lu Out:%d.%02d\r\n", 
                target_pos, current_pos, raw_cnt,
                val_int, val_dec);
            UART_Send(UART_CHANNEL_2, (uint8_t*)buf, strlen(buf));
        }

        // 阶跃信号自动测试 (Step Response Test)
        // 每1秒切换：0 -> 500 -> 0 -> 500 ...
        // 只在tune_Target为0时自动切换（自动模式）
        if (tune_Target == 0 && now - step_change_time > 1000) {
            step_change_time = now;
            
            if (target_pos == 0) {
                target_pos = 500;  // 阶跃到500
                UART_Debug_Printf(">>> Step UP: Target = 500\r\n");
            } else {
                target_pos = 0;    // 回到0
                UART_Debug_Printf(">>> Step DOWN: Target = 0\r\n");
            }
        }
    }
}