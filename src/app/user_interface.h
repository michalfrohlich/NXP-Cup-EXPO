#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#include "Platform_Types.h"
#include "ultrasonic.h"
#include "../services/braking.h"

/* What UI page is currently selected */
typedef enum
{
    UI_PAGE_MENU = 0,
    UI_PAGE_ULTRASONIC,
    UI_PAGE_SERVO_MANUAL,
    UI_PAGE_ESC_MANUAL,
    UI_PAGE_VISION_DEBUG,
    UI_PAGE_FULL_AUTO,
    UI_PAGE_SETTINGS
} UiPageType;

/* Initialize internal UI state */
void UI_Init(void);

/* Give UI the one-shot button events + pot value.
   IMPORTANT:
   - app_modes.c should call Buttons_Update()
   - app_modes.c should call Buttons_WasPressed()
   - then pass those events here
*/
void UI_Input(boolean sw2Pressed, boolean sw3Pressed, uint8 pot);

/* UI draws the screen (menu/terminal pages).
   It does NOT read buttons or pot directly. */
void UI_Task(Ultrasonic_StatusType ultraStatus,
             uint32 ultraIrqCount,
             uint32 ultraTicks,
             Braking_StateType brakeState,
             float lastDistanceCm);

/* The currently selected page (when user enters a terminal) */
UiPageType UI_GetSelectedPage(void);

/* Are we currently inside a terminal (not menu)? */
boolean UI_IsInTerminal(void);

#endif /* USER_INTERFACE_H */
