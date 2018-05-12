#include <stdint.h>
#include <stdbool.h>
#include <grlib/grlib.h>
#include <grlib/widget.h>
#include <grlib/canvas.h>
#include <grlib/pushbutton.h>
#include "drivers/kentec320x240x16_ssd2119.h"
#include "utils/ustdlib.h"
#include "../tabs.h"
#include "../state.h"
#include "stats.h"

static VISIBILITY visibility = LINE_MOTOR_SPEED | LINE_CURRENT | LINE_TEMP;

void draw_charts(tWidget *psWidget, tContext *context);

void toggle_vis_motor_speed();
void toggle_vis_current();
void toggle_vis_temp();

void toggle_visibility(VISIBILITY vis) {
    visibility ^= vis;
}

char speedString[20] = "Speed";
char currentString[20] = "Current";
char tempString[20] = "Temp";

RectangularButton(legendMotorSpeed, 0, 0, 0, &g_sKentec320x240x16_SSD2119,
  10, 240 - 24 - 8 - 30, 100, 16,
  PB_STYLE_TEXT | PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY, ClrChartreuse, ClrChartreuse, 0, ClrBlack,
  g_psFontCmss16, speedString, 0, 0, 0, 0, toggle_vis_motor_speed);

RectangularButton(legendCurrent, 0, 0, 0, &g_sKentec320x240x16_SSD2119,
  110, 240 - 24 - 8 - 30, 100, 16,
  PB_STYLE_TEXT | PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY, ClrCornflowerBlue, ClrCornflowerBlue, 0, ClrBlack,
  g_psFontCmss16, currentString, 0, 0, 0, 0, toggle_vis_current);

RectangularButton(legendTemp, 0, 0, 0, &g_sKentec320x240x16_SSD2119,
  210, 240 - 24 - 8 - 30, 100, 16,
  PB_STYLE_TEXT | PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY, ClrDeepPink, ClrDeepPink, 0, ClrBlack,
  g_psFontCmss16, tempString, 0, 0, 0, 0, toggle_vis_temp);

Canvas(graphArea, 0, 0, 0, &g_sKentec320x240x16_SSD2119,
       11, 28, 299, 145, CANVAS_STYLE_APP_DRAWN | CANVAS_STYLE_FILL,
       0, 0, 0, 0, 0, 0, draw_charts);

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
    WidgetPaint((tWidget *)&graphArea);
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
    WidgetAdd(psWidget, (tWidget *)&graphArea);
}


uint32_t scale_value(uint32_t largest, uint32_t value) {
    return 170 - (value * ((float)140 / largest));
}

void draw_chart(tContext *context, uint32_t color, uint32_t *list, uint32_t largest) {
    GrContextForegroundSet(context, color);
    for (uint8_t i = 0; i < LIST_ITEM_COUNT; i++) {
        uint32_t current = scale_value(largest, list[i]);
        GrCircleDraw(context, LINE_X_VALUE(i), current, CIRCLE_RADIUS);
        // skip the first because we can't draw a line to nowhere
        if (i > 0) {
            uint32_t last = scale_value(largest, list[i - 1]);
            // draw a line linking the two dots
            GrLineDraw(context, LINE_X_VALUE(i - 1), last, LINE_X_VALUE(i), current);
        }
    }
}

void draw_charts(tWidget *psWidget, tContext *context) {
    if (visibility & LINE_MOTOR_SPEED) {
        draw_chart(context, ClrChartreuse, get_motor_speed_list(), get_largest_motor_speed());
        usprintf(speedString, "Speed: %d rpm", get_largest_motor_speed());
    } else {
        usprintf(speedString, "Speed");
    }
    if (visibility & LINE_CURRENT) {
        draw_chart(context, ClrCornflowerBlue, get_current_list(), get_largest_current());
        usprintf(currentString, "Current: %d mA", get_largest_current());
    } else {
        usprintf(currentString, "Current");
    }
    if (visibility & LINE_TEMP) {
        draw_chart(context, ClrDeepPink, get_temp_list(), get_largest_temp());
        usprintf(tempString, "Temp: %d C", get_largest_temp());
    } else {
        usprintf(tempString, "Temp");
    }
    WidgetPaint((tWidget *)&legendMotorSpeed);
    WidgetPaint((tWidget *)&legendTemp);
    WidgetPaint((tWidget *)&legendCurrent);
}

void stats_redrawGraphs() {
    WidgetPaint((tWidget *)&graphArea);
}
