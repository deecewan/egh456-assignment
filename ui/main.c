#include <stdint.h>
#include <stdbool.h>
#include <grlib/grlib.h>
#include <grlib/widget.h>
#include <driverlib/gpio.h>
#include <driverlib/rom.h>
#include <driverlib/rom_map.h>
#include <inc/hw_memmap.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>
#include "drivers/kentec320x240x16_ssd2119.h"
#include "drivers/frame.h"
#include "drivers/touch.h"
#include "../constants.h"
#include "../state.h"
#include "tabs.h"
#include "tabs/home.h"
#include "main.h"

#define TASKSTACKSIZE   512

Char taskRedrawLoopStack[TASKSTACKSIZE];
Task_Struct taskRedrawLoopStruct;
Clock_Struct clockRuntimeTrackerStruct;

MOTOR_STATE last_known_state;

// this task runs when the system is idle
// at the lowest priority to redraw the screen
// contents. It really isn't that important when
// (within reason), the screen is redrawn
Void idleTask() {
    MOTOR_POWER power = get_motor_power();
    switch (power) {
        case ON:
            ROM_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_5, 0x0);
            ROM_GPIOPinWrite(GPIO_PORTQ_BASE, GPIO_PIN_7, GPIO_PIN_7);
            break;
        case OFF:
            ROM_GPIOPinWrite(GPIO_PORTQ_BASE, GPIO_PIN_7, 0x0);
            ROM_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_5, GPIO_PIN_5);
            break;
    }
    MOTOR_STATE state = get_motor_state();
    if (state != last_known_state) {
        last_known_state = state;
        tabs_onStateChange();
    }
    // process anything pending
    WidgetMessageQueueProcess();
}

Void clockRuntimeTracker(UArg arg) {
    increment_run_time();

    appendToMotorSpeed(urand());
    appendToCurrent(urand());
    appendToTemp(urand());

    update_on_clock_cycle();
}

void ui_setup(uint32_t sysclock) {
  usrand(sysclock);
  // Init the display driver
  Kentec320x240x16_SSD2119Init(sysclock);

  // Init Graphics Context
  GrContextInit(&g_sContext, &g_sKentec320x240x16_SSD2119);

  // Init touchscreen
  TouchScreenInit(sysclock);
  TouchScreenCallbackSet(WidgetPointerMessage);

  // Draw the application frame
  FrameDraw(&g_sContext, "Group 10 Motor Controller");

  // Perform all setup functionality **here**
  setup_tabs(); // buttons are setup now

  // perform the first paint of the widgets
  WidgetPaint(WIDGET_ROOT);

  // set up clock to track run time
  Clock_Params clkParams;
  Clock_Params_init(&clkParams);
  clkParams.startFlag = TRUE;
  clkParams.period = 1000;
  Clock_construct(
      &clockRuntimeTrackerStruct,
      (Clock_FuncPtr)clockRuntimeTracker,
      1,
      &clkParams
  );
}

void make_background_color(uint32_t color) {
    GrContextForegroundSet(&g_sContext, color);
}

