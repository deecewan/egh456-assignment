#include <stdint.h>
#include <stdbool.h>
#include <grlib/grlib.h>
#include <grlib/widget.h>
#include <grlib/canvas.h>
#include <grlib/pushbutton.h>
#include "drivers/kentec320x240x16_ssd2119.h"
#include "state.h"
#include "constants.h"
#include "tabs/stats.h"
#include "tabs/home.h"
#include "tabs/settings.h"
#include "tabs.h"
#include "main.h"

extern tCanvasWidget panels[];
volatile PANEL selected_panel = SETTINGS;

void redraw_tab_buttons();

Canvas(statsPanel, panels + 0, 0, 0, &g_sKentec320x240x16_SSD2119,
       PANEL_X_VALUE, PANEL_Y_VALUE, PANEL_WIDTH,  PANEL_HEIGHT,
       CANVAS_STYLE_APP_DRAWN, 0, 0, 0, 0, 0, 0, paint_stats);
Canvas(homePanel, panels + 1, 0, 0, &g_sKentec320x240x16_SSD2119,
       PANEL_X_VALUE, PANEL_Y_VALUE, PANEL_WIDTH,  PANEL_HEIGHT,
       CANVAS_STYLE_APP_DRAWN, 0, 0, 0, 0, 0, 0, paint_home);
Canvas(settingsPanel, panels + 2, 0, 0, &g_sKentec320x240x16_SSD2119,
       PANEL_X_VALUE, PANEL_Y_VALUE, PANEL_WIDTH,  PANEL_HEIGHT,
       CANVAS_STYLE_APP_DRAWN, 0, 0, 0, 0, 0, 0, paint_settings);

tCanvasWidget panels[] =
{
    CanvasStruct(0, 0, &statsPanel, &g_sKentec320x240x16_SSD2119,
                 PANEL_X_VALUE, PANEL_Y_VALUE, PANEL_WIDTH, PANEL_HEIGHT,
                 CANVAS_STYLE_FILL, ClrBlack, 0, 0, 0, 0, 0, 0),
    CanvasStruct(0, 0, &homePanel, &g_sKentec320x240x16_SSD2119,
                 PANEL_X_VALUE, PANEL_Y_VALUE, PANEL_WIDTH, PANEL_HEIGHT,
                 CANVAS_STYLE_FILL, ClrBlack, 0, 0, 0, 0, 0, 0),
    CanvasStruct(0, 0, &settingsPanel, &g_sKentec320x240x16_SSD2119,
                 PANEL_X_VALUE, PANEL_Y_VALUE, PANEL_WIDTH, PANEL_HEIGHT,
                 CANVAS_STYLE_FILL, ClrBlack, 0, 0, 0, 0, 0, 0),
};

void select_panel(PANEL panel) {
    if (selected_panel == panel) {
        // we're already looking at this panel
        return;
    }
    WidgetRemove((tWidget *)(panels + selected_panel));
    selected_panel = panel;
    WidgetPaint((tWidget *)(panels + selected_panel));
    WidgetAdd(WIDGET_ROOT, (tWidget *)(panels + selected_panel));
    redraw_tab_buttons();
}

void select_stats_panel() {
    select_panel(STATS);
}

void select_home_panel() {
    select_panel(HOME);
}

void select_settings_panel() {
    select_panel(SETTINGS);
}

/* ========= TAB BUTTONS =========== */
RectangularButton(btnStats, 0, 0, 0, &g_sKentec320x240x16_SSD2119,
                  PANEL_X_VALUE, SCREEN_HEIGHT - TAB_HEIGHT - MARGIN_BOTTOM, TAB_WIDTH, TAB_HEIGHT,
                  PB_STYLE_TEXT | PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY, ClrBlue, ClrAqua, 0, ClrWhite,
                  g_psFontCmss20, "Stats", 0, 0, 0, 0, select_stats_panel);
// this one is initially selected
RectangularButton(btnHome, 0, 0, 0, &g_sKentec320x240x16_SSD2119,
                  PANEL_X_VALUE + TAB_WIDTH, SCREEN_HEIGHT - TAB_HEIGHT - MARGIN_BOTTOM, TAB_WIDTH, TAB_HEIGHT,
                  PB_STYLE_TEXT | PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY, ClrBlue, ClrAqua, 0, ClrWhite,
                  g_psFontCmss20, "Home", 0, 0, 0, 0, select_home_panel);

RectangularButton(btnSettings, 0, 0, 0, &g_sKentec320x240x16_SSD2119,
                  PANEL_X_VALUE + (TAB_WIDTH * 2), SCREEN_HEIGHT - TAB_HEIGHT - MARGIN_BOTTOM, TAB_WIDTH, TAB_HEIGHT,
                  PB_STYLE_TEXT | PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY, ClrBlue, ClrAqua, 0, ClrWhite,
                  g_psFontCmss20, "Settings", 0, 0, 0, 0, select_settings_panel);

void redraw_tab_buttons() {
    // background color onto the selected one
    PushButtonFillColorSet(&btnStats, selected_panel == STATS ? ClrGray : ClrBlue);
    PushButtonFillColorPressedSet(&btnStats, selected_panel == STATS ? ClrGray : ClrAqua);

    PushButtonFillColorSet(&btnHome, selected_panel == HOME ? ClrGray : ClrBlue);
    PushButtonFillColorPressedSet(&btnHome, selected_panel == HOME ? ClrGray : ClrAqua);

    PushButtonFillColorSet(&btnSettings, selected_panel == SETTINGS ? ClrGray : ClrBlue);
    PushButtonFillColorPressedSet(&btnSettings, selected_panel == SETTINGS ? ClrGray : ClrAqua);

    WidgetPaint((tWidget *)&btnStats);
    WidgetPaint((tWidget *)&btnHome);
    WidgetPaint((tWidget *)&btnSettings);
}


void setup_tabs() {
    WidgetAdd(WIDGET_ROOT, (tWidget *)&btnStats);
    WidgetAdd(WIDGET_ROOT, (tWidget *)&btnHome);
    WidgetAdd(WIDGET_ROOT, (tWidget *)&btnSettings);
    redraw_tab_buttons();

    // Add the first panel - home panel
    WidgetAdd(WIDGET_ROOT, (tWidget *)(panels + selected_panel));
}

void tabs_onStateChange() {
    switch (selected_panel) {
        case HOME:
            home_onStateChange();
            break;
        case SETTINGS:
        case STATS:
            break;
    }
}

void update_runtime_display() {
    if (selected_panel == HOME) {
        home_updateRuntime();
    }
}

// this is a cheeky way to clear the whole screen
void wipe_panel_area() {
    static const tRectangle sRect =
    {
        MARGIN_LEFT,
        MARGIN_TOP,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
    };
    GrContextForegroundSet(&g_sContext, ClrBlack);
    GrRectFill(&g_sContext, &sRect);
}

void hide_current_panel() {
    WidgetRemove((tWidget *)(panels + selected_panel));
    WidgetRemove((tWidget *)&btnStats);
    WidgetRemove((tWidget *)&btnHome);
    WidgetRemove((tWidget *)&btnSettings);
    wipe_panel_area();
}

void show_current_panel() {
    wipe_panel_area();
    setup_tabs();
    WidgetPaint((tWidget *)(panels + selected_panel));
}
