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

void StartTakingMeasurements();
void StopTakingMeasurements();
bool ReadingsReady();
double GetFilteredSpeed();
double GetFilteredTemperature();
double GetFilteredCurrentValue();

static bool measurements_ready = false, take_measurements = false;
static double recent_speeds[5] = {0, 0, 0, 0, 0};
static double recent_temperatures[3] = {0, 0, 0};
static double recent_currents[5] = {0, 0, 0, 0, 0};
static Semaphore_Handle semHandle;

static uint8_t Ps = 0, Pt = 0, Pc = 0; // keep track of the next value to be modified
static bool currents_ready = false, speeds_ready = false, temperatures_ready = false;

// NOTE: TIMERS 0, 2 and 3 ARE BEING USED AS PWM OUTPUTS.
// HENCE, DO NOT USE THEM AT ALL FOR FILTERING HERE.

void MeasurementInit() {
    // Initialize semaphore lock
    Semaphore_Params semParams;
    Semaphore_Struct semStruct;
    Semaphore_Params_init(&semParams);
    semParams.mode = Semaphore_Mode_BINARY;
    Semaphore_construct(&semStruct, 0, &semParams);
    semHandle = Semaphore_handle(&semStruct);
    Semaphore_post(semHandle);
}

/*
 *  Starts and continues the process of getting motor readings unless signalled to stop.
 *
 *  Outputs: 0 if the process is properly ended, -1 if a fault has occured.
 */
void TakeMeasurements() {
    // Update recent values being stored
    //Semaphore_pend(semHandle, BIOS_WAIT_FOREVER);
    recent_speeds[Ps] = GetMotorSpeed(0.001);
    recent_currents[Pc] = GetCurrentValue();
    //recent_temperatures[Pt] = GetTemperature();
    //Semaphore_post(semHandle);

    // Move to next array element for overwriting value
    ++Ps;
    ++Pt;
    ++Pc;

    // Ensure pointer value moves back to start of array once it reaches end
    if (Ps > 4) {
        speeds_ready = true; // 5 samples have been taken so speeds are ready to be plotted
        Ps = 0;
    }

    if (Pt > 2) {
        temperatures_ready = true; // 2 samples have been taken so temperatures are ready to be plotted
        Pt = 0;
    }

    if (Pc > 4) {
        currents_ready = true; // 5 samples have been taken so currents are ready to be plotted
        Pc = 0;
    }

    measurements_ready = (speeds_ready && temperatures_ready && currents_ready);
}

/*
 * Signals whether enough initial readings have been taken for graphs to be plotted.
 */
bool ReadingsReady() {
    return (speeds_ready && temperatures_ready && currents_ready);
}

/*
 * Stops the entire motor measurements process.
 */
void StopTakingMeasurements() {
    take_measurements = false;
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

    //Semaphore_pend(semHandle, BIOS_WAIT_FOREVER);
    for (i = 0; i < 5; i++) {
        sum += recent_speeds[i];
    }
    //Semaphore_post(semHandle);

    return (sum / 5);
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

    //Semaphore_pend(semHandle, BIOS_WAIT_FOREVER);
    for (i = 0; i < 3; i++) {
        sum += recent_temperatures[i];
    }
    //Semaphore_post(semHandle);

    return (sum / 3);
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

    //Semaphore_pend(semHandle, BIOS_WAIT_FOREVER);
    for (i = 0; i < 5; i++) {
        sum += recent_currents[i];
    }
    //Semaphore_post(semHandle);

    return (sum / 5);
}
