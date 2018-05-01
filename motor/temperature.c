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

int temperature_C = 25; // Recommended initial value in page ... of MLX90632 datasheet

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
    GetCalibrationConstants();
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
 * Reads calibration constants from the temperature sensor. Needs to be called once during device
 * setup as indicated by the word 'constants'.
 */
// Refer to pages 11, 12 and 16 of MLX90632 datasheet.
void GetCalibrationConstants() {
    int i, j, num_of_constants = 36;
    uint16_t constants_buffer[num_of_constants];
    uint16_t start_address = 0x240C; // starting address to read calibration constants

    // Get object temperature values and needed calibration constants through incremental addressing
    for (i = 0; i < num_of_constants; i++) {
        for (j = 0; j < 6; j++) {
            switch(i) { // Read command transmission as specified in page 16 of sensor datasheet
                case 0:
                    I2CMasterSlaveAddrSet(I2C2_BASE, T_SLAVE_ADDRESS, false);
                    break;
                case 1:
                    I2CMasterDataPut(I2C2_BASE, start_address >> 8);
                    I2CMasterControl(I2C2_BASE, I2C_MASTER_CMD_SINGLE_SEND); // Transmit data bits
                    break;
                case 2:
                    I2CMasterDataPut(I2C2_BASE, start_address & 0xff);
                    I2CMasterControl(I2C2_BASE, I2C_MASTER_CMD_SINGLE_SEND); // Transmit data bits
                    break;
                case 3:
                    I2CMasterSlaveAddrSet(I2C2_BASE, T_SLAVE_ADDRESS, true);
                    break;
                case 4:
                    constants_buffer[i] = (I2CMasterDataGet(I2C2_BASE) << 8);
                    I2CMasterControl(I2C2_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE); // Transmit data bits
                    break;
                case 5:
                    constants_buffer[i] |= I2CMasterDataGet(I2C2_BASE);
                    I2CMasterControl(I2C2_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE); // Transmit data bits
                    break;
            }
            while(I2CMasterBusy(I2C2_BASE)); // Wait for transmission to end
        }
        start_address = start_address + 1;
    }
}

/*
 * Initializes connections needed to communicate with the temperature sensor
 * mounted on motor as well as configure connecting pins to be used for I2C
 * communication.
 */
void ConvertTemperatureReadings() {
    ;
}

/*
 * Get motor's average temperature readings as a 32 bit integer format.
 */
int GetTemperature() {
    InitialiseI2CConnection();
    FetchSensorReadings();
    ConvertTemperatureReadings();
    return 0;
}
