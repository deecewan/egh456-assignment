#include <stdint.h>
#include "constants.h"

/**
 * Controls whether on not the motor is turned on
 */
volatile MOTOR_STATE motor_state = OFF;
MOTOR_STATE get_motor_state() {
  return motor_state;
}

void set_motor_state(MOTOR_STATE state) {
  motor_state = state;
}

void toggle_motor_state() {
  motor_state ^= ON;
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
