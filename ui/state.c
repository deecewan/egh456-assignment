#include "constants.h"

MOTOR_STATE motor_state = OFF;

MOTOR_STATE get_motor_state() {
  return motor_state;
}

void set_motor_state(MOTOR_STATE state) {
  motor_state = state;
}

void toggle_motor_state() {
  motor_state ^= ON;
}

