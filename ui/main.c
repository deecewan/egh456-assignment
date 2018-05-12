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
#include "utils/ustdlib.h"
#include "motor/measurement.h"
#include "motor/speed.h"
#include "../constants.h"
#include "../state.h"
#include "tabs.h"
#include "tabs/home.h"
#include "main.h"

#define TASKSTACKSIZE   512

int counter = 0;
Clock_Struct clockRuntimeTrackerStruct;
Clock_Struct clockTakeMeasurementStruct;

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

Void checkWithinLimits(double current, double temp) {
    if (current > get_current_limit()) {
        // stop
        set_motor_power(OFF);
        StopMotor();
    } else if (temp > get_temp_limit()) {
        // stop
        set_motor_power(OFF);
        StopMotor();
    }
}

Void updateMotorState(double speed) {
    switch (get_motor_power()) {
    case ON:
        if (speed < (get_motor_speed() * 0.9)) {
            set_motor_state(STARTING);
        } else {
            set_motor_state(RUNNING);
        }
        break;
    case OFF:
        if (speed > 0) {
            set_motor_state(STOPPING);
        } else {
            set_motor_state(IDLE);
        }
        break;
    }
}

Void clockRuntimeTracker(UArg arg) {
    counter++;
    RotateMotor();
    TakeMeasurements();

    if (!ReadingsReady()) {
        return;
    }

    double latest_average_speed = GetFilteredSpeed();
    double latest_average_current = GetFilteredCurrentValue();
    double latest_average_temp = 0;//GetFilteredTemperature();
    checkWithinLimits(latest_average_current, latest_average_temp);
    updateMotorState(latest_average_speed);
    updateStartStopButton();

    if (counter == 1000) {
        increment_run_time();
        appendToMotorSpeed(latest_average_speed);
        appendToCurrent(latest_average_current * 1000);
        appendToTemp(latest_average_temp);
        update_on_clock_cycle();
        counter = 0;
    }
}

void ui_setup(uint32_t sysclock, int hardware_status) {
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
  if (hardware_status == -1) {
      GrStringDraw(&g_sContext, "Invalid initial Hall Sensor reading", 36, 30, 0, 1);
      GrStringDraw(&g_sContext, "Move motor and restart program", 31, 0, 60, 1);
      return;
  }

  // set up clock to track run time
  Clock_Params clkParams;
  Clock_Params_init(&clkParams);
  clkParams.startFlag = TRUE;
  clkParams.period = 1;
  Clock_construct(
      &clockRuntimeTrackerStruct,
      (Clock_FuncPtr)clockRuntimeTracker,
      1,
      &clkParams
  );

  // Perform all setup functionality **here**
  setup_tabs(); // buttons are setup now

  // perform the first paint of the widgets
  WidgetPaint(WIDGET_ROOT);

}

void make_background_color(uint32_t color) {
    GrContextForegroundSet(&g_sContext, color);
}

