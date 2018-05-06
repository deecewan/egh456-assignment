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
static void StartTemperatureSensor();
static void InitialiseCalibrationConstants();
static double CalculateTemperature(double temperature_old, int16_t status_reading);
static int16_t ReadFromRegister(uint16_t register_address);

#endif /* MOTOR_TEMPERATURE_H_ */
