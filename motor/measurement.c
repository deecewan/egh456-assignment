#include <stdint.h>
#include "stdbool.h"
#include "speed.h"
#include "current.h"
#include "temperature.h"

double GetFilteredSpeed();
double GetFilteredTemperature();
double GetFilteredCurrentValue();

// NOTE: TIMERS 0, 2 and 3 ARE BEING USED AS PWM OUTPUTS.
// HENCE, DO NOT USE THEM AT ALL FOR FILTERING HERE.

/*
 * Collects 6 samples of motor speed per 6 milliseconds interval
 * (single sample is collected by calling GetMotorSpeed() function
 * but for now, just make a random sample value for testing purposes)
 * and then uses those samples to return an averaged sample value.
 */
double GetFilteredSpeed() {
    ;
}

/*
 * Collects 3 samples of motor temperature per 1.5 seconds interval
 * (single sample is collected by calling GetTemperature() function
 * but for now, just make a random sample value for testing purposes)
 * and then uses those samples to return an averaged sample value.
 */
double GetFilteredTemperature() {
    ;
}

/*
 * Collects 5 samples of motor temperature per 5 millisecond intervals
 * (single sample is collected by calling GetCurrentValue() function
 * but for now, just make a random sample value for testing purposes)
 * and then uses those samples to return a median sample value.
 */
double GetFilteredCurrentValue() {
    ;
}
