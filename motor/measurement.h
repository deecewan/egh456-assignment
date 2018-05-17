#ifndef MOTOR_MEASUREMENT_H_
#define MOTOR_MEASUREMENT_H_

void TakeMeasurements();
void MeasureTemperature();
bool ReadingsReady();
double GetFilteredSpeed();
double GetFilteredTemperature();
double GetFilteredCurrentValue();

#endif /* MOTOR_MEASUREMENT_H_ */
