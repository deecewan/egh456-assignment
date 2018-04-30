#ifndef STATE_H
#define STATE_H
#include <stdint.h>
#include "constants.h"

MOTOR_STATE get_motor_state();
void set_motor_state(MOTOR_STATE state);
void toggle_motor_state();

uint32_t get_current_limit();
void set_current_limit(uint32_t limit);

uint32_t get_temp_limit();
void set_temp_limit(uint32_t limit);
#endif // STATE_H
