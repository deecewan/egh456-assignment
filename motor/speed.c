#include <stdint.h>
#include "stdbool.h"
#include <driverlib/sysctl.h>
#include <driverlib/pin_map.h>
#include <driverlib/gpio.h>
#include <inc/hw_memmap.h>

#define NUM_STATES 6
#define MILLISECONDS_IN_MINUTE 6000

const uint8_t HALL_SENSOR_STATES[NUM_STATES] = {1, 101, 100, 110, 10, 11};
static uint8_t checkpoint_state; // can only be changed through publicly available functions

/*
 * Function Prototypes.
 */
uint8_t ConnectWithHallSensors();
double GetMotorSpeed();
void StartMotor();
void SetMotorSpeed();
void CalculateMaximumAcceleration();
static uint8_t GetCurrentHallState();

/*
 * Initializes connection to read from all three hall sensors.
 */
uint8_t ConnectWithHallSensors() {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOP);
    GPIOPinTypeGPIOInput(GPIO_PORTL_BASE, GPIO_PIN_3);
    GPIOPinTypeGPIOInput(GPIO_PORTP_BASE, GPIO_PIN_4);
    GPIOPinTypeGPIOInput(GPIO_PORTP_BASE, GPIO_PIN_5);
    checkpoint_state = GetCurrentHallState();
    return checkpoint_state;
}

/*
 * Measures distance traveled from previous reference point to calculate motor
 * speed in revolutions per minute (RPM).
 *
 * Assumptions:
 * Sampling time stays fixed at 1 millisecond (1000 Hz).
 * Rotations less than or equal to 1 happen per millisecond.
 */
double GetMotorSpeed() {
    int difference;
    double current_speed = 0;
    uint8_t current_state = 6;

    current_state = GetCurrentHallState();
    if (current_state == 6) {
        return -1;
    }

    difference = checkpoint_state - current_state;
    if (difference <= 0) {
        difference += NUM_STATES;
    }

    current_speed = (difference / ((double)NUM_STATES)) * MILLISECONDS_IN_MINUTE;
    checkpoint_state = current_state;
    return current_speed;
}

void StartMotor() {
    ;
}

void SetMotorSpeed() {
    ;
}

void CalculateMaximumAcceleration() {
    ;
}

/*
 * Gets the current hall state sensors' reading.
 */
static uint8_t GetCurrentHallState() {
    uint8_t i, h1, h2, h3, checkpoint;
    h1 = (GPIOPinRead(GPIO_PORTL_BASE, GPIO_PIN_3) >> 3) & 1;
    h2 = (GPIOPinRead(GPIO_PORTP_BASE, GPIO_PIN_4) >> 4) & 1;
    h3 = (GPIOPinRead(GPIO_PORTP_BASE, GPIO_PIN_5) >> 5) & 1;
    checkpoint = h1*100 + h2*10 + h3;

    for (i = 0; i < 6; i++) {
        if (checkpoint == HALL_SENSOR_STATES[i]) {
            return i;
        }
    }

    return 6; // hall sensor readings indicate a hardware fault
}

// Code Cemetery
/*
double revolutions = 0, milliseconds = 0;
double current_speed = 0;

// Method that is constantly measuring speed in parallel to other methods
void StartTrackingMotorSpeed() {
    int8_t h1, h2, h3, target = 0, result = 0;

    h1 = (GPIOPinRead(GPIO_PORTL_BASE, GPIO_PIN_3) >> 3) & 1;
    h2 = (GPIOPinRead(GPIO_PORTP_BASE, GPIO_PIN_4) >> 4) & 1;
    h3 = (GPIOPinRead(GPIO_PORTP_BASE, GPIO_PIN_5) >> 5) & 1;
    target = h1*100 + h2*10 + h3;

    if (target == 0 || target == 111) {
        return;
    }

    while (1) {
        h1 = (GPIOPinRead(GPIO_PORTL_BASE, GPIO_PIN_3) >> 3) & 1;
        h2 = (GPIOPinRead(GPIO_PORTP_BASE, GPIO_PIN_4) >> 4) & 1;
        h3 = (GPIOPinRead(GPIO_PORTP_BASE, GPIO_PIN_5) >> 5) & 1;
        result = h1*100 + h2*10 + h3;

        if (target == result) {
            revolutions += 1;
        } else if (target == 0 || target == 111) {
            return;
        }
    }
}

// Configure and enable timer interrupts for speed tracking.
void StartTimerZero() {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
    TimerLoadSet(TIMER0_BASE, TIMER_A, SysCtlClockGet());
    IntEnable(INT_TIMER0A);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    TimerEnable(TIMER0_BASE, TIMER_A);
    TimerIntRegister(TIMER0_BASE, TIMER_A, &AddTenMilliSeconds);
    IntMasterEnable();
}

void AddTenMilliSeconds() {
    milliseconds += 10;
}
*/
