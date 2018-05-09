#include <stdint.h>
#include <math.h>
#include "stdbool.h"
#include <driverlib/sysctl.h>
#include <driverlib/pin_map.h>
#include <driverlib/gpio.h>
#include <driverlib/i2c.h>
#include <inc/hw_memmap.h>

#define MIN_TA_ALLOWED -40 // For motor to work according to its datasheet
#define MAX_TA_ALLOWED 85
#define T_SLAVE_ADDRESS 0x3A // or 0X3B depending upon how pin is configured // thermometer 7-bit address given in page 15 of MLX90632 datasheet
#define LOW_TEMP_LIMIT -20 // Minimum temperature that can be detected for object
#define UPPER_TEMP_LIMIT 200 // Maximum temperature that can be detected for object
#define CALCULATION_ITERATIONS 3 // number of iterations sensor takes to calculate object temperature
#define KELVIN_OFFSET 273.15
#define TA_O 25
#define TO_O 25

// Private global variables shared only between this file's functions
static int16_t Gb, Ka, Ha, Hb;
static int32_t Ea, Eb, Fa, Fb, Ga, Pr, Po, Pg, Pt;

/*
 * Function Prototypes
 */
void ConnectWithTemperatureSensor();
double GetTemperature();
static void StartTemperatureSensor();
static void InitialiseCalibrationConstants();
static double CalculateTemperature(double temperature_old, int16_t status_reading);
static int16_t ReadFromRegister(uint16_t register_address);

/*
 * Initializes connections and constants needed to read object temperature
 * measurements continuously from the temperature sensor.
 */
void ConnectWithTemperatureSensor() {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C2);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL);
    SysCtlDelay(10);

    GPIOPinConfigure(GPIO_PL1_I2C2SCL);
    GPIOPinConfigure(GPIO_PL0_I2C2SDA);
    GPIOPinTypeI2CSCL(GPIO_PORTL_BASE, GPIO_PIN_1);
    GPIOPinTypeI2C(GPIO_PORTL_BASE, GPIO_PIN_0);
    I2CMasterInitExpClk(I2C2_BASE, SysCtlClockGet(), false);
    StartTemperatureSensor();
    InitialiseCalibrationConstants();
}

/*
 * Returns the motor's object temperature based on current sensor readings.
 */
double GetTemperature() {
    int16_t i, status_reading, new_data = 0;
    double temperature_C = 25; // Recommended initial value in page 22 of MLX90632 datasheet

    for (i = 0; i < CALCULATION_ITERATIONS; i++) {
        while (new_data == 0) {
            status_reading = ReadFromRegister(0x3FFF);
            new_data = (status_reading & 0b1);
        }

        temperature_C = CalculateTemperature(temperature_C, status_reading);
    }

    return temperature_C;
}

/*
 * Starts the temperature sensor connected to the motor.
 */
// Refer to pages 11, 12 and 16 of MLX90632 datasheet for exact process details.
static void StartTemperatureSensor() {
    int i = 0;
    I2CMasterSlaveAddrSet(I2C2_BASE, T_SLAVE_ADDRESS, false); // Send first slave address data bit to configure master device to be writer

    // Tell sensor to activate in continuous mode
    for (i = 0; i < 4; i++) { // four data bytes to send for a write operation
        switch(i) {
            case 0:
                I2CMasterDataPut(I2C2_BASE, 0x3001 >> 8); // Connect to REG_CONTROL register (MSByte transmission)
                break;
            case 1:
                I2CMasterDataPut(I2C2_BASE, 0x3001 & 0xff); // Connect to REG_CONTROL register (MSByte transmission)
                break;
            case 2:
                I2CMasterDataPut(I2C2_BASE, 0x00); // Enable temperature sensor in continous mode
                break;
            case 3:
                I2CMasterDataPut(I2C2_BASE, 0x06); // Enable temperature sensor in continous mode
                break;
        }

        I2CMasterControl(I2C2_BASE, I2C_MASTER_CMD_SINGLE_SEND); // Transmit data bits
        while(I2CMasterBusy(I2C2_BASE)); // Wait for transmission to end
    }
}

/*
 * Initializes relevant calibration constants from the temperature sensor using
 * addresses given in pages 11 and 12 of the datasheet.
 */
// Refer to pages 11, 12 and 16 of MLX90632 datasheet.
static void InitialiseCalibrationConstants() {
    Gb = (ReadFromRegister(0x242E)) * pow(2, -10);
    Ka = (ReadFromRegister(0x242F)) * pow(2, -10);
    Ha = (ReadFromRegister(0x2481)) * pow(2, -14);
    Hb = (ReadFromRegister(0x2482)) * pow(2, -14);
    Ea = ((ReadFromRegister(0x2425) << 16) | ReadFromRegister(0x2424)) * pow(2, -16);
    Eb = ((ReadFromRegister(0x2427) << 16) | ReadFromRegister(0x2426)) * pow(2, -8);
    Fa = ((ReadFromRegister(0x2429) << 16) | ReadFromRegister(0x2428)) * pow(2, -46);
    Fb = ((ReadFromRegister(0x242B) << 16) | ReadFromRegister(0x242A)) * pow(2, -36);
    Ga = ((ReadFromRegister(0x242D) << 16) | ReadFromRegister(0x242C)) * pow(2, -36);
    Pr = ((ReadFromRegister(0x240D) << 16) | ReadFromRegister(0x240C)) * pow(2, -8);
    Po = ((ReadFromRegister(0x2413) << 16) | ReadFromRegister(0x2412)) * pow(2, -8);
    Pg = ((ReadFromRegister(0x240F) << 16) | ReadFromRegister(0x240E)) * pow(2, -20);
    Pt = ((ReadFromRegister(0x2411) << 16) | ReadFromRegister(0x2410)) * pow(2, -44);
}

/*
 * Calculates the motor's object temperature based on current iteration's sensor readings.
 */
static double CalculateTemperature(double temperature_old, int16_t status_reading) {
    int16_t cycle_position = ((status_reading & 0b1111100) >> 2);
    int16_t RAM_6 = ReadFromRegister(0x4005), RAM_9 = ReadFromRegister(0x4008), RAM_A, RAM_B;
    double AMB, Ta_C, Ta_K, To_K, To_C, S, STO, VRTA, VRTO;

    if (cycle_position % 2 == 0) { // (RAM_4 and RAM_5) and (RAM_7 and RAM_8) should be used alternatively.
        // 0,2,4 .... N-2 measurements for (RAM_4 and RAM_5)
        RAM_A = ReadFromRegister(0x4003);
        RAM_B = ReadFromRegister(0x4004);
    } else {
        // 1,3,5 .... N-1 measurements for (RAM_7 and RAM_8)
        RAM_A = ReadFromRegister(0x4006);
        RAM_B = ReadFromRegister(0x4007);
    }

    // Calculation of object temperature using values and procedure giving in section 12 of datasheet.
    VRTA = RAM_9 + Gb * (RAM_6 / 12.0);
    AMB = ((RAM_6 / 12.0) / VRTA) * pow(2, 19);
    Ta_C = Po + ((AMB - Pr) / Pg) + Pt * pow((AMB - Pr), 2);
    S = (RAM_A + RAM_B) / 2.0;
    VRTO = RAM_9 + Ka * (RAM_6 / 12.0);
    STO = ((S / 12.0) / VRTO) * pow(2, 19);
    Ta_K = Ta_C + KELVIN_OFFSET;
    To_K = (STO / (Fa * Ha * (1.0 + Ga * (temperature_old - TO_O) + Fb * (Ta_C - TA_O)))) + pow(Ta_K, 4);
    To_C = pow(To_K, 0.25) - KELVIN_OFFSET - Hb;
    return To_C;
}

/*
 *  Performs a read operation on the specified 16 bit register address in the temperature sensor.
 */
static int16_t ReadFromRegister(uint16_t register_address) {
    int i, j, k;
    int16_t fetched_constant = 0x00;
    uint32_t send_receive[4] = {I2C_MASTER_CMD_BURST_SEND_START, I2C_MASTER_CMD_BURST_SEND_FINISH, I2C_MASTER_CMD_BURST_RECEIVE_START, I2C_MASTER_CMD_BURST_RECEIVE_FINISH};
    bool write_read[2] = {false, true};

    // Data bits transmission order as specified in page 16 of sensor datasheet.
    for (i = 0; i < 2; i++) {
        I2CMasterSlaveAddrSet(I2C2_BASE, T_SLAVE_ADDRESS, write_read[i]);

        for (j = 0; j < 2; j++) {
            switch (i*10 + j) {
                case 0:
                    I2CMasterDataPut(I2C2_BASE, register_address >> 8);
                    k = 0;
                    break;
                case 1:
                    I2CMasterDataPut(I2C2_BASE, register_address & 0xff);
                    k = 1;
                    break;
                case 10:
                    fetched_constant = (int16_t)(I2CMasterDataGet(I2C2_BASE) << 8);
                    k = 2;
                    break;
                case 11:
                    fetched_constant |= ((int16_t)I2CMasterDataGet(I2C2_BASE));
                    k = 3;
                    break;
            }

            I2CMasterControl(I2C2_BASE, send_receive[k]); // Transmit data bits
            while(I2CMasterBusy(I2C2_BASE)); // Wait for transmission to end
        }
    }

    return fetched_constant;
}
