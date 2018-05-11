#include <stdint.h>
#include "stdbool.h"
#include <driverlib/sysctl.h>
#include <driverlib/pin_map.h>
#include <driverlib/gpio.h>
#include <driverlib/timer.h>
#include <driverlib/pwm.h>
#include <inc/hw_memmap.h>
#include <xdc/runtime/System.h>

#define NUM_STATES 6
#define SECONDS_IN_MINUTE 60
#define MAX_SPEED 6000 // Max speed at which present motor can spin in revolutions per minute (RPM)
#define T_CPU_CLOCK_SPEED 120000000
#define SAMPLING_FREQUENCY 500000 // Fixed PWM frequency at which motor performs best
/*
#define MOTOR_VREF 3.3
#define FAULT_VOLTS MOTOR_VREF // 11 indicates everything is normal, 00 shutdown, all else is warning
#define MAX_PWM_V 3.6
#define MIN_PWM_V 2.0
*/

static const uint8_t HALL_SENSOR_STATES[NUM_STATES] = {1, 101, 100, 110, 10, 11};
static const uint16_t TIMER_CYCLES = T_CPU_CLOCK_SPEED / SAMPLING_FREQUENCY;

static uint8_t checkpoint_state, initial_state;
static double current_speed = 0, desired_speed = 0, duty_cycle = 0, error_sum = 0;
static double Kp = 0, Ki = 0; // Will be determined through trial and error
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
    duty_cycle = 0.1;
    run_motor = true;

    ConnectWithHallSensors();
    ConnectWithMotor();
    TimerEnable(TIMER0_BASE, TIMER_BOTH);
    TimerEnable(TIMER2_BASE, TIMER_BOTH);
    TimerEnable(TIMER3_BASE, TIMER_BOTH);

    // To avoid scrolling way above:
    // HALL_SENSOR_STATES[NUM_STATES] = {001, 101, 100, 110, 010, 011};
    // 3A = PWM_A, 2B = PWM_B = 2A = PWM_C
    while (run_motor) {
        //TimerSynchronize(TIMER0_BASE, (TIMER_0A_SYNC | TIMER_0B_SYNC | TIMER_2A_SYNC | TIMER_2B_SYNC | TIMER_3A_SYNC | TIMER_3B_SYNC));
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
        if (duty_cycle >= 0.97) { // Have a 100ns PWM pulse as specified in datasheet
            duty_cycle = 0.97;
        }
    }
}

/*
 * Brings the motor speed up or down to the desired speed by using a
 * safe acceleration or deceleration margin until it has reached
 * desired speed.
 */
void SetMotorSpeed(int speed) {
    double error = 0;

    // User input error handling for unsupported speed demands
    if (speed < 0) {
        return;
    } else if (speed >= MAX_SPEED) {
        desired_speed = MAX_SPEED;
    } else {
        desired_speed = speed;
    }

    // Use PI feedback controller to modify speed as suggested in week 7 lecture
    error = desired_speed - current_speed;
    error_sum += error;
    duty_cycle += (Kp * error + Ki * error_sum);
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

    return 6; // Reading to indicate hardware fault
}
