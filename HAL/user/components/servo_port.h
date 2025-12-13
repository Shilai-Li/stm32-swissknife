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

#endif // SERVO_PORT_H
