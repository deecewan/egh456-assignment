#ifndef MOTOR_MEASUREMENT_H_
#define MOTOR_MEASUREMENT_H_

void MeasurementInit();
void TakeMeasurements();
void MeasureTemperature();
double GetFilteredSpeed();
double GetFilteredTemperature();
double GetFilteredCurrentValue();

#endif /* MOTOR_MEASUREMENT_H_ */
