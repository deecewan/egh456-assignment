#include <stdint.h>
#include <math.h>
#include <stdbool.h>
#include <driverlib/sysctl.h>
#include <driverlib/pin_map.h>
#include <driverlib/gpio.h>
#include <driverlib/i2c.h>
#include <ti/drivers/I2C.h>
#include <inc/hw_memmap.h>
//#include "Board.h"

#define T_SLAVE_ADDRESS 0x3A // page 15 of MLX90632 datasheet
#define CALCULATION_ITERATIONS 3 // page 22 of MLX90632 datasheet

#define KELVIN_OFFSET 273.15
#define TA_O 25
#define TO_O 25

static double Gb, Ka, Ha, Hb;
static double Ea, Eb, Fa, Fb, Ga, Pr, Po, Pg, Pt;
static double temperature_C = 25; // Recommended initial value in page 22 of MLX90632 datasheet

/*
 * Function Prototypes
 */
void ConnectWithTemperatureSensor();
int* ReturnSamples();
double GetTemperature();
static void InitialiseCalibrationConstants();
static double CalculateTemperature(double temperature_old, int16_t status_reading);
static void WriteToRegister(uint16_t register_address, uint16_t data_packet);
static int32_t ReadFromRegister(uint16_t register_address);

/*
 * Initializes connections and constants needed to read object temperature
 * measurements continuously from the temperature sensor.
 */
void ConnectWithTemperatureSensor() {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C2);
    GPIOPinConfigure(GPIO_PL1_I2C2SCL);
    GPIOPinConfigure(GPIO_PL0_I2C2SDA);
    GPIOPinTypeI2CSCL(GPIO_PORTL_BASE, GPIO_PIN_1);
    GPIOPinTypeI2C(GPIO_PORTL_BASE, GPIO_PIN_0);
    I2CMasterInitExpClk(I2C2_BASE, SysCtlClockGet(), false);
    WriteToRegister(0x3001, 0x06); // starts sensor in continuous mode
    InitialiseCalibrationConstants();
    WriteToRegister(0x3FFF, 0x100); // reset bits in REG_STATUS
}

/*
 * Initializes relevant calibration constants from the temperature sensor using
 * addresses and equations given in pages 11, 12 and 23 of the datasheet.
 */
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
 * Returns the motor's object temperature based on current sensor readings.
 */
double GetTemperature() {
    int16_t status_reading = 0, new_data = 0;
    status_reading = ReadFromRegister(0x3FFF);
    new_data = (status_reading & 1);

    if (new_data != 0) { // Check if sensor has new reading
        temperature_C = CalculateTemperature(temperature_C, status_reading) / 275619218;
        WriteToRegister(0x3FFF, 0x100); // reset bits in REG_STATUS
    }

    return temperature_C;
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
 * Writes the given data packet to the given register address by following
 * the process described in page 16 of the MLX90632 datasheet.
 */
void WriteToRegister(uint16_t register_address, uint16_t data_packet) {
    I2CMasterSlaveAddrSet(I2C2_BASE, T_SLAVE_ADDRESS, false);
    while(I2CMasterBusy(I2C2_BASE));

    I2CMasterDataPut(I2C2_BASE, register_address >> 8);
    I2CMasterControl(I2C2_BASE, I2C_MASTER_CMD_BURST_SEND_START);
    while(I2CMasterBusy(I2C2_BASE));

    I2CMasterDataPut(I2C2_BASE, register_address & 0xff);
    I2CMasterControl(I2C2_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
    while(I2CMasterBusy(I2C2_BASE));

    I2CMasterDataPut(I2C2_BASE, data_packet >> 8);
    I2CMasterControl(I2C2_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
    while(I2CMasterBusy(I2C2_BASE));

    I2CMasterDataPut(I2C2_BASE, data_packet & 0xff);
    I2CMasterControl(I2C2_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);
    while(I2CMasterBusy(I2C2_BASE));
}

/*
 *  Performs a read operation on the specified 16 bit register address in the
 *  temperature sensor by following the process described in page 16 of the
 *  MLX90632 datasheet.
 */
static int32_t ReadFromRegister(uint16_t register_address) {
    int i, j, k = 0;
    int32_t fetched_constant = 0;
    uint32_t send_receive[4] = {I2C_MASTER_CMD_BURST_SEND_START, I2C_MASTER_CMD_BURST_SEND_FINISH, I2C_MASTER_CMD_BURST_RECEIVE_START, I2C_MASTER_CMD_BURST_RECEIVE_FINISH};
    bool write_read[2] = {false, true};

    // Data bits transmission order as specified in page 16 of sensor datasheet.
    for (i = 0; i < 2; i++) {
        I2CMasterSlaveAddrSet(I2C2_BASE, T_SLAVE_ADDRESS, write_read[i]);

        for (j = 0; j < 2; j++) {
            switch (i*10 + j) {
                case 0:
                    I2CMasterDataPut(I2C2_BASE, register_address >> 8);
                    break;
                case 1:
                    I2CMasterDataPut(I2C2_BASE, register_address & 0xff);
                    break;
                case 10:
                    fetched_constant = ((int32_t)I2CMasterDataGet(I2C2_BASE));
                    fetched_constant = fetched_constant << 8;
                    break;
                case 11:
                    fetched_constant |= ((int32_t)I2CMasterDataGet(I2C2_BASE));
                    break;
            }

            I2CMasterControl(I2C2_BASE, send_receive[k]); // Transmit data bits
            while(I2CMasterBusy(I2C2_BASE)); // Wait for transmission to end
            ++k;
        }
    }

    return fetched_constant;
}
