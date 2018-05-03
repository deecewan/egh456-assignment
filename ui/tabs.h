#ifndef UI_TABS_H
#define UI_TABS_H
#include <stdint.h>

void setup_tabs();
void select_tab(uint32_t idx);
void tabs_onStateChange();
void update_runtime_display();

void wipe_panel_area();
void hide_current_panel();
void show_current_panel();

#define MARGIN_TOP 24
#define MARGIN_LEFT 8
#define MARGIN_BOTTOM 9
#define MARGIN_RIGHT 8

#define SCREEN_HEIGHT 240
#define SCREEN_WIDTH 320

#define TAB_HEIGHT 30
#define TAB_WIDTH ((int)(SCREEN_WIDTH - MARGIN_LEFT - MARGIN_RIGHT) / 3)

#define PANEL_X_VALUE MARGIN_LEFT
#define PANEL_Y_VALUE MARGIN_TOP
#define PANEL_WIDTH (SCREEN_WIDTH - MARGIN_LEFT - MARGIN_RIGHT)
#define PANEL_HEIGHT (SCREEN_HEIGHT - PANEL_Y_VALUE - MARGIN_BOTTOM - TAB_HEIGHT)
#endif // UI_TABS_H
