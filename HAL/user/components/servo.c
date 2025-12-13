#include "servo.h"
#include "tim.h"
#include "uart_driver.h"
#include "usart.h"
#include "delay_driver.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>

volatile float servo_kp = SERVO_KP_DEFAULT;
volatile float servo_ki = SERVO_KI_DEFAULT;
volatile float servo_kd = SERVO_KD_DEFAULT;
volatile float servo_pid_limit = SERVO_PID_LIMIT_DEFAULT;

Motor_Handle_t myMotor;
PIDController posPID;
volatile Servo_State_t servo;

#define SERVO_MAX_INSTANCES 4
static Servo_Handle_t *servo_instances[SERVO_MAX_INSTANCES];
static uint8_t servo_instance_count = 0;
static uint8_t servo_scheduler_started = 0;

/* External Handles from main.c/tim.c */
extern TIM_HandleTypeDef htim1; // PWM (PA8)
extern TIM_HandleTypeDef htim2; // Encoder (PA0/PA1)
extern TIM_HandleTypeDef htim3; // 1ms Interrupt

/* Debug / Telemetry */
volatile uint32_t debug_counter = 0;
volatile float debug_last_pwm = 0.0f;
volatile uint8_t servo_enabled = 0;
volatile uint8_t servo_error_state = 0; // 0=OK, 1=Stall/Runaway Detected

#define CMD_BUFFER_SIZE 64
static char cmd_buffer[CMD_BUFFER_SIZE];
static uint8_t cmd_index = 0;

static void Servo_BindDefaultPointers(Servo_Handle_t *handle)
{
    if (handle->debug_counter == NULL) {
        handle->debug_counter_storage = 0;
        handle->debug_counter = &handle->debug_counter_storage;
    }
    if (handle->debug_last_pwm == NULL) {
        handle->debug_last_pwm_storage = 0.0f;
        handle->debug_last_pwm = &handle->debug_last_pwm_storage;
    }
    if (handle->enabled == NULL) {
        handle->enabled_storage = 0;
        handle->enabled = &handle->enabled_storage;
    }
    if (handle->error_state == NULL) {
        handle->error_state_storage = 0;
        handle->error_state = &handle->error_state_storage;
    }
}

static HAL_StatusTypeDef Servo_RegisterInstance(Servo_Handle_t *handle)
{
    if (handle == NULL) return HAL_ERROR;

    __disable_irq();
    for (uint8_t i = 0; i < servo_instance_count; i++) {
        if (servo_instances[i] == handle) {
            __enable_irq();
            return HAL_OK;
        }
    }
    if (servo_instance_count >= SERVO_MAX_INSTANCES) {
        __enable_irq();
        return HAL_ERROR;
    }
    servo_instances[servo_instance_count++] = handle;
    __enable_irq();

    return HAL_OK;
}

static float Servo_ComputeTrajectory_Instance(Servo_Handle_t *handle)
{
    volatile Servo_State_t *s = handle->state;
    float error = (float)s->target_pos - s->setpoint_pos;

    if (fabsf(error) < 0.5f) {
        s->setpoint_vel = 0.0f;
        return (float)s->target_pos;
    }

    float direction = (error > 0.0f) ? 1.0f : -1.0f;
    float abs_error = fabsf(error);

    float max_vel_at_dist = sqrtf(2.0f * SERVO_DECELERATION * abs_error);

    float target_vel = max_vel_at_dist;
    if (target_vel > SERVO_MAX_VELOCITY) target_vel = SERVO_MAX_VELOCITY;
    target_vel *= direction;

    float vel_diff = target_vel - s->setpoint_vel;
    float max_vel_change = SERVO_ACCELERATION * CONTROL_DT;

    if (vel_diff > max_vel_change) {
        s->setpoint_vel += max_vel_change;
    } else if (vel_diff < -max_vel_change) {
        s->setpoint_vel -= max_vel_change;
    } else {
        s->setpoint_vel = target_vel;
    }

    return s->setpoint_pos + (s->setpoint_vel * CONTROL_DT);
}

static void Check_Runaway_Condition_Instance(Servo_Handle_t *handle, float pid_output, int32_t current_pos)
{
    if (fabsf(pid_output) > 80.0f) {
        if (handle->high_load_duration == 0) {
            handle->load_start_pos = current_pos;
        }

        handle->high_load_duration++;

        if (handle->high_load_duration > 500) {
            int32_t moved = abs(current_pos - handle->load_start_pos);

            if (moved < 10) {
                *(handle->enabled) = 0;
                *(handle->error_state) = 1;
                Motor_Stop(handle->motor);
            }
        }
    } else {
        handle->high_load_duration = 0;
    }
}

HAL_StatusTypeDef Servo_InitInstance(Servo_Handle_t *handle,
                                      Motor_Handle_t *motor,
                                      PIDController *pid,
                                      volatile Servo_State_t *state,
                                      const Servo_Config_t *cfg)
{
    if (handle == NULL || motor == NULL || pid == NULL || state == NULL || cfg == NULL) return HAL_ERROR;

    volatile uint32_t *dbg_counter = handle->debug_counter;
    volatile float *dbg_last_pwm = handle->debug_last_pwm;
    volatile uint8_t *enabled = handle->enabled;
    volatile uint8_t *error_state = handle->error_state;

    memset(handle, 0, sizeof(*handle));
    handle->motor = motor;
    handle->pid = pid;
    handle->state = state;

    handle->debug_counter = dbg_counter;
    handle->debug_last_pwm = dbg_last_pwm;
    handle->enabled = enabled;
    handle->error_state = error_state;

    Servo_BindDefaultPointers(handle);

    handle->kp = cfg->kp;
    handle->ki = cfg->ki;
    handle->kd = cfg->kd;
    handle->pid_limit = cfg->pid_limit;

    memset((void*)handle->state, 0, sizeof(Servo_State_t));

    motor->htim = cfg->pwm_htim;
    motor->channel = cfg->pwm_channel;
    motor->en_port = cfg->en_port;
    motor->en_pin = cfg->en_pin;
    motor->dir_port = cfg->dir_port;
    motor->dir_pin = cfg->dir_pin;
    motor->pwm_period = cfg->pwm_period;
    motor->htim_enc = cfg->enc_htim;

    Motor_Init(motor);
    Motor_Encoder_Init(motor);
    Motor_ResetEncoderCount(motor, 0);
    Motor_SetSpeed(motor, 0);

    PID_Init(pid, cfg->kp, cfg->ki, cfg->kd, cfg->pid_limit, cfg->pid_ramp);
    PID_Reset(pid);

    int32_t initial_pos = Motor_GetEncoderCount(motor);
    handle->state->target_pos = initial_pos;
    handle->state->setpoint_pos = (float)initial_pos;
    handle->state->actual_pos = initial_pos;
    handle->state->is_at_target = 1;

    *(handle->enabled) = 1;
    *(handle->error_state) = 0;

    HAL_StatusTypeDef st = Servo_RegisterInstance(handle);
    if (st != HAL_OK) return st;

    if (!servo_scheduler_started) {
        HAL_TIM_Base_Start_IT(&htim3);
        servo_scheduler_started = 1;
    }

    HAL_Delay(10);

    if (cfg->auto_start) {
        Motor_Start(motor);
    }

    return HAL_OK;
}

static Servo_Handle_t *Servo_GetDefaultHandle(void)
{
    if (servo_instance_count > 0 && servo_instances[0] != NULL) {
        return servo_instances[0];
    }

    return NULL;
}

void Servo_UpdateInstance_1kHz(Servo_Handle_t *handle) {
    if (handle == NULL || handle->motor == NULL || handle->pid == NULL || handle->state == NULL) return;

    if (*(handle->error_state)) {
        Motor_SetSpeed(handle->motor, 0);
        return;
    }

    if (!(*(handle->enabled))) {
        Motor_SetSpeed(handle->motor, 0);
        return;
    }

    volatile Servo_State_t *s = handle->state;

    s->actual_pos = Motor_GetEncoderCount(handle->motor);
    s->setpoint_pos = Servo_ComputeTrajectory_Instance(handle);

    float pos_error = s->setpoint_pos - (float)s->actual_pos;

    if (fabsf(pos_error) < SERVO_POS_TOLERANCE && fabsf(s->setpoint_vel) < 1.0f) {
        s->is_at_target = 1;
        Motor_SetSpeed(handle->motor, 0);
        return;
    }

    s->is_at_target = 0;

    if (fabsf(pos_error) < 1.0f) pos_error = 0.0f;

    float pid_out = PID_Compute(handle->pid, pos_error, CONTROL_DT);
    *(handle->debug_last_pwm) = pid_out;

    Check_Runaway_Condition_Instance(handle, pid_out, s->actual_pos);
    if (*(handle->error_state)) return;

    uint8_t pwm_val = 0;
    float abs_out = fabsf(pid_out);

    if (abs_out > 100.0f) abs_out = 100.0f;
    pwm_val = (uint8_t)abs_out;

    if (pwm_val < 2) pwm_val = 0;

    if (pid_out >= 0) {
        Motor_SetDirection(handle->motor, 1);
    } else {
        Motor_SetDirection(handle->motor, 0);
    }

    Motor_SetSpeed(handle->motor, pwm_val);
}

void Servo_SetTargetInstance(Servo_Handle_t *handle, int32_t position)
{
    if (handle == NULL || handle->state == NULL) return;
    handle->state->target_pos = position;
    handle->state->is_at_target = 0;
}

uint8_t Servo_IsAtTargetInstance(Servo_Handle_t *handle)
{
    if (handle == NULL || handle->state == NULL) return 0;
    return handle->state->is_at_target;
}

/**
 * @brief  Convert degrees to encoder pulses
 */
static inline int32_t Degrees_To_Pulses(int32_t degrees) {
    return degrees;  // 360 pulses/rev, so 1 degree = 1 pulse
}

/**
 * @brief  Convert encoder pulses to degrees
 */
static inline float Pulses_To_Degrees(int32_t pulses) {
    return (float)pulses;  // Direct 1:1 conversion
}

/**
 * @brief  Parse and execute UART commands
 */
void Process_Command(char* cmd)
{
    if (cmd[0] == '\0') return;

    Servo_Handle_t *h = Servo_GetDefaultHandle();
    if (h == NULL || h->motor == NULL || h->pid == NULL || h->state == NULL) {
        UART_Debug_Printf("[ERROR] Servo instance not initialized.\r\n> ");
        return;
    }

    volatile Servo_State_t *s = h->state;

    char cmd_type = cmd[0];
    int32_t value = 0;

    switch (cmd_type) {
        case 'G':
        case 'g':
            value = atoi(&cmd[1]);
            UART_Debug_Printf("[CMD] Go to %ld degrees (%ld pulses)\r\n", value, Degrees_To_Pulses(value));
            Servo_SetTargetInstance(h, Degrees_To_Pulses(value));
            break;

        case 'R':
        case 'r':
            UART_Debug_Printf("[DEBUG] Raw cmd: '%s', Parsing substring: '%s'\r\n", cmd, &cmd[1]);
            value = atoi(&cmd[1]);
            UART_Debug_Printf("[DEBUG] Parsed Value: %ld (0x%08lX)\r\n", value, value);
        {
            int32_t new_target = s->target_pos + Degrees_To_Pulses(value);
            UART_Debug_Printf("[CMD] Rotate %ld degrees (new target: %ld pulses)\r\n", value, new_target);
            Servo_SetTargetInstance(h, new_target);
        }
            break;

        case 'Z':
        case 'z':
            UART_Debug_Printf("[CMD] Zeroing encoder position\r\n");
            Motor_ResetEncoderCount(h->motor, 0);
            s->target_pos = 0;
            s->setpoint_pos = 0.0f;
            s->setpoint_vel = 0.0f;
            s->actual_pos = 0;
            s->is_at_target = 1;
            PID_Reset(h->pid);
            break;

        case 'S':
        case 's':
        {
            int32_t current = Motor_GetEncoderCount(h->motor);
            UART_Debug_Printf("[CMD] Stop and hold at %ld pulses (%.1f degrees)\r\n",
                current, Pulses_To_Degrees(current));
            Servo_SetTargetInstance(h, current);
        }
            break;

        case 'H':
        case 'h':
        case '?':
            UART_Debug_Printf("\r\n=== Motor Position Control ===\r\n");
            UART_Debug_Printf("Encoder: 360 pulses/rev (1 degree = 1 pulse)\r\n");
            UART_Debug_Printf("Commands:\r\n");
            UART_Debug_Printf("  G<deg>  - Go to absolute position (e.g., G90, G-45, G360)\r\n");
            UART_Debug_Printf("  R<deg>  - Rotate relative (e.g., R90, R-180)\r\n");
            UART_Debug_Printf("  Z       - Zero encoder (set current position as 0)\r\n");
            UART_Debug_Printf("  S       - Stop and hold current position\r\n");
            UART_Debug_Printf("  P       - Print current position\r\n");
            UART_Debug_Printf("  E       - Test Encoder Readings (Continuous)\r\n");
            UART_Debug_Printf("  L<val>  - Set Max PWM Limit (0-100%%)\r\n");
            UART_Debug_Printf("  H/?     - Show this help\r\n");
            UART_Debug_Printf("Examples: G90 (go to 90°), R360 (rotate one full turn), L80 (limit power to 80%%)\r\n\r\n");
            UART_Debug_Printf("> ");
            break;

        case 'P':
        case 'p':
            UART_Debug_Printf("[INFO] Position: %ld pulses = %.1f degrees\r\n",
                s->actual_pos, Pulses_To_Degrees(s->actual_pos));
            UART_Debug_Printf("[INFO] Target: %ld pulses = %.1f degrees\r\n",
                s->target_pos, Pulses_To_Degrees(s->target_pos));
            UART_Debug_Printf("[INFO] At target: %s\r\n", s->is_at_target ? "Yes" : "No");
            break;

        case 'E':
        case 'e':
            Test_Encoder_Readings();
            break;

        case 'L':
        case 'l':
            value = atoi(&cmd[1]);
            if (value < 0) value = 0;
            if (value > 100) value = 100;

            servo_pid_limit = (float)value;
            h->pid_limit = servo_pid_limit;
            h->pid->limit = servo_pid_limit;

            UART_Debug_Printf("[CMD] Set PWM Limit to %ld%%\r\n", value);
            break;

        default:
            UART_Debug_Printf("[ERROR] Unknown command: %c. Type 'H' for help.\r\n", cmd_type);
            UART_Debug_Printf("> ");
            break;
    }
}

/**
 * @brief  Non-blocking UART receive and command processing
 */
void Poll_UART_Commands(void)
{
    uint8_t rx_byte;

    while (UART_Read(UART_DEBUG_CHANNEL, &rx_byte)) {
        UART_Send(UART_DEBUG_CHANNEL, &rx_byte, 1);

        if (rx_byte == '\r' || rx_byte == '\n') {
            if (cmd_index > 0) {
                cmd_buffer[cmd_index] = '\0';
                UART_Debug_Printf("\r\n");
                Process_Command(cmd_buffer);
                cmd_index = 0;
            }
        } else if (rx_byte == 0x7F || rx_byte == 0x08) {
            if (cmd_index > 0) {
                cmd_index--;
                UART_Debug_Printf("\b \b");
            }
        } else if (cmd_index < CMD_BUFFER_SIZE - 1) {
            cmd_buffer[cmd_index++] = (char)rx_byte;
        }
    }
}

/**
 * @brief  Interrupt Callback - TIM3 Period Elapsed
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    Delay_TIM_PeriodElapsedCallback(htim);
    if (htim->Instance == TIM3) {
        for (uint8_t i = 0; i < servo_instance_count; i++) {
            Servo_Handle_t *h = servo_instances[i];
            if (h == NULL) continue;
            Servo_UpdateInstance_1kHz(h);
            if (h->debug_counter != NULL) {
                (*(h->debug_counter))++;
            }
        }
    }
}

/**
 * @brief Continuous Encoder Test Mode
 *        Loops and prints encoder values until 'q' is pressed.
 */
void Test_Encoder_Readings(void)
{
    UART_Debug_Printf("\r\n=== Encoder Test Mode ===\r\n");
    UART_Debug_Printf("Controls:\r\n");
    UART_Debug_Printf("  'w' : Increase PWM (+10%%)\r\n");
    UART_Debug_Printf("  's' : Decrease PWM (-10%%)\r\n");
    UART_Debug_Printf("  ' ' : Stop (PWM 0)\r\n");
    UART_Debug_Printf("  'q' : Exit\r\n\r\n");

    int8_t current_pwm = 0;

    // Safety: Disable control loop
    servo_enabled = 0;
    HAL_Delay(10);

    // Explicitly enable motor hardware (driver EN pin)
    Motor_Start(&myMotor);

    uint8_t rx_byte = 0;

    // Flush any pending data
    while (UART_Read(UART_DEBUG_CHANNEL, &rx_byte));

    while (1) {
        int32_t count = Motor_GetEncoderCount(&myMotor);
        int32_t deg_int = count;

        UART_Debug_Printf("\r[ENC] PWM: %3d%% | Count: %6ld | Deg: %ld   ",
            current_pwm, count, deg_int);

        if (UART_Read(UART_DEBUG_CHANNEL, &rx_byte)) {
            if (rx_byte == 'q' || rx_byte == 'Q') {
                Motor_Stop(&myMotor);
                UART_Debug_Printf("\r\nExiting Test Mode.\r\n> ");
                break;
            }

            if (rx_byte == 'w' || rx_byte == 'W') {
                current_pwm += 10;
                if (current_pwm > 100) current_pwm = 100;

                if (current_pwm >= 0) {
                    Motor_SetDirection(&myMotor, 1);
                    Motor_SetSpeed(&myMotor, (uint8_t)current_pwm);
                } else {
                    Motor_SetDirection(&myMotor, 0);
                    Motor_SetSpeed(&myMotor, (uint8_t)(-current_pwm));
                }
            }

            if (rx_byte == 's' || rx_byte == 'S') {
                current_pwm -= 10;
                if (current_pwm < -100) current_pwm = -100;

                if (current_pwm >= 0) {
                    Motor_SetDirection(&myMotor, 1);
                    Motor_SetSpeed(&myMotor, (uint8_t)current_pwm);
                } else {
                    Motor_SetDirection(&myMotor, 0);
                    Motor_SetSpeed(&myMotor, (uint8_t)(-current_pwm));
                }
            }

            if (rx_byte == ' ') {
                current_pwm = 0;
                Motor_Stop(&myMotor);
            }
        }

        HAL_Delay(10);
    }

    // Resume control loop
    int32_t current_pos = Motor_GetEncoderCount(&myMotor);
    servo.target_pos = current_pos;
    servo.setpoint_pos = (float)current_pos;
    servo.setpoint_vel = 0.0f;
    servo.is_at_target = 1;
    PID_Reset(&posPID);
    servo_enabled = 1;
}