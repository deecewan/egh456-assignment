#ifndef MOTOR_TEMPERATURE_H_
#define MOTOR_TEMPERATURE_H_

#define LOW_TEMP_LIMIT -20 // Minimum temperature that can be detected for object
#define UPPER_TEMP_LIMIT 200 // Maximum temperature that can be detected for object
#define KELVIN_OFFSET 273.15

/*
 * Function Prototypes
 */
void ConnectWithTemperatureSensor();
double GetTemperature();

#endif /* MOTOR_TEMPERATURE_H_ */
