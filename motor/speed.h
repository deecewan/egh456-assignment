#ifndef MOTOR_SPEED_H_
#define MOTOR_SPEED_H_

int ConnectWithHallSensors();
void ConnectWithMotor();
void StartMotor();
bool IsMotorFaulty();
void RotateMotor();
double GetMotorSpeed();
void SetMotorSpeed(int speed);
void StopMotor();

#endif /* MOTOR_SPEED_H_ */
