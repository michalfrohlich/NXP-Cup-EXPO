#include "app_internal.h"

ReceiverTestState_t g_receiverTest;
TeensyImuTestState_t g_teensyImuTest;
UltrasonicTestState_t g_ultrasonicTest;
UltrasonicEscTestState_t g_ultrasonicEscTest;
ServoTestState_t g_servoTest;
ServoRateTestState_t g_servoRateTest;
EscManualTestState_t g_escTest;
LinearCameraTestState_t g_linearCameraTest;
SimpleDriveState_t g_simpleDriveTest;
TestsMenuState_t g_testsMenu;
HonorLapState_t g_honorLap;
NxpCupState_t g_nxpCup;
RaceModeState_t g_raceMode;
SerialTuneState_t g_serialTune;
RuntimeTuneState_t g_runtimeTune;

const CamTuneProfile_t g_nxpCupProfiles[NXP_CUP_PROFILE_COUNT] =
{
    {
        NXP_CUP_SUPERFAST_KP,
        NXP_CUP_SUPERFAST_KD,
        NXP_CUP_SUPERFAST_KI,
        NXP_CUP_SUPERFAST_ITERM_CLAMP,
        NXP_CUP_SUPERFAST_STEER_LPF_ALPHA,
        (sint16)NXP_CUP_SUPERFAST_STEER_CLAMP,
        (sint16)NXP_CUP_SUPERFAST_STEER_RATE_MAX,
        (uint8)NXP_CUP_SUPERFAST_SPEED_PCT
    },
    {
        NXP_CUP_5050_KP,
        NXP_CUP_5050_KD,
        NXP_CUP_5050_KI,
        NXP_CUP_5050_ITERM_CLAMP,
        NXP_CUP_5050_STEER_LPF_ALPHA,
        (sint16)NXP_CUP_5050_STEER_CLAMP,
        (sint16)NXP_CUP_5050_STEER_RATE_MAX,
        (uint8)NXP_CUP_5050_SPEED_PCT
    },
    {
        NXP_CUP_SLOW_KP,
        NXP_CUP_SLOW_KD,
        NXP_CUP_SLOW_KI,
        NXP_CUP_SLOW_ITERM_CLAMP,
        NXP_CUP_SLOW_STEER_LPF_ALPHA,
        (sint16)NXP_CUP_SLOW_STEER_CLAMP,
        (sint16)NXP_CUP_SLOW_STEER_RATE_MAX,
        (uint8)NXP_CUP_SLOW_SPEED_PCT
    }
};

const char g_testsMenuItems[RUNTIME_TEST_COUNT][17] =
{
    "Camera         ",
    "ESC            ",
    "Servo          ",
    "Ultrasonic     ",
    "Cam+Servo      ",
    "Simple test drv",
    "Serial tune    ",
    "Ultra+ESC      ",
    "Receiver - x   ",
    "Teensy IMU     "
};

const uint16 g_servoRateTestFreqHz[SERVO_RATE_TEST_FREQ_COUNT] =
{
    10U,
    50U,
    100U,
    250U
};

const uint16 g_servoRateTestPeriodMs[SERVO_RATE_TEST_FREQ_COUNT] =
{
    100U,
    20U,
    10U,
    4U
};

const char g_servoRateTestMotionText[SERVO_RATE_TEST_MOTION_COUNT][12] =
{
    "POT",
    "SWEEP FINE",
    "SWEEP STEP"
};
boolean time_reached(uint32 nowMs, uint32 dueMs)
{
    return ((uint32)(nowMs - dueMs) < 0x80000000u) ? TRUE : FALSE;
}

void display_power_stabilize_delay(void)
{
    uint32 t0 = Timebase_GetMs();

    while ((uint32)(Timebase_GetMs() - t0) < 200u)
    {
    }
}

void busy_delay(volatile uint32 ticks)
{
    while (ticks != 0U)
    {
        ticks--;
    }
}

uint16 VisionDebug_WhiteMaxFromPot(uint8 potLevel)
{
    uint32 range = (uint32)VDBG_WHITE_MAX_FULL_SCALE - (uint32)VDBG_WHITE_MAX_MIN_ZOOM;
    uint32 scaled = ((uint32)potLevel * range) / 255U;
    uint32 whiteMax = (uint32)VDBG_WHITE_MAX_FULL_SCALE - scaled;

    if (whiteMax < (uint32)VDBG_WHITE_MAX_MIN_ZOOM)
    {
        whiteMax = (uint32)VDBG_WHITE_MAX_MIN_ZOOM;
    }
    if (whiteMax > (uint32)VDBG_WHITE_MAX_FULL_SCALE)
    {
        whiteMax = (uint32)VDBG_WHITE_MAX_FULL_SCALE;
    }

    return (uint16)whiteMax;
}

Sw2Action_t ButtonTracker_Update(Sw2Tracker_t *st, ButtonId_t id, uint32 nowMs)
{
    boolean pressedNow;

    if (st == NULL_PTR)
    {
        return SW2_ACTION_NONE;
    }

    pressedNow = Buttons_IsPressed(id);

    if ((pressedNow == TRUE) && (st->wasPressed != TRUE))
    {
        st->wasPressed = TRUE;
        st->holdHandled = FALSE;
        st->pressedSinceMs = nowMs;
    }

    if ((pressedNow == TRUE) && (st->holdHandled != TRUE))
    {
        if ((uint32)(nowMs - st->pressedSinceMs) >= CAM_DEBUG_PAUSE_HOLD_MS)
        {
            st->holdHandled = TRUE;
            return SW2_ACTION_HOLD;
        }
    }

    if ((pressedNow != TRUE) && (st->wasPressed == TRUE))
    {
        boolean wasHoldHandled = st->holdHandled;

        st->wasPressed = FALSE;
        st->holdHandled = FALSE;
        st->pressedSinceMs = 0U;

        if (wasHoldHandled != TRUE)
        {
            return SW2_ACTION_CLICK;
        }
    }

    return SW2_ACTION_NONE;
}

Sw2Action_t Sw2Tracker_Update(Sw2Tracker_t *st, uint32 nowMs)
{
    return ButtonTracker_Update(st, BUTTON_ID_SW2, nowMs);
}

void StatusLed_Blue(void)
{
    RgbLed_ChangeColor((RgbLed_Color){ .r = false, .g = false, .b = true });
}

void StatusLed_Green(void)
{
    RgbLed_ChangeColor((RgbLed_Color){ .r = false, .g = true, .b = false });
}

void StatusLed_Cyan(void)
{
    RgbLed_ChangeColor((RgbLed_Color){ .r = false, .g = true, .b = true });
}

void StatusLed_Yellow(void)
{
    RgbLed_ChangeColor((RgbLed_Color){ .r = true, .g = true, .b = false });
}

void StatusLed_Red(void)
{
    RgbLed_ChangeColor((RgbLed_Color){ .r = true, .g = false, .b = false });
}

void StatusLed_Off(void)
{
    RgbLed_ChangeColor((RgbLed_Color){ .r = false, .g = false, .b = false });
}

void Esc_StopNeutral(void)
{
    Esc_SetBrake(0U, 0U);
    Esc_SetSpeed(ESC_TRUE_NEUTRAL_CMD, ESC_TRUE_NEUTRAL_CMD);
}

static int esc_apply_neutral_offset(int logicalCmd)
{
    int physicalCmd = logicalCmd + ESC_TRUE_NEUTRAL_CMD;

    if (physicalCmd > 100)
    {
        physicalCmd = 100;
    }
    if (physicalCmd < -100)
    {
        physicalCmd = -100;
    }

    return physicalCmd;
}

void Esc_SetLogicalSpeed(int primaryLogicalCmd, int secondaryLogicalCmd)
{
    const int primaryPhysicalCmd = esc_apply_neutral_offset(primaryLogicalCmd);
    const int secondaryPhysicalCmd = esc_apply_neutral_offset(secondaryLogicalCmd);

    Esc_SetSpeed(primaryPhysicalCmd, secondaryPhysicalCmd);
}

void RuntimeTune_InitDefaults(void)
{
    if (g_runtimeTune.initialized == TRUE)
    {
        return;
    }

    g_runtimeTune.profile.kp = KP;
    g_runtimeTune.profile.kd = KD;
    g_runtimeTune.profile.ki = KI;
    g_runtimeTune.profile.iTermClamp = ITERM_CLAMP;
    g_runtimeTune.profile.steerLpfAlpha = SERVO_TEST_LPF_ALPHA;
    g_runtimeTune.profile.steerClamp = (sint16)SERVO_TEST_CMD_CLAMP;
    g_runtimeTune.profile.steerRateMax = (sint16)SERVO_TEST_RATE_MAX;
    g_runtimeTune.profile.baseSpeedPct = 20U;
    g_runtimeTune.initialized = TRUE;
}

void App_InitRuntimeCore(void)
{
    RuntimeTune_InitDefaults();
    Board_InitDrivers();
    Timebase_Init();
    OnboardPot_Init();
    StatusLed_Off();
}

void App_InitRuntimeCommon(void)
{
    App_InitRuntimeCore();
    display_power_stabilize_delay();
    DisplayInit(0U, STD_ON);
    DisplayClear();
    DisplayRefresh();
}

void DisplayTextPadded(uint16 displayLine, const char *text)
{
    char lineBuf[17];
    uint16 textLen = 0U;

    (void)memset(lineBuf, ' ', 16U);
    lineBuf[16] = '\0';

    while ((text[textLen] != '\0') && (textLen < 16U))
    {
        lineBuf[textLen] = text[textLen];
        textLen++;
    }

    DisplayText(displayLine, lineBuf, 16U, 0U);
}
