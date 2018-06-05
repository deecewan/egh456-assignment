
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/timer.h"
#include "driverlib/gpio.h"

#include "motorLib.h"


void resetLines(uint8_t val_a, uint8_t val_b, uint8_t val_c)
{
    GPIOPinWrite(EGH456_RESET_A, val_a);
    GPIOPinWrite(EGH456_RESET_B, val_b);
    GPIOPinWrite(EGH456_RESET_C, val_c);
}

void pwmSet(uint32_t timer_base, uint32_t timer, int32_t pwm_val)
{
//    uint32_t pwm_val = 999 * pwm_percent / 100;
    TimerMatchSet(timer_base, timer, pwm_val);
}


void driveMotor(uint8_t phase, int32_t pwm_val)
{
    switch(phase)
        {

        case PHASE_001:
            resetLines(0xff, 0, 0xff);
            pwmSet(MOTOR_A_TIMER, pwm_val);
            pwmSet(MOTOR_C_TIMER, 0.f);
            break;

        case PHASE_010:
            resetLines(0xff, 0xff, 0);
            pwmSet(MOTOR_B_TIMER, pwm_val);
            pwmSet(MOTOR_A_TIMER, 0.f);
            break;

        case PHASE_011:
            resetLines(0, 0xff, 0xff);
            pwmSet(MOTOR_B_TIMER, pwm_val);
            pwmSet(MOTOR_C_TIMER, 0.f);
            break;

        case PHASE_100:
            resetLines(0, 0xff, 0xff);
            pwmSet(MOTOR_C_TIMER, pwm_val);
            pwmSet(MOTOR_B_TIMER, 0.f);
            break;

        case PHASE_101:
            resetLines(0xff, 0xff, 0);
            pwmSet(MOTOR_A_TIMER, pwm_val);
            pwmSet(MOTOR_B_TIMER, 0.f);
            break;

        case PHASE_110:
            resetLines(0xff, 0, 0xff);
            pwmSet(MOTOR_C_TIMER, pwm_val);
            pwmSet(MOTOR_A_TIMER, 0.f);
            break;
        }
}


