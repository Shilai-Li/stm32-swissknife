#ifndef SERVO_PORT_H
#define SERVO_PORT_H

#include "servo.h"

// Standard interfaces for the project's default drivers
extern const Servo_MotorInterface_t servo_motor_driver_interface;
extern const Servo_PIDInterface_t servo_pid_driver_interface;

#endif // SERVO_PORT_H
