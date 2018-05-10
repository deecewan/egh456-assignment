#include <stdint.h>
#include "stdbool.h"
#include <driverlib/sysctl.h>
#include <driverlib/pin_map.h>
#include <driverlib/gpio.h>
#include <driverlib/timer.h>
#include <driverlib/pwm.h>
#include <inc/hw_memmap.h>

#define NUM_STATES 6
#define MILLISECONDS_IN_MINUTE 6000

#define MOTOR_VREF 3.3
#define FAULT_VOLTS MOTOR_VREF // 11 indicates everything is normal, 00 shutdown, all else is warning
#define MAX_PWM_V 3.6
#define MIN_PWM_V 2.0
#define SAMPLING_FREQUENCY 500000 // 500 kHz
// Have a 100ns PWM pulse

const uint8_t HALL_SENSOR_STATES[NUM_STATES] = {1, 101, 100, 110, 10, 11};
static uint8_t checkpoint_state; // can only be changed through publicly available functions
static uint16_t counter = 1000;
static double duty_cycle = 0.25;


/*
 * Function Prototypes.
 */
uint8_t ConnectWithHallSensors();
double GetMotorSpeed();
void StartMotor();
void SetMotorSpeed();
void StopMotor();
void CalculateMaximumAcceleration();
static uint8_t GetCurrentHallState();

/*
 * Initializes connection to read from all three hall sensors and fault lines.
 */
uint8_t ConnectWithHallSensors() {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOP);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    GPIOPinTypeGPIOInput(GPIO_PORTC_BASE, GPIO_PIN_6);
    GPIOPinTypeGPIOInput(GPIO_PORTL_BASE, GPIO_PIN_2);
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
    if (difference < 0) {
        difference += NUM_STATES;
    }

    current_speed = (difference / ((double)NUM_STATES)) * MILLISECONDS_IN_MINUTE;
    checkpoint_state = current_state;
    return current_speed;
}

void StartMotor() {
    // Initialise timer hardware for PWM wave
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER3);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM);
    GPIOPinConfigure(GPIO_PM2_T3CCP0);
    GPIOPinTypeTimer(GPIO_PORTM_BASE, GPIO_PIN_2);

    // Configure timer and by extension, PWM parameters
    TimerConfigure(TIMER0_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PWM);
    TimerLoadSet(TIMER0_BASE, TIMER_A, 20000);
    TimerMatchSet(TIMER0_BASE, TIMER_A, counter); // PWM
    TimerEnable(TIMER0_BASE, TIMER_A);

    while(1) {
        counter++;
        TimerMatchSet(TIMER0_BASE, TIMER_A, counter); // PWM

        if (counter > 10000) {
            counter = 0;
        }
        SysCtlDelay(30000);
    }
    /*
    TimerConfigure(TIMER2_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PWM | TIMER_CFG_B_PWM);
    TimerLoadSet(TIMER2_BASE, TIMER_BOTH, SysCtlClockGet());
    TimerMatchSet(TIMER2_BASE, TIMER_BOTH, 0); // PWM
    TimerConfigure(TIMER3_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PWM | TIMER_CFG_B_PWM);
    TimerLoadSet(TIMER3_BASE, TIMER_BOTH, SysCtlClockGet());
    TimerMatchSet(TIMER3_BASE, TIMER_BOTH, 0); // PWM
    TimerEnable(TIMER2_BASE, TIMER_BOTH);
    TimerEnable(TIMER3_BASE, TIMER_BOTH);
    */
/*
    // Initiate connection with motor's half bridges and fault sensor
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    // Configure PWM pins
    GPIOPinConfigure(GPIO_PM0_T2CCP0);
    GPIOPinConfigure(GPIO_PM1_T2CCP1);
    GPIOPinConfigure(GPIO_PM2_T3CCP0);
    GPIOPinConfigure(GPIO_PA7_T3CCP1);
    GPIOPinConfigure(GPIO_PL4_T0CCP0);
    GPIOPinConfigure(GPIO_PL5_T0CCP1);
    GPIOPinTypeTimer(GPIO_PORTM_BASE, GPIO_PIN_0);
    GPIOPinTypeTimer(GPIO_PORTM_BASE, GPIO_PIN_1);
    GPIOPinTypeTimer(GPIO_PORTM_BASE, GPIO_PIN_2);
    GPIOPinTypeTimer(GPIO_PORTA_BASE, GPIO_PIN_7);
    GPIOPinTypeTimer(GPIO_PORTL_BASE, GPIO_PIN_4);
    GPIOPinTypeTimer(GPIO_PORTL_BASE, GPIO_PIN_5);

    // Configure timer and by extension, PWM parameters
    TimerConfigure(TIMER0_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PWM | TIMER_CFG_B_PWM);
    TimerLoadSet(TIMER0_BASE, TIMER_BOTH, SysCtlClockGet());
    TimerMatchSet(TIMER0_BASE, TIMER_BOTH, (uint32_t)(duty_cycle * SysCtlClockGet())); // PWM
    TimerConfigure(TIMER2_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PWM | TIMER_CFG_B_PWM);
    TimerLoadSet(TIMER2_BASE, TIMER_BOTH, SysCtlClockGet());
    TimerMatchSet(TIMER2_BASE, TIMER_BOTH, 0); // PWM
    TimerConfigure(TIMER3_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PWM | TIMER_CFG_B_PWM);
    TimerLoadSet(TIMER3_BASE, TIMER_BOTH, SysCtlClockGet());
    TimerMatchSet(TIMER3_BASE, TIMER_BOTH, 0); // PWM
    TimerEnable(TIMER0_BASE, TIMER_BOTH);
    TimerEnable(TIMER2_BASE, TIMER_BOTH);
    TimerEnable(TIMER3_BASE, TIMER_BOTH);

    // Configure PWM parameters
    PWMGenConfigure(PWM0_BASE, PWM_GEN_0, PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);
    //
    // Set the PWM period to 250Hz.  To calculate the appropriate parameter
    // use the following equation: N = (1 / f) * SysClk.  Where N is the
    // function parameter, f is the desired frequency, and SysClk is the
    // system clock frequency.
    // In this case you get: (1 / 250Hz) * 16MHz = 64000 cycles.  Note that
    // the maximum period you can set is 2^16 - 1.
    // TODO: modify this calculation to use the clock frequency that you are
    // using.
    //
    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_0, 64000);
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, PWMGenPeriodGet(PWM0_BASE, PWM_GEN_0) / 4);

    // Enable PWM signal
    PWMOutputState(PWM0_BASE, PWM_OUT_1_BIT | PWM_OUT_0_BIT, true);
    PWMGenEnable(PWM0_BASE, PWM_GEN_0);
    */
}

void SetMotorSpeed() {
    ;
}


void StopMotor() {
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
