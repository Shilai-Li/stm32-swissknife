#ifndef SERVO_PORT_H
#define SERVO_PORT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "servo.h"
#include "motor.h"
#include "algorithms/pid.h"

// Standard interfaces for the project's default drivers
extern const Servo_MotorInterface_t servo_motor_driver_interface;
extern const Servo_PIDInterface_t servo_pid_driver_interface;

// Standard instances (Global for debug/monitoring if needed)
extern Motor_Handle_t myMotor;
extern PIDController posPID;

/**
 * @brief Initialize a Servo handle with the default hardware port (Motor + PID)
 * 
 * This helper function performs the "Dependency Injection" internally.
 * It sets up the default Motor_Handle_t and PIDController, then binds them
 * to the generic Servo_Handle_t.
 * 
 * @param handle Pointer to your Servo_Handle_t instance
 * @return Servo_Status
 */
Servo_Status ServoPort_Init(Servo_Handle_t *handle);

// Helper to just set up the hardware structs (called by ServoPort_Init)
void ServoPort_Init_Hardware_Config(void);

#ifdef __cplusplus
}
#endif

#endif // SERVO_PORT_H
