#include <stdint.h>
#include "constants.h"
#include "state.h"

/**
 * Controls whether on not the motor is turned on
 */
volatile MOTOR_POWER motor_power = OFF;
MOTOR_POWER get_motor_power() {
  return motor_power;
}

void set_motor_power(MOTOR_POWER power) {
   motor_power = power;
}

void toggle_motor_power() {
    motor_power ^= ON;
}

/**
 * Tracks the state of the motor at any given point in time
 */
volatile MOTOR_STATE motor_state = IDLE;
MOTOR_STATE get_motor_state() {
    return motor_state;
}

void set_motor_state(MOTOR_STATE state) {
    motor_state = state;
}

/**
 * Controls the speed at which the motor should be rotating
 * Set in revs per minute
 */
volatile uint32_t motor_speed = 0;

uint32_t get_motor_speed() {
  return motor_speed;
}

void set_motor_speed(uint32_t speed) {
  motor_speed = speed;
}

/**
 * Controls the maximum current draw allowed by the motors
 * before forcing shutdown
 */
volatile uint32_t current_limit = 0;

uint32_t get_current_limit() {
  return current_limit;
}

void set_current_limit(uint32_t limit) {
  current_limit = limit;
}

/**
 * Controls the maximum temperature allowed by the motors
 * before forcing shutdown
 */
volatile uint32_t temp_limit = 0;

uint32_t get_temp_limit() {
  return temp_limit;
}

void set_temp_limit(uint32_t limit) {
  temp_limit = limit;
}

/**
 * Keeps track of how long the program has been running
 */
static volatile uint32_t run_time = 0;

uint32_t get_run_time() {
    return run_time;
}

void reset_run_time() {
    run_time = 0;
}

void increment_run_time() {
    run_time++;
}

/**
 * All the values for motor speed that we're current storing
 * We also keep the last value to make it easier to append to the end
 */

static uint32_t listMotorSpeed[LIST_ITEM_COUNT] = { 0, 30, 60, 80, 100, 60, 160, 280, 240, 180 };
static uint32_t listMotorSpeedLargest = 280;
static uint32_t listCurrent[LIST_ITEM_COUNT] = { 10, 10, 10, 10, 10, 10, 10, 10, 10, 10 };
static uint32_t listCurrentLargest = 140;
static uint32_t listTemp[LIST_ITEM_COUNT] = { 20, 20, 20, 20, 20, 20, 20, 20, 20, 20 };
static uint32_t listTempLargest = 0;

void append(uint32_t *list, uint32_t value) {
    for (uint8_t i = 1; i < LIST_ITEM_COUNT; i++) {
        list[i - 1] = list[i];
    }
    list[LIST_ITEM_COUNT - 1] = value;
}

void appendToMotorSpeed(uint32_t value) {
    if (value > listMotorSpeedLargest) {
        listMotorSpeedLargest = value;
    }
    append(listMotorSpeed, value);

}

void appendToCurrent(uint32_t value) {
    if (value > listCurrentLargest) {
        listCurrentLargest = value;
    }
    append(listCurrent, value);
}

void appendToTemp(uint32_t value) {
    if (value > listTempLargest) {
        listTempLargest = value;
    }
    append(listTemp, value);
}

uint32_t *get_motor_speed_list() {
    return listMotorSpeed;
}

uint32_t get_largest_motor_speed() {
    return listMotorSpeedLargest;
}

uint32_t *get_current_list() {
    return listCurrent;
}

uint32_t get_largest_current() {
    return listCurrentLargest;
}

uint32_t *get_temp_list() {
    return listTemp;
}

uint32_t get_largest_temp() {
    return listTempLargest;
}
