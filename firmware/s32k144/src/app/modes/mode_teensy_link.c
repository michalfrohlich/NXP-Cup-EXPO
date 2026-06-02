#include "../app_internal.h"

void mode_teensy_link_test(void)
{
    uint32 nextButtonsMs;

    App_InitRuntimeCommon();
    teensy_link_test_enter(Timebase_GetMs());
    nextButtonsMs = Timebase_GetMs();

    for (;;)
    {
        uint32 nowMs = Timebase_GetMs();
        boolean sw2Pressed;

        while (time_reached(nowMs, nextButtonsMs) == TRUE)
        {
            Buttons_Update();
            nextButtonsMs += BUTTONS_PERIOD_MS;
        }

        sw2Pressed = Buttons_WasPressed(BUTTON_ID_SW2);
        (void)Buttons_WasPressed(BUTTON_ID_SW3);

        teensy_link_test_update(nowMs, sw2Pressed);
    }
}
