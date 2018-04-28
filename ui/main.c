#include <stdint.h>
#include <stdbool.h>
#include "grlib/grlib.h"
#include "../drivers/kentec320x240x16_ssd2119.h"
#include "../drivers/frame.h"
#include "./constants.h"

// Global graphics context;
tContext g_sContext;

MOTOR_STATE motor_state = OFF;

void set_motor_state(MOTOR_STATE state) {
  motor_state = state;
}

void toggle_motor_state() {
  motor_state ^= ON;
}

void ui_setup(uint32_t sysclock) {
  // Init the display driver
  Kentec320x240x16_SSD2119Init(sysclock);

  // Init Graphics Context
  GrContextInit(&g_sContext, &g_sKentec320x240x16_SSD2119);

  // Draw the application frame
  FrameDraw(&g_sContext, "Group 10 Motor Controller");
}


