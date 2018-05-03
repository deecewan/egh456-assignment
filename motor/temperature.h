/*
 * temperature.h
 *
 *  Created on: May 4, 2018
 *      Author: Dhiraj
 */

#ifndef MOTOR_TEMPERATURE_H_
#define MOTOR_TEMPERATURE_H_

#define T_SLAVE_ADDRESS 0x3A // or 0X3B depending upon how pin is configured // thermometer 7-bit address given in page 15 of MLX90632 datasheet
#define LOW_TEMP_LIMIT -20 // Minimum temperature that can be detected for object
#define UPPER_TEMP_LIMIT 200 // Maximum temperature that can be detected for object
#define CALCULATION_ITERATIONS 3 // number of iterations sensor takes to calculate object temperature
#define KELVIN_OFFSET 273.15
#define TA_O 25
#define TO_O 25

int16_t Gb, Ka, Ha, Hb;
int32_t Ea, Eb, Fa, Fb, Ga, Pr, Po, Pg, Pt;

/*
 * Function Prototypes
 */
void ConnectWithTemperatureSensor();
double GetTemperature();
void StartTemperatureSensor();
void InitialiseCalibrationConstants();
double CalculateTemperature(double temperature_old, int16_t status_reading);
int16_t ReadFromRegister(uint16_t register_address);

#endif /* MOTOR_TEMPERATURE_H_ */
