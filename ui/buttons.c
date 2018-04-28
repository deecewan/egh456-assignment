#include <stdint.h>
#include <stdbool.h>
#include <grlib/grlib.h>
#include <grlib/widget.h>
#include <grlib/pushbutton.h>
#include "drivers/kentec320x240x16_ssd2119.h"
#include "constants.h"
#include "state.h"

bool SHOULD_REPAINT = 1;

tPushButtonWidget btnStart, btnStop;
const char* textStart = "Start";
const char* textStarting = "Starting";
const char* textStop = "Stop";
const char* textStopping = "Stopping";

void onStartPress(tWidget *psWidget) {
    if (get_motor_state() == OFF) {
        set_motor_state(ON);
        SHOULD_REPAINT = true;
    }
}

void onStopPress(tWidget *psWidget) {
    if (get_motor_state() == ON) {
        set_motor_state(OFF);
        SHOULD_REPAINT = true;
    }
}

tPushButtonWidget create_start_stop_button(
        const char *text,
        uint32_t x,
        bool enabled,
        uint32_t fill,
        uint32_t press_fill,
        void (*cb)(tWidget *psWidget)
) {
    tPushButtonWidget _some_button = RectangularButtonStruct(
                0, 0, 0, // no parent, children or next
                /* display */&g_sKentec320x240x16_SSD2119,
                /* x */x, /* y */ 100, /* width */ 100, /* height */ 100,
                /* style */PB_STYLE_TEXT | PB_STYLE_FILL,
                // if we can press is, make it green, otherwise grey
                /* fill */ enabled ? fill: ClrGray,
                // if we can press it, highlight when pressed. otherwise, leave
                /* onPress fill */ enabled ? press_fill : ClrGray,
                /* outline color */0, /* text color */ClrWhite,
                /* font */ g_psFontCm20, /* text */text,
                0, 0, 0, 0,
                /* callback */cb
                );
    return _some_button;
}

void create_buttons() {
    MOTOR_STATE state = get_motor_state();

    btnStart = create_start_stop_button(textStart, 200, state == OFF, ClrGreenYellow, ClrHoneydew, onStartPress);
    btnStop = create_start_stop_button(textStop, 20, state == ON, ClrRed, ClrLightSalmon, onStopPress);
}

void redraw_buttons() {
    if (SHOULD_REPAINT) {
        SHOULD_REPAINT = false;
        create_buttons();
        WidgetPaint((tWidget *)&btnStart);
        WidgetPaint((tWidget *)&btnStop);
    }
}


void setup_buttons() {
    create_buttons();
    WidgetAdd(WIDGET_ROOT, (tWidget *)&btnStart);
    WidgetAdd(WIDGET_ROOT, (tWidget *)&btnStop);
}
