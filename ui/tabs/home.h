#ifndef UI_TABS_HOME_H
#define UI_TABS_HOME_H
#include <grlib/grlib.h>
#include <grlib/widget.h>

#define BUTTON_WIDTH 200
#define BUTTON_HEIGHT 40

// The motor state colors

#define COLOR_IDLE     ClrAquamarine
#define COLOR_STOPPING ClrCrimson
#define COLOR_STARTING ClrDarkOrange
#define COLOR_RUNNING  ClrLawnGreen

void paint_home(tWidget *psWidget, tContext *psContext);
void home_onStateChange();
void home_updateRuntime();
#endif // UI_TABS_HOME_H
