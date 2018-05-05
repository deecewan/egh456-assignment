#include <stdint.h>
#include <stdbool.h>
#include <grlib/grlib.h>
#include <grlib/widget.h>
#include <grlib/canvas.h>
#include <grlib/pushbutton.h>
#include "drivers/kentec320x240x16_ssd2119.h"
#include "../tabs.h"
#include "stats.h"

static VISIBILITY visibility = LINE_MOTOR_SPEED | LINE_CURRENT | LINE_TEMP;

void toggle_vis_motor_speed();
void toggle_vis_current();
void toggle_vis_temp();

void toggle_visibility(VISIBILITY vis) {
    visibility ^= vis;
}

RectangularButton(legendMotorSpeed, 0, 0, 0, &g_sKentec320x240x16_SSD2119,
  10, 240 - 24 - 8 - 30, 100, 16,
  PB_STYLE_TEXT | PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY, ClrChartreuse, ClrChartreuse, 0, ClrBlack,
  g_psFontCmss16, "Motor Speed", 0, 0, 0, 0, toggle_vis_motor_speed);

RectangularButton(legendCurrent, 0, 0, 0, &g_sKentec320x240x16_SSD2119,
  110, 240 - 24 - 8 - 30, 100, 16,
  PB_STYLE_TEXT | PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY, ClrCornflowerBlue, ClrCornflowerBlue, 0, ClrBlack,
  g_psFontCmss16, "Current", 0, 0, 0, 0, toggle_vis_current);

RectangularButton(legendTemp, 0, 0, 0, &g_sKentec320x240x16_SSD2119,
  210, 240 - 24 - 8 - 30, 100, 16,
  PB_STYLE_TEXT | PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY, ClrDeepPink, ClrDeepPink, 0, ClrBlack,
  g_psFontCmss16, "Temperature", 0, 0, 0, 0, toggle_vis_temp);

void paint_legend_item(VISIBILITY filter, tPushButtonWidget *button, uint32_t color) {
    uint32_t fill, text;
    if (visibility & filter) {
        fill = color;
        text = ClrBlack;
    } else {
        fill = ClrBlack;
        text = color;
    }

    PushButtonFillColorSet(button, fill);
    PushButtonTextColorSet(button, text);
    WidgetPaint((tWidget *)button);
}

void toggle_vis_motor_speed() {
    toggle_visibility(LINE_MOTOR_SPEED);
    paint_legend_item(LINE_MOTOR_SPEED, &legendMotorSpeed, ClrChartreuse);
}

void toggle_vis_current() {
    toggle_visibility(LINE_CURRENT);
    paint_legend_item(LINE_CURRENT, &legendCurrent, ClrCornflowerBlue);
}

void toggle_vis_temp() {
    toggle_visibility(LINE_TEMP);
    paint_legend_item(LINE_TEMP, &legendTemp, ClrDeepPink);
}

void paint_stats(tWidget *psWidget, tContext *psContext) {
    GrContextForegroundSet(psContext, ClrWhite);
    GrLineDraw(psContext, 10, 28, 10, 173);
    GrLineDraw(psContext, 10, 173, 309, 173);

    // Draw legend indicators
    WidgetAdd(psWidget, (tWidget *)&legendMotorSpeed);
    WidgetAdd(psWidget, (tWidget *)&legendCurrent);
    WidgetAdd(psWidget, (tWidget *)&legendTemp);
}
