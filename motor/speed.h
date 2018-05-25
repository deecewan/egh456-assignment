#ifndef MOTOR_SPEED_H_
#define MOTOR_SPEED_H_

#include <stdbool.h>

int ConnectWithHallSensors();
void ConnectWithMotor();
void StartMotor();
bool IsMotorFaulty();
void RotateMotor();
double GetMotorSpeed();
void SetMotorSpeed(int speed);
void StopMotor();

#endif /* MOTOR_SPEED_H_ */
