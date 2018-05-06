#ifndef MOTOR_SPEED_H_
#define MOTOR_SPEED_H_

#define MILLISECONDS_IN_MINUTE 6000

uint8_t ConnectWithHallSensors();
double GetMotorSpeed();
void StartMotor();
void SetMotorSpeed();
void CalculateMaximumAcceleration();
static uint8_t GetCurrentHallState();

#endif /* MOTOR_SPEED_H_ */
