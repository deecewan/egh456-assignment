#ifndef UI_TABS_SETTINGS_H
#define UI_TABS_SETTINGS_H
#include <grlib/grlib.h>
#include <grlib/widget.h>

typedef enum INPUT_FIELDS {
    INPUT_MOTOR_SPEED,
    INPUT_CURRENT_LIMIT,
    INPUT_TEMP_LIMIT,
} INPUT_FIELDS;


void paint_settings(tWidget *psWidget, tContext *psContext);
#endif // UI_TABS_SETTINGS_H
