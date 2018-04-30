#include <stdint.h>
#include <stdbool.h>
#include <grlib/grlib.h>
#include <grlib/widget.h>
#include <grlib/canvas.h>
#include <grlib/pushbutton.h>
#include "drivers/kentec320x240x16_ssd2119.h"

void paint_settings(tWidget *psWidget, tContext *psContext) {
    GrContextFontSet(psContext, g_psFontCm16);
    GrContextForegroundSet(psContext, ClrSilver);
    GrStringDraw(psContext, "Settings Panel", -1,
                    10, 30, 0);
}
