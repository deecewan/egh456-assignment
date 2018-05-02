#include <stdint.h>
#include "stdbool.h"
#include <xdc/runtime/System.h>
#include <driverlib/sysctl.h>
#include <driverlib/pin_map.h>
#include <driverlib/gpio.h>
#include "driverlib/i2c.h"
#include <inc/hw_memmap.h>

#define T_SLAVE_ADDRESS 0x3A // or 0X3B depending upon how pin is configured // thermometer 7-bit address given in page 15 of MLX90632 datasheet
#define LOW_TEMP_LIMIT -20 // Minimum temperature that can be detected for object
#define UPPER_TEMP_LIMIT 200 // Maximum temperature that can be detected for object
#define CALIBRATION_CONSTANTS 36

int temperature_C = 25; // Recommended initial value in page ... of MLX90632 datasheet
int16_t Gb, Ka, Ha, Hb;
int32_t Ea, Eb, Fa, Fb, Ga, Pr, Po, Pg, Pt;

/*
 * Function Prototypes
 */
void ConnectWithTemperatureSensor();
void StartTemperatureSensor();
void InitialiseCalibrationConstants();
uint16_t ReadFromRegister(uint16_t register_address);
int CalculateTemperatureReading(int temperature_old);
int GetTemperature();

/*
 * Initializes connections needed to communicate with the temperature sensor
 * connected to motor.
 */
void ConnectWithTemperatureSensor() {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL);
    GPIOPinConfigure(GPIO_PL1_I2C2SCL);
    GPIOPinConfigure(GPIO_PL0_I2C2SDA);
    GPIOPinTypeI2CSCL(GPIO_PORTL_BASE, GPIO_PIN_1);
    GPIOPinTypeI2C(GPIO_PORTL_BASE, GPIO_PIN_0);
    I2CMasterInitExpClk(I2C2_BASE, SysCtlClockGet(), true);
    StartTemperatureSensor();
    InitialiseCalibrationConstants();
}

/*
 * Starts the temperature sensor connected to the motor.
 */
// Refer to pages 11, 12 and 16 of MLX90632 datasheet for exact process details.
void StartTemperatureSensor() {
    int i = 0;
    I2CMasterSlaveAddrSet(I2C2_BASE, T_SLAVE_ADDRESS, false); // Send first slave address data bit to configure master device to be writer

    // Tell sensor to activate in continous mode
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
 * Reads relevant calibration constants from the temperature sensor using addresses given in datasheet.
 * Needs to be called once during device setup as indicated by the word 'constants'.
 */
// Refer to pages 11, 12 and 16 of MLX90632 datasheet.
void InitialiseCalibrationConstants() {
    Gb = ReadFromRegister(0x242E);
    Ka = ReadFromRegister(0x242F);
    Ha = ReadFromRegister(0x2481);
    Hb = ReadFromRegister(0x2482);
    Ea = ((ReadFromRegister(0x2425) << 16) | ReadFromRegister(0x2424));
    Eb = ((ReadFromRegister(0x2427) << 16) | ReadFromRegister(0x2426));
    Fa = ((ReadFromRegister(0x2429) << 16) | ReadFromRegister(0x2428));
    Fb = ((ReadFromRegister(0x242B) << 16) | ReadFromRegister(0x242A));
    Ga = ((ReadFromRegister(0x242D) << 16) | ReadFromRegister(0x242C));
    Pr = ((ReadFromRegister(0x240D) << 16) | ReadFromRegister(0x240C));
    Po = ((ReadFromRegister(0x2413) << 16) | ReadFromRegister(0x2412));
    Pg = ((ReadFromRegister(0x240F) << 16) | ReadFromRegister(0x240E));
    Pt = ((ReadFromRegister(0x2411) << 16) | ReadFromRegister(0x2410));
}

/*
 *  Sends a request to the temperature sensor to read data stored in the specified 16 bit register.
 */
uint16_t ReadFromRegister(uint16_t register_address) {
    int i, j;
    uint16_t fetched_constant = 0x00;
    uint32_t send_receive[2] = {I2C_MASTER_CMD_SINGLE_SEND, I2C_MASTER_CMD_SINGLE_RECEIVE};
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
                    fetched_constant = (uint16_t)(I2CMasterDataGet(I2C2_BASE) << 8);
                    break;
                case 11:
                    fetched_constant |= ((uint16_t)I2CMasterDataGet(I2C2_BASE));
                    break;
            }

            I2CMasterControl(I2C2_BASE, send_receive[i]); // Transmit data bits
            while(I2CMasterBusy(I2C2_BASE)); // Wait for transmission to end
        }
    }

    return fetched_constant;
}

/*
 * Based on the calibration constants initially loaded and old temperature value in degrees Celsius,
 * get RAM measurements to calculate current temperature value in degrees Celsius.
 */
int CalculateTemperatureReading(int temperature_old) {
    int temperature_new = 0;



    return temperature_new;
}

/*
 * Get motor's average temperature readings as a 32 bit integer format.
 */
int GetTemperature() {
    int i;

    for (i = 0; i < 3; i++) {
        temperature_C = CalculateTemperatureReading(temperature_C);
    }

    return temperature_C;
}
