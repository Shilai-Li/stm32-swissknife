#ifndef SERVO_PORT_H
#define SERVO_PORT_H

#include "servo.h"

#include "motor_driver.h"
#include "algorithms/pid.h"

// Standard interfaces for the project's default drivers
extern const Servo_MotorInterface_t servo_motor_driver_interface;
extern const Servo_PIDInterface_t servo_pid_driver_interface;

// Standard instances
extern Motor_Handle_t myMotor;
extern PIDController posPID;

void Servo_Port_Init_Default_Config(void);

/**
 * @brief Default Port Implementation for Servo Component
 * 
 * This file encapsulates the "Glue Code" (Adapters) that connects
 * the generic `servo.h` logic to the specific hardware drivers used in this project:
 * - Motor Driver: `motor_driver.h`
 * - PID Library: `algorithms/pid.h`
 * 
 * It also holds the standard `myMotor` and `posPID` instances used by the basic tests.
 * 
 * To add a new example or change the driver:
 * 1. Create a similar file (e.g., `servo_port_stepper.c`).
 * 2. Define your own adapter functions calling your specific driver APIs.
 * 3. Use your new interface struct in `Servo_InitInstance` instead of these defaults.
 */
#endif // SERVO_PORT_H
