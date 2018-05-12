#ifndef MOTOR_SPEED_H_
#define MOTOR_SPEED_H_

#define SECONDS_IN_MINUTE 60
#define T_CPU_CLOCK_SPEED 120000000

int ConnectWithHallSensors();
double GetMotorSpeed(double seconds);
void ConnectWithMotor();
void RunMotor();
void SetMotorSpeed(int speed);
void StopMotor();

#endif /* MOTOR_SPEED_H_ */
