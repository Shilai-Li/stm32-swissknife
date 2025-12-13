#include "servo.h"
#include "tim.h"
#include "uart_driver.h"
#include "delay_driver.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

/*******************************************************************************
 * INTERNAL STATE & HELPERS
 ******************************************************************************/

#define SERVO_MAX_INSTANCES 4
static Servo_Handle_t *servo_instances[SERVO_MAX_INSTANCES];
static uint8_t servo_instance_count = 0;
static bool scheduler_started = false;

/* External Handles for Timing */
extern TIM_HandleTypeDef htim3; // 1ms Interrupt

/* Command Buffer */
#define CMD_BUFFER_SIZE 64
static char cmd_buffer[CMD_BUFFER_SIZE];
static uint8_t cmd_index = 0;

/**
 * @brief  Register instance internally for ISR and CLI access.
 */
static Servo_Status Register_Instance(Servo_Handle_t *handle)
{
    if (handle == NULL) return SERVO_ERROR;

    __disable_irq();
    for (uint8_t i = 0; i < servo_instance_count; i++) {
        if (servo_instances[i] == handle) {
            __enable_irq();
            return SERVO_OK; // Already registered
        }
    }
    if (servo_instance_count >= SERVO_MAX_INSTANCES) {
        __enable_irq();
        return SERVO_ERROR; // Full
    }
    servo_instances[servo_instance_count++] = handle;
    __enable_irq();

    return SERVO_OK;
}

/**
 * @brief  Get the default instance (first registered) for CLI commands.
 */
static Servo_Handle_t *Get_Default_Handle(void)
{
    if (servo_instance_count > 0) {
        return servo_instances[0];
    }
    return NULL;
}

/*******************************************************************************
 * TRAJECTORY GENERATOR
 ******************************************************************************/

static float Compute_Trajectory(Servo_Handle_t *h)
{
    float error = (float)h->state.target_pos - h->state.setpoint_pos;

    // Deadband
    if (fabsf(error) < 0.5f) {
        h->state.setpoint_vel = 0.0f;
        return (float)h->state.target_pos;
    }

    float direction = (error > 0.0f) ? 1.0f : -1.0f;
    float abs_error = fabsf(error);

    // V^2 = 2*a*s => V = sqrt(2*a*s)
    float max_vel_at_dist = sqrtf(2.0f * SERVO_DECELERATION * abs_error);

    float target_vel = max_vel_at_dist;
    if (target_vel > SERVO_MAX_VELOCITY) target_vel = SERVO_MAX_VELOCITY;
    target_vel *= direction;

    // Ramp velocity
    float vel_diff = target_vel - h->state.setpoint_vel;
    float max_change = SERVO_ACCELERATION * SERVO_DT;

    if (vel_diff > max_change) {
        h->state.setpoint_vel += max_change;
    } else if (vel_diff < -max_change) {
        h->state.setpoint_vel -= max_change;
    } else {
        h->state.setpoint_vel = target_vel;
    }

    return h->state.setpoint_pos + (h->state.setpoint_vel * SERVO_DT);
}

/*******************************************************************************
 * SAFETY CHECKS
 ******************************************************************************/

static void Check_Runaway(Servo_Handle_t *h, float pid_out)
{
    // If output is high but motor isn't moving effectively?
    // Simplified logic check:
    if (fabsf(pid_out) > 80.0f) {
        if (h->overload_counter == 0) {
            h->overload_start_pos = h->state.actual_pos;
        }
        h->overload_counter++;

        // If high load for > 500ms
        if (h->overload_counter > 500) {
            int32_t moved = abs(h->state.actual_pos - h->overload_start_pos);
            if (moved < 10) { // Stall detection
                h->enabled = false;
                h->error = true;
                h->motor_if->stop(h->motor_ctx);
            }
        }
    } else {
        h->overload_counter = 0;
    }
}

/*******************************************************************************
 * PUBLIC API IMPLEMENTATION
 ******************************************************************************/

Servo_Status Servo_Init(Servo_Handle_t *handle,
                             const Servo_MotorInterface_t *motor_if, void *motor_ctx,
                             const Servo_PIDInterface_t *pid_if, void *pid_ctx,
                             const Servo_Config_t *config)
{
    if (!handle || !motor_if || !motor_ctx || !pid_if || !pid_ctx || !config) {
        return SERVO_ERROR;
    }

    // Zero entire struct
    memset(handle, 0, sizeof(Servo_Handle_t));

    // Bind Dependencies
    handle->motor_if = motor_if;
    handle->motor_ctx = motor_ctx;
    handle->pid_if = pid_if;
    handle->pid_ctx = pid_ctx;
    handle->config = *config; // Copy config

    // Hardware Init
    handle->motor_if->init(handle->motor_ctx);
    handle->motor_if->reset_encoder(handle->motor_ctx, 0);
    handle->motor_if->set_speed(handle->motor_ctx, 0);

    // PID Init
    handle->pid_if->init(handle->pid_ctx, 
                         config->kp, config->ki, config->kd, 
                         config->output_limit, config->ramp_rate);
    handle->pid_if->reset(handle->pid_ctx);

    // Initial State
    int32_t current_pos = handle->motor_if->get_encoder(handle->motor_ctx);
    handle->state.target_pos = current_pos;
    handle->state.setpoint_pos = (float)current_pos;
    handle->state.actual_pos = current_pos;
    handle->state.is_at_target = true;

    handle->enabled = true;
    handle->error = false;

    // Register
    if (Register_Instance(handle) != SERVO_OK) {
        return SERVO_ERROR;
    }

    // Start Scheduler if needed
    if (!scheduler_started) {
        HAL_TIM_Base_Start_IT(&htim3);
        scheduler_started = true;
    }
    
    // Auto start
    if (config->auto_start) {
        Servo_Start(handle);
    }
    
    // Allow system to stabilize
    HAL_Delay(10); 
    
    return SERVO_OK;
}

void Servo_Update(Servo_Handle_t *h)
{
    if (!h || !h->enabled || h->error) {
        if (h && h->motor_if && h->motor_ctx) {
            h->motor_if->set_speed(h->motor_ctx, 0);
        }
        return;
    }

    // 1. Read Sensor
    h->state.actual_pos = h->motor_if->get_encoder(h->motor_ctx);

    // 2. Trajectory Generation
    h->state.setpoint_pos = Compute_Trajectory(h);

    // 3. Error Calculation
    float pos_error = h->state.setpoint_pos - (float)h->state.actual_pos;

    // 4. Target Reached Check
    if (fabsf(pos_error) < SERVO_POS_TOLERANCE && fabsf(h->state.setpoint_vel) < 1.0f) {
        h->state.is_at_target = true;
        h->motor_if->set_speed(h->motor_ctx, 0);
        return;
    }
    h->state.is_at_target = false;
    
    // Small deadzone for silence
    if (fabsf(pos_error) < 1.0f) pos_error = 0.0f;

    // 5. PID Compute
    float pid_out = h->pid_if->compute(h->pid_ctx, pos_error, SERVO_DT);
    h->debug_last_output = pid_out;

    // 6. Safety
    Check_Runaway(h, pid_out);
    if (h->error) return;

    // 7. Motor Actuation
    uint8_t pwm = (uint8_t)fabsf(pid_out);
    if (pwm > 100) pwm = 100;
    if (pwm < 2) pwm = 0; // Filter tiny spikes

    h->motor_if->set_direction(h->motor_ctx, (pid_out >= 0) ? 1 : 0);
    h->motor_if->set_speed(h->motor_ctx, pwm);
}

void Servo_SetTarget(Servo_Handle_t *handle, int32_t position)
{
    if (handle) {
        handle->state.target_pos = position;
        handle->state.is_at_target = false;
    }
}

bool Servo_IsAtTarget(Servo_Handle_t *handle)
{
    return (handle) ? handle->state.is_at_target : false;
}

void Servo_Start(Servo_Handle_t *handle)
{
    if (handle) {
         handle->enabled = true;
         handle->error = false;
         if (handle->motor_if->start) 
             handle->motor_if->start(handle->motor_ctx);
    }
}

void Servo_Stop(Servo_Handle_t *handle)
{
    if (handle) {
        handle->enabled = false;
        handle->motor_if->stop(handle->motor_ctx);
    }
}

int32_t Servo_DegreesToPulses(float degrees) {
    return (int32_t)degrees; // 1:1 for now
}

float Servo_PulsesToDegrees(int32_t pulses) {
    return (float)pulses;
}

/*******************************************************************************
 * CLI / DEBUG IMPLEMENTATION
 ******************************************************************************/

void Servo_ProcessCommand(Servo_Handle_t *h, char* cmd)
{
    if (!h || !cmd) return;
    
    char type = cmd[0];
    int32_t val = atoi(&cmd[1]);
    
    switch (type) {
        case 'G': case 'g': // Go Absolute
            UART_Debug_Printf("[CMD] Go Abs: %d\r\n", val);
            Servo_SetTarget(h, val);
            break;
            
        case 'R': case 'r': // Relative
            UART_Debug_Printf("[CMD] Go Rel: %d\r\n", val);
            Servo_SetTarget(h, h->state.target_pos + val);
            break;
            
        case 'Z': case 'z': // Zero
            UART_Debug_Printf("[CMD] Zero Position\r\n");
            h->motor_if->reset_encoder(h->motor_ctx, 0);
            h->state.target_pos = 0;
            h->state.setpoint_pos = 0;
            h->state.actual_pos = 0;
            h->pid_if->reset(h->pid_ctx);
            break;
            
        case 'S': case 's': // Stop
            UART_Debug_Printf("[CMD] Stop\r\n");
            Servo_SetTarget(h, h->state.actual_pos);
            break;
            
        case 'L': case 'l': // Limit
            if (val < 0) val = 0;
            if (val > 100) val = 100;
            UART_Debug_Printf("[CMD] Set Limit: %d%%\r\n", val);
            h->config.output_limit = (float)val;
            if (h->pid_if->set_limit)
                h->pid_if->set_limit(h->pid_ctx, (float)val);
            break;

        case 'P': case 'p': // Print
            UART_Debug_Printf("[STAT] Tgt: %d, Act: %d, PWM: %.1f\r\n", 
                h->state.target_pos, h->state.actual_pos, h->debug_last_output);
            break;
            
        case 'E': case 'e': // Test
            Servo_RunEncoderTest(h);
            break;
            
        case 'H': case 'h': default:
            UART_Debug_Printf("Help: G=Go, R=Rel, Z=Zero, S=Stop, L=Limit, P=Print, E=Test\r\n> ");
            break;
    }
}

void Poll_UART_Commands(void)
{
    uint8_t rx;
    while (UART_Read(UART_DEBUG_CHANNEL, &rx)) {
        UART_Send(UART_DEBUG_CHANNEL, &rx, 1); // Echo

        if (rx == '\r' || rx == '\n') {
            if (cmd_index > 0) {
                cmd_buffer[cmd_index] = '\0';
                UART_Debug_Printf("\r\n");
                // Use default handle for global CLI
                Servo_ProcessCommand(Get_Default_Handle(), cmd_buffer);
                cmd_index = 0;
            }
        } else if (rx == 0x08 || rx == 0x7F) { // Backspace
            if (cmd_index > 0) {
                cmd_index--;
                UART_Debug_Printf("\b \b");
            }
        } else if (cmd_index < CMD_BUFFER_SIZE - 1) {
            cmd_buffer[cmd_index++] = (char)rx;
        }
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    Delay_TIM_PeriodElapsedCallback(htim); // Chain existing
    
    if (htim->Instance == TIM3) {
        for (uint8_t i = 0; i < servo_instance_count; i++) {
            if (servo_instances[i]) {
                Servo_Update(servo_instances[i]);
                servo_instances[i]->debug_counter++;
            }
        }
    }
}

void Servo_RunEncoderTest(Servo_Handle_t *h)
{
    if (!h) return;
    
    UART_Debug_Printf("\r\n=== Encoder Test (W/S/Space/Q) ===\r\n");
    h->enabled = false; // Disable loop
    h->motor_if->start(h->motor_ctx);
    
    int16_t pwm = 0;
    uint8_t rx;
    
    while(1) {
        int32_t enc = h->motor_if->get_encoder(h->motor_ctx);
        UART_Debug_Printf("\r[TEST] PWM: %3d | Enc: %6d   ", pwm, enc);
        
        if (UART_Read(UART_DEBUG_CHANNEL, &rx)) {
            if (rx == 'q' || rx == 'Q') break;
            if (rx == 'w') pwm += 10;
            if (rx == 's') pwm -= 10;
            if (rx == ' ') pwm = 0;
            
            if (pwm > 100) pwm = 100;
            if (pwm < -100) pwm = -100;
            
            h->motor_if->set_direction(h->motor_ctx, (pwm >= 0));
            h->motor_if->set_speed(h->motor_ctx, (uint8_t)abs(pwm));
        }
        HAL_Delay(20);
    }
    
    h->motor_if->set_speed(h->motor_ctx, 0);
    UART_Debug_Printf("\r\nDone.\r\n> ");
    
    // Restore state
    int32_t now = h->motor_if->get_encoder(h->motor_ctx);
    h->state.target_pos = now;
    h->state.setpoint_pos = (float)now;
    h->state.actual_pos = now;
    h->pid_if->reset(h->pid_ctx);
    h->enabled = true;
}
