#ifndef UI_TABS_STATS_H
#define UI_TABS_STATS_H
#include <grlib/grlib.h>
#include <grlib/widget.h>

typedef enum VISIBILITY {
    LINE_MOTOR_SPEED = 0b1,
    LINE_CURRENT = 0b10,
    LINE_TEMP = 0b100,
} VISIBILITY;

void paint_stats(tWidget *psWidget, tContext *psContext);
#endif // UI_TABS_STATS_H
