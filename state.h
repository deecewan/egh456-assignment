#ifndef STATE_H
#define STATE_H
#include <stdint.h>
#include "constants.h"

MOTOR_POWER get_motor_power();
void set_motor_power(MOTOR_POWER power);
void toggle_motor_power();

MOTOR_STATE get_motor_state();
void set_motor_state(MOTOR_STATE state);

uint32_t get_motor_speed();
void set_motor_speed(uint32_t speed);

uint32_t get_current_limit();
void set_current_limit(uint32_t limit);

uint32_t get_temp_limit();
void set_temp_limit(uint32_t limit);

uint32_t get_run_time();
void increment_run_time();
#endif // STATE_H
