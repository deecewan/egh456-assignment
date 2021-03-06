#include <stdint.h>
#include <stdbool.h>
#include <grlib/grlib.h>
#include <grlib/widget.h>
#include <grlib/canvas.h>
#include <grlib/pushbutton.h>
#include <utils/ustdlib.h>
#include <ti/sysbios/hal/Seconds.h>
#include <time.h>
#include "drivers/kentec320x240x16_ssd2119.h"
#include "motor/speed.h"
#include "constants.h"
#include "state.h"
#include "../tabs.h"
#include "home.h"
#include "../calendar.h"

static bool stop_motor = false;
void onPress(tWidget *psWidget);
void home_onStateUpdate();
void updateStateIndicator();
void home_updateRuntime();
void updateStartStopButton();
void StopFaultyMotor();
bool ShouldMotorBeStopped();

RectangularButton(btnToggleMotor, 0, 0, 0, &g_sKentec320x240x16_SSD2119,
                      (SCREEN_WIDTH - BUTTON_WIDTH) / 2, MARGIN_TOP + 10, BUTTON_WIDTH, BUTTON_HEIGHT,
                      PB_STYLE_TEXT | PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY, ClrGreenYellow, ClrGreen, 0, ClrWhite,
                      g_psFontCmss20, "Start", 0, 0, 0, 0, onPress);

Canvas(boxStateIndicator, 0, 0, 0, &g_sKentec320x240x16_SSD2119,
                 10, 100, 20, 20,
                 CANVAS_STYLE_FILL | CANVAS_STYLE_OUTLINE, COLOR_IDLE, ClrGreen, 0, 0, 0, 0, 0);
Canvas(textStateIndicator, 0, 0, 0, &g_sKentec320x240x16_SSD2119,
                 40, 100, 75, 20,
                 CANVAS_STYLE_TEXT | CANVAS_STYLE_FILL | CANVAS_STYLE_TEXT_LEFT,
                 ClrBlack, ClrGreen, ClrWhite, g_psFontCmss20, "Idle", 0, 0);
Canvas(textRuntime, 0, 0, 0, &g_sKentec320x240x16_SSD2119,
                 10, 140, 105, 20,
                 CANVAS_STYLE_TEXT | CANVAS_STYLE_TEXT_LEFT | CANVAS_STYLE_FILL,
                 ClrBlack, ClrGreen, ClrWhite, g_psFontCmss20, "Time: ", 0, 0);
char runtime[20] = "0s";
Canvas(textRuntimeValue, 0, 0, 0, &g_sKentec320x240x16_SSD2119,
                 60, 140, 105, 20,
                 CANVAS_STYLE_TEXT | CANVAS_STYLE_TEXT_LEFT | CANVAS_STYLE_FILL,
                 ClrBlack, ClrGreen, ClrWhite, g_psFontCmss20, runtime, 0, 0);
char timestamp[50] = "";
Canvas(textTimestamp, 0, 0, 0, &g_sKentec320x240x16_SSD2119,
                 10, 170, 170, 20,
                 CANVAS_STYLE_TEXT | CANVAS_STYLE_TEXT_LEFT | CANVAS_STYLE_FILL,
                 ClrBlack, ClrGreen, ClrWhite, g_psFontCmss20, timestamp, 0, 0);
static char motorSpeed[22] = "Motor Speed: 0 rpm"; // allows up to 5 digits
Canvas(textMotorSpeed, 0, 0, 0, &g_sKentec320x240x16_SSD2119,
                 160, 100, 150, 20,
                 CANVAS_STYLE_TEXT | CANVAS_STYLE_TEXT_RIGHT | CANVAS_STYLE_FILL,
                 ClrBlack, ClrGreen, ClrWhite, g_psFontCmss16, motorSpeed, 0, 0);
static char currentLimit[24] = "Current Limit: 0 mA"; // allows up to 5 digits
Canvas(textCurrentLimit, 0, 0, 0, &g_sKentec320x240x16_SSD2119,
                 160, 130, 160, 20,
                 CANVAS_STYLE_TEXT | CANVAS_STYLE_TEXT_RIGHT | CANVAS_STYLE_FILL,
                 ClrBlack, ClrGreen, ClrWhite, g_psFontCmss16, currentLimit, 0, 0);
static char tempLimit[16] = "Temp Limit: 0 C"; // allows up to 3 digits
Canvas(textTempLimit, 0, 0, 0, &g_sKentec320x240x16_SSD2119,
                 160, 160, 150, 20,
                 CANVAS_STYLE_TEXT | CANVAS_STYLE_TEXT_RIGHT | CANVAS_STYLE_FILL,
                 ClrBlack, ClrGreen, ClrWhite, g_psFontCmss16, tempLimit, 0, 0);

void updateDateDisplay() {

    GetCalendarTime(timestamp);
//    time_t t;
//    struct tm ltm;
//    t = time(NULL);
//    t = Seconds_get();
//    ulocaltime(t, &ltm);
//    usprintf(timestamp, "%d/%d/%d %d:%d:%d", 1900 + ltm.tm_year, 1 + ltm.tm_mon, ltm.tm_mday, ltm.tm_hour, ltm.tm_min, ltm.tm_sec);
//
//    CanvasTextSet(&textTimestamp, timestamp);
    WidgetPaint((tWidget *)&textTimestamp);
}

void paint_home(tWidget *psWidget, tContext *psContext) {
    WidgetAdd(psWidget, (tWidget *)&btnToggleMotor);

    WidgetAdd(psWidget, (tWidget *)&boxStateIndicator);
    WidgetAdd(psWidget, (tWidget *)&textStateIndicator);

    WidgetAdd(psWidget, (tWidget *)&textRuntime);
    WidgetAdd(psWidget, (tWidget *)&textRuntimeValue);

    usprintf(motorSpeed, "Motor Speed: %d rpm", get_motor_speed());
    usprintf(currentLimit, "Current Limit: %d mA", get_current_limit());
    usprintf(tempLimit, "Temp Limit: %d C", get_temp_limit());

    WidgetAdd(psWidget, (tWidget *)&textMotorSpeed);
    WidgetAdd(psWidget, (tWidget *)&textCurrentLimit);
    WidgetAdd(psWidget, (tWidget *)&textTempLimit);

    WidgetAdd(psWidget, (tWidget *)&textTimestamp);


    updateStateIndicator();
    home_updateRuntime();
}

void StopFaultyMotor() {
    uint32_t fill, press_fill;
    char *text;
    fill = ClrGreenYellow;
    press_fill = ClrGreen;
    text = "Start";
    set_motor_speed(0);
    stop_motor = true;

    PushButtonFillColorSet(&btnToggleMotor, fill);
    PushButtonFillColorPressedSet(&btnToggleMotor, press_fill);
    PushButtonTextSet(&btnToggleMotor, text);
    WidgetPaint((tWidget *)&btnToggleMotor);
}

bool ShouldMotorBeStopped() {
    return stop_motor;
}

void updateStartStopButton() {
    toggle_motor_power();
    MOTOR_POWER power = get_motor_power();
    uint32_t fill, press_fill;
    char *text;

    switch (power) {
      case ON:
        fill = ClrRed;
        press_fill = ClrLightSalmon;
        text = "Stop";
        stop_motor = false;
        SetMotorSpeed((int)get_motor_speed());
        // if statement below is just there in case user violates state diagram
        // and user wants to run motor while it is stopping
        if (get_motor_state() == IDLE || get_motor_state() == STARTING) {
            StartMotor();
        }
        break;
      case OFF:
        fill = ClrGreenYellow;
        press_fill = ClrGreen;
        text = "Start";
        set_motor_speed(0);
        stop_motor = true;
        break;
    }

    PushButtonFillColorSet(&btnToggleMotor, fill);
    PushButtonFillColorPressedSet(&btnToggleMotor, press_fill);
    PushButtonTextSet(&btnToggleMotor, text);
    WidgetPaint((tWidget *)&btnToggleMotor);
}

void onPress(tWidget *psWidget) {
    updateStartStopButton();
}

void updateStateIndicator() {
    MOTOR_STATE state = get_motor_state();

    uint32_t boxColor;
    char *text;
    switch (state) {
      case IDLE:
        text = "Idle";
        boxColor = COLOR_IDLE;
        break;
      case STARTING:
        text = "Starting";
        boxColor = COLOR_STARTING;
        break;
      case RUNNING:
        text = "Running";
        boxColor = COLOR_RUNNING;
        break;
      case STOPPING:
        text = "Stopping";
        boxColor = COLOR_STOPPING;
        break;
    }

    CanvasFillColorSet(&boxStateIndicator, boxColor);
    CanvasTextSet(&textStateIndicator, text);
    WidgetPaint((tWidget *)&boxStateIndicator);
    WidgetPaint((tWidget *)&textStateIndicator);
}

void home_onStateChange() {
    updateStateIndicator();
}

void home_updateRuntime() {
    updateDateDisplay();
    uint32_t hours, minutes, seconds = get_run_time();
    minutes = seconds / 60;
    hours = minutes / 60;
    seconds = seconds % 60;
    minutes = minutes % 60;
    if (hours > 0) {
        usprintf(runtime, "%uh:%um:%us", hours, minutes, seconds);
    } else if (minutes > 0) {
        usprintf(runtime, "%um:%us", minutes, seconds);
    } else {
        usprintf(runtime, "%us", seconds);
    }
//    CanvasTextSet(&textRuntimeValue, runtime);
    WidgetPaint((tWidget *)&textRuntimeValue);
}
