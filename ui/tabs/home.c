#include <stdint.h>
#include <stdbool.h>
#include <grlib/grlib.h>
#include <grlib/widget.h>
#include <grlib/canvas.h>
#include <grlib/pushbutton.h>
#include "drivers/kentec320x240x16_ssd2119.h"
#include "constants.h"
#include "state.h"
#include "../tabs.h"
#include "home.h"

void onPress(tWidget *psWidget);

RectangularButton(btnToggleMotor, 0, 0, 0, &g_sKentec320x240x16_SSD2119,
                      (SCREEN_WIDTH - BUTTON_WIDTH) / 2, MARGIN_TOP + 10, BUTTON_WIDTH, BUTTON_HEIGHT,
                      PB_STYLE_TEXT | PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY, ClrGreenYellow, ClrGreen, 0, ClrWhite,
                      g_psFontCm20, "Start", 0, 0, 0, 0, onPress);

void paint_home(tWidget *psWidget, tContext *psContext) {
    WidgetAdd(psWidget, (tWidget *)&btnToggleMotor);
}

void onPress(tWidget *psWidget) {
    toggle_motor_state();
    MOTOR_STATE state = get_motor_state();
    uint32_t fill, press_fill;
    char *text;

    switch (state) {
      case ON:
        fill = ClrRed;
        press_fill = ClrLightSalmon;
        text = "Stop";
        break;
      case OFF:
        fill = ClrGreenYellow;
        press_fill = ClrGreen;
        text = "Start";
        break;
    }

    PushButtonFillColorSet(&btnToggleMotor, fill);
    PushButtonFillColorPressedSet(&btnToggleMotor, press_fill);
    PushButtonTextSet(&btnToggleMotor, text);
    WidgetPaint((tWidget *)&btnToggleMotor);
}
