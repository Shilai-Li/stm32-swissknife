#if 1

#include "motor_driver.h"

#include "tim.h"

/* 定义电机对象 */
Motor_Handle_t myMotor;

void User_Entry(void)
{
    /* 填充电机配置 */
    myMotor.htim = &htim2;
    myMotor.channel = TIM_CHANNEL_1;
    myMotor.en_port = GPIOC;
    myMotor.en_pin = GPIO_PIN_15;
    myMotor.dir_port = GPIOB;
    myMotor.dir_pin = GPIO_PIN_13;
    myMotor.pwm_period = 99;

    // /* 初始化并启动 */
    Motor_Init(&myMotor);
    Motor_Start(&myMotor);

    for (int i = 0; i < 2; i++) {

        // 1. 确保方向是正转 (假设1为正)
        Motor_Stop(&myMotor);
        HAL_Delay(5000); // 停1秒，让惯性消失

        Motor_SetDirection(&myMotor, 1);
        Motor_Start(&myMotor);

        // 2. 正向加速 (0 -> 100)
        for (int i = 0; i <= 100; i++) {
            Motor_SetSpeed(&myMotor, i);
            HAL_Delay(20); // 稍微快一点的加速
        }

        // 3. 正转全速保持一小会儿
        HAL_Delay(1000);


        // ====================
        // 阶段二：反转测试 (这就是你需要的"反转加速")
        // ====================

        // 1. 停车缓冲 (关键保护步骤！)
        Motor_Stop(&myMotor);
        HAL_Delay(5000); // 停1秒，让惯性消失

        // 2. 切换方向为反转 (0)
        Motor_SetDirection(&myMotor, 0);
        Motor_Start(&myMotor);

        // 3. 反向加速 (0 -> 100) ——【这里就是添加的部分】
        // 如果不加这个循环，电机可能猛冲或不动
        for (int i = 0; i <= 100; i++) {
            Motor_SetSpeed(&myMotor, i);
            HAL_Delay(20);
        }

        // 4. 反转全速保持一小会儿
        HAL_Delay(1000);
    }

}

#endif