#ifndef UI_TABS_STATS_H
#define UI_TABS_STATS_H
#include <grlib/grlib.h>
#include <grlib/widget.h>
#include "../state.h"

typedef enum VISIBILITY {
    LINE_MOTOR_SPEED = 0b1,
    LINE_CURRENT = 0b10,
    LINE_TEMP = 0b100,
} VISIBILITY;

#define CIRCLE_RADIUS 2

#define LINE_X_VALUE(i) (13 + ((i) * (320 / LIST_ITEM_COUNT)))

void paint_stats(tWidget *psWidget, tContext *psContext);
void stats_redrawGraphs();
#endif // UI_TABS_STATS_H
