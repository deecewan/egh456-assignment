#ifndef UI_MAIN_H
#define UI_MAIN_H
#include <grlib/grlib.h>

void ui_setup();

// Global graphics context;
tContext g_sContext;
void make_background_color(uint32_t color);
#endif // UI_MAIN_H
