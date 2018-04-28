#include <stdint.h>
#include <stdbool.h>
#include <grlib/grlib.h>
#include <grlib/widget.h>
#include <driverlib/rom.h>
#include <driverlib/rom_map.h>
#include <ti/sysbios/knl/Task.h>
#include "drivers/kentec320x240x16_ssd2119.h"
#include "drivers/frame.h"
#include "drivers/touch.h"
#include "constants.h"
#include "buttons.h"

#define TASKSTACKSIZE   512

// Global graphics context;
tContext g_sContext;

Char taskRedrawLoopStack[TASKSTACKSIZE];
Task_Struct taskRedrawLoopStruct;

Void taskRedrawLoop(UArg arg0, UArg arg1) {
  while (1) {
    // process anything pending
    WidgetMessageQueueProcess();

    // perform all redraws
    redraw_buttons();
  }
}

void ui_setup(uint32_t sysclock) {
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
  setup_buttons(); // buttons are setup now

  // perform the first paint of the widgets
  WidgetPaint(WIDGET_ROOT);

  // we kick off a task in here to handle drawing
  Task_Params taskParams;
  Task_Params_init(&taskParams);
  taskParams.stackSize = TASKSTACKSIZE;
  taskParams.stack = &taskRedrawLoopStack;
  taskParams.priority = 10;
  Task_construct(&taskRedrawLoopStruct, (Task_FuncPtr)taskRedrawLoop, &taskParams, NULL);
}


