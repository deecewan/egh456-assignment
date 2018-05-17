#include <stdint.h>
#include "stdbool.h"
#include <driverlib/sysctl.h>
#include <driverlib/pin_map.h>
#include <driverlib/gpio.h>
#include <driverlib/timer.h>
#include <driverlib/pwm.h>
#include <inc/hw_memmap.h>
#include <xdc/runtime/System.h>

/*
 * Module constants.
 */
#define NUM_STATES 6
#define MILLISECONDS_IN_MINUTE 6000
#define MAX_SPEED 5000 // Max speed at which present motor can spin in revolutions per minute (RPM)
#define T_CPU_CLOCK_SPEED 120000000
#define SAMPLING_FREQUENCY 500000 // Fixed PWM frequency at which motor performs best
#define MAX_DUTY 0.5

static const uint8_t HALL_SENSOR_STATES[NUM_STATES] = {1, 101, 100, 110, 10, 11};
static const uint16_t TIMER_CYCLES = T_CPU_CLOCK_SPEED / SAMPLING_FREQUENCY;

/*
 * Module variables.
 */
static uint8_t current_state, checkpoint_state;
static uint16_t match_point;
static int milliseconds = 0;
static double current_speed = 0, desired_speed = 0, duty_cycle = 0, error_sum = 0;
static double Kp = 0, Ki = 0, revolutions = 0; // Kp and Ki be determined through trial and error
static bool run_motor = false, faulty_motor = false;

/*
 * Function Prototypes.
 */
int ConnectWithHallSensors();
void ConnectWithMotor();
void StartMotor();
bool IsMotorFaulty();
void RotateMotor();
double GetMotorSpeed();
void SetMotorSpeed(int speed);
void StopMotor();
static void CheckForFaultSignal();
static uint8_t GetCurrentHallState();
static void AddToCurrentRevolutions();
static void PIControl();
static void FeedbackControl();

/*
 * Initializes connection to read from all three hall sensors and fault lines.
 *
 * Output: Hall sensor readings that the calling function can check for any
 * faults (represented by -1).
 */
int ConnectWithHallSensors() {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOP);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    GPIOPinTypeGPIOInput(GPIO_PORTC_BASE, GPIO_PIN_6);
    GPIOPinTypeGPIOInput(GPIO_PORTL_BASE, GPIO_PIN_2);
    GPIOPinTypeGPIOInput(GPIO_PORTL_BASE, GPIO_PIN_3);
    GPIOPinTypeGPIOInput(GPIO_PORTP_BASE, GPIO_PIN_4);
    GPIOPinTypeGPIOInput(GPIO_PORTP_BASE, GPIO_PIN_5);
    current_state = GetCurrentHallState();
    checkpoint_state = current_state;

    if (checkpoint_state >= 0 && checkpoint_state <= 5) {
        return ((int)checkpoint_state);
    } else {
        return -1;
    }
}

/*
 * Initializes all the connections needed to send a PWM wave through to the
 * motor's half wave bridges.
 */
void ConnectWithMotor() {
    // Initiate connection with motor's half bridges and fault sensor
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    // Initialize timer hardware for PWM wave
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER3);

    // Configure PWM pins
    GPIOPinConfigure(GPIO_PM0_T2CCP0);
    GPIOPinConfigure(GPIO_PM1_T2CCP1);
    GPIOPinConfigure(GPIO_PM2_T3CCP0);
    GPIOPinConfigure(GPIO_PA7_T3CCP1);
    GPIOPinConfigure(GPIO_PL4_T0CCP0);
    GPIOPinConfigure(GPIO_PL5_T0CCP1);
    GPIOPinTypeTimer(GPIO_PORTM_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2);
    GPIOPinTypeTimer(GPIO_PORTA_BASE, GPIO_PIN_7);
    GPIOPinTypeTimer(GPIO_PORTL_BASE, GPIO_PIN_4 | GPIO_PIN_5);

    // Configure timers to send PWM wave later on
    TimerDisable(TIMER0_BASE, TIMER_BOTH);
    TimerDisable(TIMER2_BASE, TIMER_BOTH);
    TimerDisable(TIMER3_BASE, TIMER_BOTH);
    TimerConfigure(TIMER0_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PWM | TIMER_CFG_B_PWM);
    TimerConfigure(TIMER2_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PWM | TIMER_CFG_B_PWM);
    TimerConfigure(TIMER3_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PWM | TIMER_CFG_B_PWM);
    TimerLoadSet(TIMER0_BASE, TIMER_BOTH, TIMER_CYCLES - 1);
    TimerLoadSet(TIMER2_BASE, TIMER_BOTH, TIMER_CYCLES - 1);
    TimerLoadSet(TIMER3_BASE, TIMER_BOTH, TIMER_CYCLES - 1);
    TimerMatchSet(TIMER0_BASE, TIMER_BOTH, TIMER_CYCLES - 1);
    TimerMatchSet(TIMER2_BASE, TIMER_BOTH, TIMER_CYCLES - 1);
    TimerMatchSet(TIMER3_BASE, TIMER_BOTH, TIMER_CYCLES - 1);
}

/*
 * Initialises and enables all the connections needed to start the motor.
 */
void StartMotor() {
    ConnectWithHallSensors();
    ConnectWithMotor();
    TimerEnable(TIMER0_BASE, TIMER_BOTH);
    TimerEnable(TIMER2_BASE, TIMER_BOTH);
    TimerEnable(TIMER3_BASE, TIMER_BOTH);
    match_point = TIMER_CYCLES-1;
    duty_cycle = 0.1;
    run_motor = true;
}

/*
 * Returns whether the motor is in a faulty state or not.
 */
bool IsMotorFaulty() {
    return faulty_motor;
}

/*
 * Changes PWM cycle sent to motor depending upon current hall sensor reading of motor.
 *
 * Assumption: StartMotor() has been called before this function.
 */
void RotateMotor() {
    if (!run_motor) {
        return;
    }

    TimerSynchronize(TIMER0_BASE, (TIMER_0A_SYNC | TIMER_0B_SYNC | TIMER_2A_SYNC | TIMER_2B_SYNC | TIMER_3A_SYNC | TIMER_3B_SYNC));
    match_point = ((uint16_t)(TIMER_CYCLES - (duty_cycle * TIMER_CYCLES)));
    FeedbackControl();
    CheckForFaultSignal();
    current_state = GetCurrentHallState();

    ++milliseconds;
    if (milliseconds >= 500) {
        current_speed = (MILLISECONDS_IN_MINUTE / milliseconds) * revolutions;
        revolutions = 0;
        milliseconds = 0;
    } else {
        AddToCurrentRevolutions();
    }

    duty_cycle = duty_cycle + 0.000001;
}

/*
 * Returns the currently recorded speed for the motor.
 */
double GetMotorSpeed() {
    return current_speed;
}

/*
 * Brings the motor speed up or down to the desired speed by using a
 * safe acceleration or deceleration margin until it has reached
 * desired speed.
 */
void SetMotorSpeed(int speed) {
    // User input error handling for unsupported speed demands
    if (speed < 0) {
        return; // This is because the UI doesn't let me select speeds other than 0, WILL CHANGE IT TO duty_cycle = 0
    } else if(speed >= MAX_SPEED) {
        desired_speed = MAX_SPEED;
    } else {
        desired_speed = speed;
    }
}

/*
 * Brings the motor to a stopping state and when the speed is low enough, stops the motor itself.
 */
void StopMotor() {
    //SetMotorSpeed(0);
    current_speed = 0;
    //while(1);
    run_motor = false;
    TimerDisable(TIMER0_BASE, TIMER_BOTH);
    TimerDisable(TIMER2_BASE, TIMER_BOTH);
    TimerDisable(TIMER3_BASE, TIMER_BOTH);
}

/*
 * Keeps checking whether the motor has sent a overheating or excess current fault reading.
 */
static void CheckForFaultSignal() {
    uint8_t f1, f2, sum;
    f1 = (GPIOPinRead(GPIO_PORTC_BASE, GPIO_PIN_6) >> 6) & 1;
    f2 = (GPIOPinRead(GPIO_PORTL_BASE, GPIO_PIN_2) >> 2) & 1;
    sum = f1 + f2;

    if (sum == 0) {
        faulty_motor = true;
    }
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

    faulty_motor = true;
    return 100; // reading to indicate hardware fault
}

/*
 * Measures distance traveled from previous reference point to help calculate motor
 * speed in revolutions per minute (RPM).
 *
 * Assumption: At most one rotation could have happened since this function was
 * last called.
 */
static void AddToCurrentRevolutions() {
    int difference = checkpoint_state - current_state;
    if (difference < 0) { // measurable change in states can only be from 0 to 5
        difference += NUM_STATES;
    }

    revolutions += (difference / ((double)NUM_STATES));
    checkpoint_state = current_state;
}

/*
 * Accelerates or decelerates the motor by a safe margin (using a PI controller as suggested in
 * week 7 lecture) to get it to go to a desirable speed.
 */
static void PIControl() {
    double error = 0;
    error = desired_speed - current_speed;
    error_sum += error;
    duty_cycle += (Kp * error + Ki * error_sum);

    if (duty_cycle >= MAX_DUTY) { // Have a 100ns PWM pulse as specified in datasheet
        duty_cycle = MAX_DUTY;
    }
}

/*
 * Helper function to change the PWM waves sent to motor depending upon current
 * hall sensor position of motor.
 */
static void FeedbackControl() {
    switch (current_state) {
        case 0: // H1, H2, H3 = 0, 0, 1
            // PWM + RESET A
            TimerMatchSet(TIMER3_BASE, TIMER_A, match_point);
            TimerMatchSet(TIMER3_BASE, TIMER_B, 0);

            // PWM + RESET B
            TimerMatchSet(TIMER2_BASE, TIMER_B, TIMER_CYCLES-1);
            TimerMatchSet(TIMER0_BASE, TIMER_B, TIMER_CYCLES-1);

            // PWM + RESET C
            TimerMatchSet(TIMER2_BASE, TIMER_A, match_point);
            TimerMatchSet(TIMER0_BASE, TIMER_A, TIMER_CYCLES-1);
            break;
        case 1: // H1, H2, H3 = 1, 0, 1
            // PWM + RESET A
            TimerMatchSet(TIMER3_BASE, TIMER_A, match_point);
            TimerMatchSet(TIMER3_BASE, TIMER_B, TIMER_CYCLES-1);

            // PWM + RESET B
            TimerMatchSet(TIMER2_BASE, TIMER_B, TIMER_CYCLES-1);
            TimerMatchSet(TIMER0_BASE, TIMER_B, TIMER_CYCLES-1);

            // PWM + RESET C
            TimerMatchSet(TIMER2_BASE, TIMER_A, TIMER_CYCLES-1);
            TimerMatchSet(TIMER0_BASE, TIMER_A, 0);
            break;
        case 2: // H1, H2, H3 = 1, 0, 0
            // PWM + RESET A
            TimerMatchSet(TIMER3_BASE, TIMER_A, match_point);
            TimerMatchSet(TIMER3_BASE, TIMER_B, TIMER_CYCLES-1);

            // PWM + RESET B
            TimerMatchSet(TIMER2_BASE, TIMER_B, match_point);
            TimerMatchSet(TIMER0_BASE, TIMER_B, 0);

            // PWM + RESET C
            TimerMatchSet(TIMER2_BASE, TIMER_A, TIMER_CYCLES-1);
            TimerMatchSet(TIMER0_BASE, TIMER_A, TIMER_CYCLES-1);
            break;
        case 3: // H1, H2, H3 = 1, 1, 0
            // PWM + RESET A
            TimerMatchSet(TIMER3_BASE, TIMER_A, TIMER_CYCLES-1);
            TimerMatchSet(TIMER3_BASE, TIMER_B, 0);

            // PWM + RESET B
            TimerMatchSet(TIMER2_BASE, TIMER_B, match_point);
            TimerMatchSet(TIMER0_BASE, TIMER_B, TIMER_CYCLES-1);

            // PWM + RESET C
            TimerMatchSet(TIMER2_BASE, TIMER_A, TIMER_CYCLES-1);
            TimerMatchSet(TIMER0_BASE, TIMER_A, TIMER_CYCLES-1);
            break;
        case 4: // H1, H2, H3 = 0, 1, 0
            // PWM + RESET A
            TimerMatchSet(TIMER3_BASE, TIMER_A, TIMER_CYCLES-1);
            TimerMatchSet(TIMER3_BASE, TIMER_B, TIMER_CYCLES-1);

            // PWM + RESET B
            TimerMatchSet(TIMER2_BASE, TIMER_B, match_point);
            TimerMatchSet(TIMER0_BASE, TIMER_B, TIMER_CYCLES-1);

            // PWM + RESET C
            TimerMatchSet(TIMER2_BASE, TIMER_A, match_point);
            TimerMatchSet(TIMER0_BASE, TIMER_A, 0);
            break;
        case 5: // H1, H2, H3 = 0, 1, 1
            // PWM + RESET A
            TimerMatchSet(TIMER3_BASE, TIMER_A, TIMER_CYCLES-1);
            TimerMatchSet(TIMER3_BASE, TIMER_B, TIMER_CYCLES-1);

            // PWM + RESET B
            TimerMatchSet(TIMER2_BASE, TIMER_B, TIMER_CYCLES-1);
            TimerMatchSet(TIMER0_BASE, TIMER_B, 0);

            // PWM + RESET C
            TimerMatchSet(TIMER2_BASE, TIMER_A, match_point);
            TimerMatchSet(TIMER0_BASE, TIMER_A, TIMER_CYCLES-1);
            break;
        default: // Motor shouldn't reach here in non-faulty state
            faulty_motor = true;
            break;
    }
}

