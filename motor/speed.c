#include <stdint.h>
#include "stdbool.h"
#include <driverlib/sysctl.h>
#include <driverlib/pin_map.h>
#include <driverlib/gpio.h>
#include <driverlib/timer.h>
#include <driverlib/pwm.h>
#include <inc/hw_memmap.h>
#include <xdc/runtime/System.h>

// General PWM range 25-30 kHz
#define NUM_STATES 6
#define SECONDS_IN_MINUTE 60
#define MAX_SPEED 60000 // Need to determine speed at which motor reaches terminal velocity (no it doesn't keep going until it flies off)

#define MOTOR_VREF 3.3
#define FAULT_VOLTS MOTOR_VREF // 11 indicates everything is normal, 00 shutdown, all else is warning
#define MAX_PWM_V 3.6
#define MIN_PWM_V 2.0
#define SAMPLING_FREQUENCY 500000 // 500 kHz
// Have a 100ns PWM pulse

static const uint8_t HALL_SENSOR_STATES[NUM_STATES] = {1, 101, 100, 110, 10, 11};
static const uint16_t TIMER_CYCLES = 400; // 30 kHz = Desired PWM wave frequency

static uint8_t checkpoint_state, initial_state; // can only be changed through publicly available functions
static double current_speed = 0, desired_speed = 0, duty_cycle = 0, Kp = 0, Ki = 0;
bool run_motor = false;

/*
 * Function Prototypes.
 */
uint8_t ConnectWithHallSensors();
double GetMotorSpeed(double seconds);
void ConnectWithMotor();
void RunMotor();
void SetMotorSpeed(int speed);
void StopMotor();
static uint8_t GetCurrentHallState();

/*
 * Initializes connection to read from all three hall sensors and fault lines.
 *
 * Output: Hall sensor readings so that the calling function can check for any faults.
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
    initial_state = GetCurrentHallState();
    checkpoint_state = initial_state;
    return checkpoint_state;
}

/*
 * Measures distance traveled from previous reference point to calculate motor
 * speed in revolutions per minute (RPM).
 *
 * Inputs: Time intervals at which this function is being called.
 *
 * Outputs: one of seven possible motor speeds in revolutions per minute format.
 *
 * Assumption: Only one rotation could have happened at most since this function
 * was last called.
 */
double GetMotorSpeed(double seconds) {
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

    current_speed = (difference / ((double)NUM_STATES)) * (SECONDS_IN_MINUTE / seconds);
    checkpoint_state = current_state;
    return current_speed;
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
    GPIOPinTypeTimer(GPIO_PORTM_BASE, GPIO_PIN_0);
    GPIOPinTypeTimer(GPIO_PORTM_BASE, GPIO_PIN_1);
    GPIOPinTypeTimer(GPIO_PORTM_BASE, GPIO_PIN_2);
    GPIOPinTypeTimer(GPIO_PORTA_BASE, GPIO_PIN_7);
    GPIOPinTypeTimer(GPIO_PORTL_BASE, GPIO_PIN_4);
    GPIOPinTypeTimer(GPIO_PORTL_BASE, GPIO_PIN_5);

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
 * Starts and maintains the motion of the motor with a default starting speed.
 */
void RunMotor() {
    uint8_t current_state = initial_state;
    uint16_t match_point = TIMER_CYCLES-1;

    ConnectWithHallSensors();
    ConnectWithMotor();
    TimerEnable(TIMER0_BASE, TIMER_BOTH);
    TimerEnable(TIMER2_BASE, TIMER_BOTH);
    TimerEnable(TIMER3_BASE, TIMER_BOTH);
    //TimerSynchronize(TIMER0_BASE, (TIMER_0A_SYNC | TIMER_0B_SYNC | TIMER_2A_SYNC | TIMER_2B_SYNC | TIMER_3A_SYNC | TIMER_3B_SYNC));

    duty_cycle = 0.1;
    run_motor = true;

    // Try to understand code below at your own risk!
    // HALL_SENSOR_STATES[NUM_STATES] = {001, 101, 100, 110, 010, 011};
    // 3A = PWM_A, 2B = PWM_B = 2A = PWM_C
    while (run_motor) {
        match_point = ((uint16_t)(TIMER_CYCLES - (duty_cycle * TIMER_CYCLES)));

        switch (current_state) {
            case 0:
                // PWM + RESET A
                TimerControlLevel(TIMER3_BASE, TIMER_A, false);
                TimerMatchSet(TIMER3_BASE, TIMER_A, TIMER_CYCLES-1);
                TimerMatchSet(TIMER3_BASE, TIMER_B, TIMER_CYCLES-1);

                // PWM + RESET B
                TimerControlLevel(TIMER2_BASE, TIMER_B, true);
                TimerMatchSet(TIMER2_BASE, TIMER_B, match_point);
                TimerMatchSet(TIMER0_BASE, TIMER_B, 0);

                // PWM + RESET C
                TimerControlLevel(TIMER2_BASE, TIMER_A, false);
                TimerMatchSet(TIMER2_BASE, TIMER_A, match_point);
                TimerMatchSet(TIMER0_BASE, TIMER_A, 0);
                break;
            case 1:
                // PWM + RESET A
                TimerControlLevel(TIMER3_BASE, TIMER_A, false);
                TimerMatchSet(TIMER3_BASE, TIMER_A, match_point);
                TimerMatchSet(TIMER3_BASE, TIMER_B, 0);

                // PWM + RESET B
                TimerControlLevel(TIMER2_BASE, TIMER_B, true);
                TimerMatchSet(TIMER2_BASE, TIMER_B, match_point);
                TimerMatchSet(TIMER0_BASE, TIMER_B, 0);

                // PWM + RESET C
                TimerControlLevel(TIMER3_BASE, TIMER_A, false);
                TimerMatchSet(TIMER2_BASE, TIMER_A, TIMER_CYCLES - 1);
                TimerMatchSet(TIMER0_BASE, TIMER_A, TIMER_CYCLES - 1);
                break;
            case 2:
                // PWM + RESET A
                TimerControlLevel(TIMER3_BASE, TIMER_A, false);
                TimerMatchSet(TIMER3_BASE, TIMER_A, match_point);
                TimerMatchSet(TIMER3_BASE, TIMER_B, 0);

                // PWM + RESET B
                TimerControlLevel(TIMER2_BASE, TIMER_B, false);
                TimerMatchSet(TIMER2_BASE, TIMER_B, TIMER_CYCLES - 1);
                TimerMatchSet(TIMER0_BASE, TIMER_B, TIMER_CYCLES - 1);

                // PWM + RESET C
                TimerControlLevel(TIMER2_BASE, TIMER_A, true);
                TimerMatchSet(TIMER2_BASE, TIMER_A, match_point);
                TimerMatchSet(TIMER0_BASE, TIMER_A, 0);
                break;
            case 3:
                // PWM + RESET A
                TimerControlLevel(TIMER3_BASE, TIMER_A, false);
                TimerMatchSet(TIMER3_BASE, TIMER_A, TIMER_CYCLES - 1);
                TimerMatchSet(TIMER3_BASE, TIMER_B, TIMER_CYCLES - 1);

                // PWM + RESET B
                TimerControlLevel(TIMER2_BASE, TIMER_B, false);
                TimerMatchSet(TIMER2_BASE, TIMER_B, match_point);
                TimerMatchSet(TIMER0_BASE, TIMER_B, 0);

                // PWM + RESET C
                TimerControlLevel(TIMER2_BASE, TIMER_A, true);
                TimerMatchSet(TIMER2_BASE, TIMER_A, match_point);
                TimerMatchSet(TIMER0_BASE, TIMER_A, 0);
                break;
            case 4:
                // PWM + RESET A
                TimerControlLevel(TIMER3_BASE, TIMER_A, true);
                TimerMatchSet(TIMER3_BASE, TIMER_A, match_point);
                TimerMatchSet(TIMER3_BASE, TIMER_B, 0);

                // PWM + RESET B
                TimerControlLevel(TIMER2_BASE, TIMER_B, false);
                TimerMatchSet(TIMER2_BASE, TIMER_B, match_point);
                TimerMatchSet(TIMER0_BASE, TIMER_B, 0);

                // PWM + RESET C
                TimerControlLevel(TIMER2_BASE, TIMER_A, false);
                TimerMatchSet(TIMER2_BASE, TIMER_A, TIMER_CYCLES - 1);
                TimerMatchSet(TIMER0_BASE, TIMER_A, TIMER_CYCLES - 1);
                break;
            case 5:
                // PWM + RESET A
                TimerControlLevel(TIMER3_BASE, TIMER_A, true);
                TimerMatchSet(TIMER3_BASE, TIMER_A, match_point);
                TimerMatchSet(TIMER3_BASE, TIMER_B, 0);

                // PWM + RESET B
                TimerControlLevel(TIMER2_BASE, TIMER_B, false);
                TimerMatchSet(TIMER2_BASE, TIMER_B, TIMER_CYCLES - 1);
                TimerMatchSet(TIMER0_BASE, TIMER_B, TIMER_CYCLES - 1);

                // PWM + RESET C
                TimerControlLevel(TIMER2_BASE, TIMER_A, false);
                TimerMatchSet(TIMER2_BASE, TIMER_A, match_point);
                TimerMatchSet(TIMER0_BASE, TIMER_A, 0);
                break;
            default: // Motor shouldn't reach here in non-faulty state
                //StopMotor();
                break;
        }

        current_state = GetCurrentHallState();
        System_printf("%d\n", current_state);
        System_flush();

        duty_cycle = duty_cycle + 0.001;
        if (duty_cycle >= 1) {
            duty_cycle = 1;
        }
    }
}
/*
            case 0:
                TimerControlLevel(TIMER3_BASE, TIMER_A, false);
                TimerMatchSet(TIMER3_BASE, TIMER_A, TIMER_CYCLES);

                TimerMatchSet(TIMER2_BASE, TIMER_B, (uint16_t)(TIMER_CYCLES - (duty_cycle * TIMER_CYCLES)));
                TimerControlLevel(TIMER2_BASE, TIMER_B, true);
                TimerMatchSet(TIMER2_BASE, TIMER_A, (uint16_t)(TIMER_CYCLES - (duty_cycle * TIMER_CYCLES)));
                TimerControlLevel(TIMER2_BASE, TIMER_A, false);
                break;
            case 1:
                TimerMatchSet(TIMER3_BASE, TIMER_A, (uint16_t)(TIMER_CYCLES - (duty_cycle * TIMER_CYCLES)));
                TimerControlLevel(TIMER3_BASE, TIMER_A, false);
                TimerMatchSet(TIMER2_BASE, TIMER_B, (uint16_t)(TIMER_CYCLES - (duty_cycle * TIMER_CYCLES)));
                TimerControlLevel(TIMER2_BASE, TIMER_B, true);
                TimerMatchSet(TIMER2_BASE, TIMER_A, TIMER_CYCLES);
                TimerControlLevel(TIMER2_BASE, TIMER_A, false);
                break;
            case 2:
                TimerMatchSet(TIMER3_BASE, TIMER_A, (uint16_t)(TIMER_CYCLES - (duty_cycle * TIMER_CYCLES)));
                TimerControlLevel(TIMER3_BASE, TIMER_A, false);
                TimerMatchSet(TIMER2_BASE, TIMER_B, TIMER_CYCLES);
                TimerControlLevel(TIMER2_BASE, TIMER_B, false);
                TimerMatchSet(TIMER2_BASE, TIMER_A, (uint16_t)(TIMER_CYCLES - (duty_cycle * TIMER_CYCLES)));
                TimerControlLevel(TIMER2_BASE, TIMER_A, true);
                break;
            case 3:
                TimerMatchSet(TIMER3_BASE, TIMER_A, TIMER_CYCLES);
                TimerControlLevel(TIMER3_BASE, TIMER_A, false);
                TimerMatchSet(TIMER2_BASE, TIMER_B, (uint16_t)(TIMER_CYCLES - (duty_cycle * TIMER_CYCLES)));
                TimerControlLevel(TIMER2_BASE, TIMER_B, false);
                TimerMatchSet(TIMER2_BASE, TIMER_A, (uint16_t)(TIMER_CYCLES - (duty_cycle * TIMER_CYCLES)));
                TimerControlLevel(TIMER2_BASE, TIMER_A, true);
                break;
            case 4:
                TimerControlLevel(TIMER3_BASE, TIMER_A, true);
                TimerMatchSet(TIMER3_BASE, TIMER_A, (uint16_t)(TIMER_CYCLES - (duty_cycle * TIMER_CYCLES)));
                TimerControlLevel(TIMER3_BASE, TIMER_A, true);
                TimerMatchSet(TIMER2_BASE, TIMER_B, (uint16_t)(TIMER_CYCLES - (duty_cycle * TIMER_CYCLES)));
                TimerControlLevel(TIMER2_BASE, TIMER_B, false);
                TimerMatchSet(TIMER2_BASE, TIMER_A, TIMER_CYCLES);
                TimerControlLevel(TIMER2_BASE, TIMER_A, false);
                break;
            case 5:
                TimerMatchSet(TIMER3_BASE, TIMER_A, (uint16_t)(TIMER_CYCLES - (duty_cycle * TIMER_CYCLES)));
                TimerMatchSet(TIMER2_BASE, TIMER_B, TIMER_CYCLES);
                TimerControlLevel(TIMER2_BASE, TIMER_B, false);
                TimerMatchSet(TIMER2_BASE, TIMER_A, (uint16_t)(TIMER_CYCLES - (duty_cycle * TIMER_CYCLES)));
                TimerControlLevel(TIMER2_BASE, TIMER_A, false);
                break;
            default:
                StopMotor();
                run_motor = false;
                break;
        }
*/
/*
    TimerLoadSet(TIMER2_BASE, TIMER_BOTH, SysCtlClockGet());
    TimerMatchSet(TIMER2_BASE, TIMER_BOTH, 0); // PWM
    TimerLoadSet(TIMER3_BASE, TIMER_BOTH, SysCtlClockGet());
    TimerMatchSet(TIMER3_BASE, TIMER_BOTH, 0); // PWM
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

/*
 * Brings the motor speed up or down to the desired speed by using a
 * safe acceleration or deceleration margin until it has reached
 * desired speed.
 */
void SetMotorSpeed(int speed) {
    // Error Handling for unsupported speed demands
    if (speed < 0) {
        return;
    } else if (speed >= MAX_SPEED) {
        desired_speed = MAX_SPEED;
    }


}

/*
 * Brings the motor to a stopping and when the speed is low enough, stops the motor itself.
 */
void StopMotor() {
    SetMotorSpeed(0);
    while(current_speed > 0);
    run_motor = false;
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
