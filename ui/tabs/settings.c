#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <xdc/runtime/System.h>
#include <grlib/grlib.h>
#include <grlib/widget.h>
#include <grlib/canvas.h>
#include <grlib/keyboard.h>
#include <grlib/pushbutton.h>
#include "utils/ustdlib.h"
#include "drivers/kentec320x240x16_ssd2119.h"
#include "state.h"
#include "../main.h"
#include "../tabs.h"
#include "settings.h"

// initialize the values for handling keyboard input
const char emptyPlaceholderString = 0;
char keyboardEntryValue[10];
volatile uint32_t keyboardInputIndex = 0;
char motorSpeed[20] = "0 rpm";
char currentLimit[20] = "0 mA";
char tempLimit[20] = "0 C";
INPUT_FIELDS visibleField;
extern tPushButtonWidget buttonMotorSpeed, buttonCurrentLimit, buttonTempLimit;
extern tCanvasWidget inputMotorSpeed, inputCurrentLimit, inputTempLimit;
void onKeyPress(tWidget *psWidget, uint32_t ui32Key, uint32_t ui32Event);

extern tCanvasWidget g_sKeyboardBackground;

// declare the keyboard screen *here*
Keyboard(g_sKeyboard, &g_sKeyboardBackground, 0, 0,
         &g_sKentec320x240x16_SSD2119,
         MARGIN_LEFT,
         MARGIN_TOP + 30, // move down to make room for display
         SCREEN_WIDTH - MARGIN_LEFT - MARGIN_RIGHT,
         SCREEN_HEIGHT - MARGIN_TOP - MARGIN_BOTTOM - 30,
         KEYBOARD_STYLE_FILL | KEYBOARD_STYLE_AUTO_REPEAT |
         KEYBOARD_STYLE_PRESS_NOTIFY | KEYBOARD_STYLE_RELEASE_NOTIFY |
         KEYBOARD_STYLE_BG,
         ClrBlack, ClrGray, ClrDarkGray, ClrGray, ClrBlack, g_psFontCmss14,
         100, 100, NUM_KEYBOARD_US_ENGLISH, g_psKeyboardUSEnglish, onKeyPress);

Canvas(g_sKeyboardText, &g_sKeyboardBackground, &g_sKeyboard, 0,
       &g_sKentec320x240x16_SSD2119, PANEL_X_VALUE, MARGIN_TOP,
       PANEL_WIDTH, 24,
       CANVAS_STYLE_FILL | CANVAS_STYLE_TEXT | CANVAS_STYLE_TEXT_LEFT,
       ClrBlack, ClrWhite, ClrWhite, g_psFontCmss24,
       &emptyPlaceholderString, 0 ,0 );

Canvas(keyboardCursor, &g_sKeyboardBackground, &g_sKeyboardText, 0,
       &g_sKentec320x240x16_SSD2119, PANEL_X_VALUE, PANEL_Y_VALUE,
       1, 20, CANVAS_STYLE_FILL, ClrWhite, ClrWhite, 0, 0, 0, 0, 0);

Canvas(g_sKeyboardBackground, WIDGET_ROOT, 0, &g_sKeyboardText,
       &g_sKentec320x240x16_SSD2119, PANEL_X_VALUE, PANEL_Y_VALUE,
       PANEL_WIDTH, PANEL_HEIGHT,
       CANVAS_STYLE_FILL, ClrBlack, ClrWhite, ClrWhite, 0, 0, 0 ,0 );

uint32_t parse_int(char *string) {
    // from http://blockofcodes.blogspot.com.au/2013/07/how-to-convert-string-to-integer-in-c.html
    uint8_t len = strlen(string);
    uint32_t result = 0;
    for (uint8_t i = 0; i < len; i++) {
        // shift up by a 'tens column', and then add the next value
        result = (result * 10) + (string[i] - '0');
    }
    return result;
}

void onKeyPress(tWidget *psWidget, uint32_t ui32Key, uint32_t ui32Event) {
    if (ui32Event != KEYBOARD_EVENT_PRESS) {
        return; // only handle push down, not up
    }
    if (ui32Key == UNICODE_BACKSPACE) {
        if (keyboardInputIndex != 0) {
            keyboardInputIndex--;
            keyboardEntryValue[keyboardInputIndex] = 0;
        }
        WidgetPaint((tWidget *)&g_sKeyboardText);
        return;
    }
    if (ui32Key == UNICODE_RETURN) {
        WidgetRemove((tWidget *)&g_sKeyboardBackground);
        // save the value and clear
        uint32_t value = parse_int(keyboardEntryValue);
        switch (visibleField) {
        case INPUT_MOTOR_SPEED:
            usprintf(motorSpeed, "%d rpm", value);
            WidgetPaint((tWidget *)&inputMotorSpeed);
            set_motor_speed(value);
            break;
        case INPUT_CURRENT_LIMIT:
            usprintf(currentLimit, "%d mA", value);
            WidgetPaint((tWidget *)&inputCurrentLimit);
            set_current_limit(value);
            break;
        case INPUT_TEMP_LIMIT:
            usprintf(tempLimit, "%d C", value);
            WidgetPaint((tWidget *)&inputTempLimit);
            set_temp_limit(value);
            break;
        }
        keyboardEntryValue[0] = 0;
        keyboardInputIndex = 0;
        show_current_panel();
    }
    if (ui32Key >= '0' && ui32Key <= '9') {
        // this is a number - let's use it
        if (keyboardInputIndex == 0) {
            CanvasTextSet(&g_sKeyboardText, keyboardEntryValue);
        }
        keyboardEntryValue[keyboardInputIndex] = (char)ui32Key;
        // increment index, then set to pointer to 0
        keyboardEntryValue[++keyboardInputIndex] = 0;
        WidgetPaint((tWidget *)&g_sKeyboardText);
    }
}
// declare the onPress functionality for the boxes *here*
// - remove the current panel
// - hide the tab buttons
// - display the keyboard
// - handle the exit of the keyboard
// - show tab buttons
// - re-add current panel

// the values

void showKeyboard(tWidget *psWidget) {
    // remove the parent, but keep a reference
    hide_current_panel();
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sKeyboardBackground);
    WidgetPaint((tWidget *)&g_sKeyboardBackground);
    WidgetPaint((tWidget *)&g_sKeyboard);
}

void setInputAndShowKeyboard(tWidget *psWidget, INPUT_FIELDS field) {
    visibleField = field;
    showKeyboard(psWidget);
}

void showMotorSpeedKeyboard(tWidget *psWidget) {
    setInputAndShowKeyboard(psWidget, INPUT_MOTOR_SPEED);
}

void showCurrentLimitKeyboard(tWidget *psWidget) {
    setInputAndShowKeyboard(psWidget, INPUT_CURRENT_LIMIT);
}

void showTempLimitKeyboard(tWidget *psWidget) {
    setInputAndShowKeyboard(psWidget, INPUT_TEMP_LIMIT);
}

// should declare the input boxes *here* so I can reference them globally from the handle

// this is just a wee lil' dummy to handle the canvas click
RectangularButton(buttonMotorSpeed, 0, 0, 0, &g_sKentec320x240x16_SSD2119,
                      (320 + 10) / 2, 30, (320 - 20) / 2, 30,
                      PB_STYLE_RELEASE_NOTIFY, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, showMotorSpeedKeyboard);
Canvas(inputMotorSpeed, 0, 0, 0, &g_sKentec320x240x16_SSD2119,
       (320 + 10) / 2, 30, (320 - 22) / 2, 30,
       CANVAS_STYLE_TEXT | CANVAS_STYLE_TEXT_RIGHT | CANVAS_STYLE_FILL | CANVAS_STYLE_OUTLINE,
       ClrBlack, ClrBlue, ClrWhite, g_psFontCmss20, motorSpeed, 0, 0);
RectangularButton(buttonCurrentLimit, 0, 0, 0, &g_sKentec320x240x16_SSD2119,
                      (320 + 10) / 2, 60, (320 - 20) / 2, 30,
                      PB_STYLE_RELEASE_NOTIFY, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, showCurrentLimitKeyboard);
Canvas(inputCurrentLimit, 0, 0, 0, &g_sKentec320x240x16_SSD2119,
       (320 + 10) / 2, 60, (320 - 20) / 2, 30,
       CANVAS_STYLE_TEXT | CANVAS_STYLE_TEXT_RIGHT | CANVAS_STYLE_FILL | CANVAS_STYLE_OUTLINE,
       ClrBlack, ClrBlue, ClrWhite, g_psFontCmss20, currentLimit, 0, 0);
RectangularButton(buttonTempLimit, 0, 0, 0, &g_sKentec320x240x16_SSD2119,
                      (320 + 10) / 2, 90, (320 - 20) / 2, 30,
                      PB_STYLE_RELEASE_NOTIFY, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, showTempLimitKeyboard);
Canvas(inputTempLimit, 0, 0, 0, &g_sKentec320x240x16_SSD2119,
       (320 + 10) / 2, 90, (320 - 20) / 2, 30,
       CANVAS_STYLE_TEXT | CANVAS_STYLE_TEXT_RIGHT | CANVAS_STYLE_FILL | CANVAS_STYLE_OUTLINE,
       ClrBlack, ClrBlue, ClrWhite, g_psFontCmss20, tempLimit, 0, 0);

void paint_settings(tWidget *psWidget, tContext *psContext) {
    System_printf("Painting Settings");
    GrContextFontSet(psContext, g_psFontCmss20);
    GrContextForegroundSet(psContext, ClrWhite);
    GrStringDraw(psContext, "Motor Speed", -1,
                    10, 30, 0);

    GrStringDraw(psContext, "Current Limit", -1,
                    10, 60, 0);

    GrStringDraw(psContext, "Temp Limit", -1,
                    10, 90, 0);

    WidgetAdd(psWidget, (tWidget *)&buttonMotorSpeed);
    WidgetAdd(psWidget, (tWidget *)&inputMotorSpeed);
    WidgetAdd(psWidget, (tWidget *)&buttonCurrentLimit);
    WidgetAdd(psWidget, (tWidget *)&inputCurrentLimit);
    WidgetAdd(psWidget, (tWidget *)&buttonTempLimit);
    WidgetAdd(psWidget, (tWidget *)&inputTempLimit);
}
