#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#include "Platform_Types.h"
#include "ultrasonic.h"
#include "../services/braking.h"

/* Pages (modes) user can select */
typedef enum
{
    UI_PAGE_NONE = 0,
    UI_PAGE_ULTRASONIC,
    UI_PAGE_SERVO_MANUAL,
    UI_PAGE_ESC_MANUAL,
    UI_PAGE_VISION_DEBUG,
    UI_PAGE_FULL_AUTO,
    UI_PAGE_SETTINGS
} UiPageType;

/* Editable settings (kept SMALL to fit display) */
typedef struct
{
    /* Camera */
    uint32 camExposureTicks;   /* used in LinearCameraGetFrameEx */

    /* Safety */
    uint16 ultraStopCm;        /* stop/fail if distance <= this */

    /* Driving */
    uint8  baseSpeedPct;       /* used by full-auto controller */

    /* Steering PID (simple + effective) */
    float  kp;
    float  kd;

    /* Extra: steering strength multiplier (commit more to one side) */
    float  steerScale;
} UiSettingsType;

void UI_Init(void);

/* SW2 = UI interaction (select/edit/back)
   SW3 = Run/Stop (UI shows, but does not “consume” it)
   POT = navigation/adjust
*/
void UI_Input(boolean sw2Pressed, boolean sw3Pressed, uint8 pot);

/* UI draw (menu or terminal) */
void UI_Task(Ultrasonic_StatusType ultraStatus,
             uint32 ultraIrqCount,
             uint32 ultraTicks,
             Braking_StateType brakeState,
             float lastDistanceCm);

/* Info for app_modes.c (so ONLY ONE draws on the display) */
boolean    UI_IsInTerminal(void);
UiPageType  UI_GetActivePage(void);

/* New helpers so app_modes can decide who owns the screen */
boolean UI_IsLiveView(void);

/* Settings access */
const UiSettingsType* UI_GetSettings(void);

/* Run state for header (I/R/F) */
void UI_SetRunState(boolean running, boolean fail);

#endif /* USER_INTERFACE_H */
