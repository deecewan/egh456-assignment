//*****************************************************************************
//
// motorLib.h - Defines and Macros for driving motor for EGH456 motor kits.
//
// Setup:
//       1. Please make sure that the GPIO pins for the reset lines are configured as outputs
//          These include:
//               RESET_A - Port A pin 7
//               RESET_B - Port L pin 5
//               RESET_C - Port L pin 4
//          Please ensure these GPIO lines are configured before using this library
//
//      2. Please configure the timers 2 & 3 as PWM mode
//         These include:
//               MOTOR_A - Timer 3 in timer PWM mode
//               MOTOR_B - Timer 2 as half timer in PWM mode Timer B
//               MOTOR_C - Timer 2 as half timer in PWM mode Timer A
//          Please see lecture examples to setup timer modules in PWM mode
//
//Usage:
//
//      This library has been created simply to drive the lines of the motor safely.
//      Simply use the driveMotor() function to set the appropriate lines based on
//      phase indicated by the hall effect sensors (to be determined by you).
//
//      e.g.   driveMotor(0b011, 10)
//
//             Will set the motor lines for the motor phase corresponding to
//             Hall effect sensor states - H3=0,H2=1,H1=1 and will load 10 into
//             the PWM timer of the correct motor based on the phase.
//
//      e.g.   driveMotor(5, 500)
//
//             where 5 corresponds to the phase (0b101) - H3=1,H2=0,H1=1 and will
//             load 500 in the PWM timer of the correct motor based on the phase
//
//      You should integrate the driveMotor function into your application
//
//*****************************************************************************


#ifndef __MOTORLIB_H__
#define __MOTORLIB_H__

//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif


#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/timer.h"
#include "driverlib/gpio.h"

//Motor Timer defines (Intentional "," in define)
#define MOTOR_A_TIMER TIMER3_BASE, TIMER_A
#define MOTOR_B_TIMER TIMER2_BASE, TIMER_B
#define MOTOR_C_TIMER TIMER2_BASE, TIMER_A

// Possible Phases are
#define PHASE_001 0b001 //PHASE 1 - bitwise H1=1 H2=0 H3=0
#define PHASE_010 0b010 //PHASE 2 - bitwise H1=0 H2=1 H3=0
#define PHASE_011 0b011 //PHASE 3 - bitwise H1=1 H2=1 H3=0
#define PHASE_100 0b100 //PHASE 4 - bitwise H1=0 H2=0 H3=1
#define PHASE_101 0b101 //PHASE 5 - bitwise H1=1 H2=0 H3=1
#define PHASE_110 0b110 //PHASE 6 - bitwise H1=0 H2=1 H3=1

// RESET lines
#define EGH456_RESET_A GPIO_PORTA_BASE, GPIO_PIN_7
#define EGH456_RESET_B GPIO_PORTL_BASE, GPIO_PIN_5
#define EGH456_RESET_C GPIO_PORTL_BASE, GPIO_PIN_4

//*****************************************************************************
//
// Prototypes for the APIs.
//
//*****************************************************************************

//
//  driveMotor function
//          Arguments:
//              uint8_t phase - is the bitwise phase of the hall effect sensors (see examples above)
//              int pwm_val - is the integer value loaded into the PWM timer and determines the duty cycle
//                            depending on how the PWM timer mode is configured.
//                            Can be calculated based on desired duty cycle percentage
//                                   pwm_val = PWM_TOP_VALUE * pwm_percent / 100
//
extern void driveMotor(uint8_t phase, int32_t pwm_val);

// Don't need to use, called within driveMotor function
extern void pwmSet(uint32_t timer_base, uint32_t timer, int32_t pwm_val);

// Don't need to use, called within driveMotor function
extern void resetLines(uint8_t val_a,
                uint8_t val_b,
                uint8_t val_c);



//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif // __MOTORLIB_H__
