#include <stdint.h>
#include "stdbool.h"
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Semaphore.h>
#include "speed.h"
#include "current.h"
#include "temperature.h"

#define MIN_TA_ALLOWED -40 // For motor to work according to its datasheet
#define MAX_TA_ALLOWED 85
#define LOW_TEMP_LIMIT -20 // Minimum temperature that can be detected for object
#define UPPER_TEMP_LIMIT 200 // Maximum temperature that can be detected for object
#define SPEED_SAMPLES 5
#define CURRENT_SAMPLES 5
#define TEMPERATURE_SAMPLES 3

void TakeMeasurements();
void MeasureTemperature();
bool ReadingsReady();
double GetFilteredSpeed();
double GetFilteredTemperature();
double GetFilteredCurrentValue();

static uint8_t Ps = 0, Pt = 0, Pc = 0; // keep track of the next value to be modified
static bool currents_ready = false, speeds_ready = false, temperatures_ready = false;
static double recent_speeds[SPEED_SAMPLES] = {0, 0, 0, 0, 0};
static double recent_temperatures[TEMPERATURE_SAMPLES] = {0, 0, 0};
static double recent_currents[CURRENT_SAMPLES] = {0, 0, 0, 0, 0};

// NOTE: TIMERS 0, 2 and 3 ARE BEING USED AS PWM OUTPUTS.
// HENCE, DO NOT USE THEM AT ALL FOR FILTERING HERE.

/*
 *  Starts the process of sampling motor current and speed readings.
 *
 *  Outputs: 0 if the process is properly ended, -1 if a fault has occured.
 */
void TakeMeasurements() {
    recent_speeds[Ps] = GetMotorSpeed();
    recent_currents[Pc] = GetCurrentValue();

    // Move to next array element for overwriting value
    ++Ps;
    ++Pc;

    // Ensure pointer value moves back to start of array once it reaches end
    if (Ps > SPEED_SAMPLES-1) {
        speeds_ready = true; // 5 samples have been taken so speeds are ready to be plotted
        Ps = 0;
    }

    if (Pc > CURRENT_SAMPLES-1) {
        currents_ready = true; // 5 samples have been taken so currents are ready to be plotted
        Pc = 0;
    }
}

/*
 *  Starts the process of sampling motor temperature readings.
 */
void MeasureTemperature() {
    ++Pt;
    recent_temperatures[Pc] = GetCurrentValue();
    if (Pt > TEMPERATURE_SAMPLES-1) {
        temperatures_ready = true;
        Pt = 0;
    }
}

/*
 * Signals whether enough initial readings have been taken for graphs to be plotted.
 */
bool ReadingsReady() {
    return (speeds_ready && temperatures_ready && currents_ready);
}

/*
 * Collects 6 samples of motor speed per 6 milliseconds interval
 * (single sample is collected by calling GetMotorSpeed() function
 * but for now, just make a random sample value for testing purposes)
 * and then uses those samples to return an averaged sample value.
 */
double GetFilteredSpeed() {
    uint8_t i = 0;
    double sum = 0;

    for (i = 0; i < SPEED_SAMPLES; i++) {
        sum += recent_speeds[i];
    }

    return (sum / SPEED_SAMPLES);
}

/*
 * Collects 3 samples of motor temperature per 1.5 seconds interval
 * (single sample is collected by calling GetTemperature() function
 * but for now, just make a random sample value for testing purposes)
 * and then uses those samples to return an averaged sample value.
 */
double GetFilteredTemperature() {
    uint8_t i = 0;
    double sum = 0;

    for (i = 0; i < TEMPERATURE_SAMPLES; i++) {
        sum += recent_temperatures[i];
    }

    return (sum / TEMPERATURE_SAMPLES);
}

/*
 * Collects 5 samples of motor temperature per 5 millisecond intervals
 * (single sample is collected by calling GetCurrentValue() function
 * but for now, just make a random sample value for testing purposes)
 * and then uses those samples to return an averaged sample value.
 */
double GetFilteredCurrentValue() {
    uint8_t i = 0;
    double sum = 0;

    for (i = 0; i < CURRENT_SAMPLES; i++) {
        sum += recent_currents[i];
    }

    return (sum / CURRENT_SAMPLES);
}
