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

/* Editable settings */
typedef struct
{
    uint32 camExposureTicks;
    uint16 ultraStopCm;
    uint8  baseSpeedPct;
    float  kp;
    float  kd;
    float  steerScale;
} UiSettingsType;

void UI_Init(void);

void UI_Input(boolean sw2Pressed, boolean sw3Pressed, uint8 pot);

/* NEW: speed telemetry for RUN HUD */
void UI_SetTelemetry(int speedPct);

/* UI draw (menu/terminal in IDLE; HUD-only in RUN) */
void UI_Task(Ultrasonic_StatusType ultraStatus,
             uint32 ultraIrqCount,
             uint32 ultraTicks,
             Braking_StateType brakeState,
             float lastDistanceCm);

boolean   UI_IsInTerminal(void);
UiPageType UI_GetActivePage(void);

/* Settings access */
const UiSettingsType* UI_GetSettings(void);

/* Run state for header (I/R/F) */
void UI_SetRunState(boolean running, boolean fail);

#endif /* USER_INTERFACE_H */
