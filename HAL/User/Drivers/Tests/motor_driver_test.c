#include "motor_driver.h"
#include "tim.h"
#include "uart_driver.h"
#include "pid.h"
#include <stdio.h>
#include <string.h>

/* 定义电机对象 */
Motor_Handle_t myMotor;

void User_Entry(void)
{
    // 初始化 UART 用于调试打印
    UART_Init();
    UART_Debug_Printf("Motor PID Control Start (Max 95%%)\r\n");

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
    // Kp=0.08 (大幅降低以消除震荡), Ki=0.0 (暂时关闭积分), Kd=0.0, DeadZone=10.0
    PID_Init(&posPID, 0.5f, 0.0f, 0.0f, 95.0f, 100.0f, 10.0f);

    int32_t target_pos = 0; // 目标位置 (脉冲数)
    int32_t current_pos = 0;
    float control_output = 0.0f;
    uint32_t last_time = HAL_GetTick();
    uint32_t print_time = 0;

    // 测试序列
    uint32_t target_change_time = 0;
    int step = 0;

    while (1) {
        uint32_t now = HAL_GetTick();
        float dt = (now - last_time) / 1000.0f; // 转换为秒

        // 简单的任务调度：10ms 控制周期 (100Hz)
        if (now - last_time >= 10) {
            last_time = now;

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

        // 打印调试信息 (200ms 一次)
        if (now - print_time >= 200) {
            print_time = now;
            
            extern TIM_HandleTypeDef htim2;
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

        // 改变目标位置逻辑
        if (now - target_change_time > 3000) { // 每3秒变一次
            target_change_time = now;
            step++;
            if (step > 3) step = 0;

            switch (step) {
                case 0: target_pos = 0; break;
                case 1: target_pos = 360; break; // 转一圈
                case 2: target_pos = 720; break; // 转两圈
                case 3: target_pos = -360; break; // 反向一圈
            }
            UART_Debug_Printf("Target Changed to: %d\r\n", target_pos);
        }
    }
}