#include "app_internal.h"

ReceiverTestState_t g_receiverTest;
UltrasonicTestState_t g_ultrasonicTest;
UltrasonicEscTestState_t g_ultrasonicEscTest;
VictoryLapTestState_t g_victoryLapTest;
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

#define APP_BACKGROUND_MAX_CATCHUP        (3U)
#define APP_TEENSY_LINK_ULTRA_FLAG_VALID  (1U)

typedef struct
{
    uint32 nextServiceMs;
    uint16 controlSeq;
    TeensyLinkS32kInputs_t input;
    boolean initialized;
} AppBackgroundState_t;

static AppBackgroundState_t g_appBackground;

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
    "Teensy Link    ",
    "Victory Lap    "
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
    uint32 nowMs = t0;

    while ((uint32)(nowMs - t0) < 200u)
    {
        App_ServiceBackground(nowMs);
        nowMs = Timebase_GetMs();
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

static uint16 app_clip_u32_to_u16(uint32 value)
{
    return (value > 65535U) ? 65535U : (uint16)value;
}

static uint8 app_clip_u32_to_u8(uint32 value)
{
    return (value > 255U) ? 255U : (uint8)value;
}

static uint16 app_cm_to_cm10(float distanceCm)
{
    float cm10 = distanceCm * 10.0f;

    if (cm10 <= 0.0f)
    {
        return 0U;
    }
    if (cm10 >= 65535.0f)
    {
        return 65535U;
    }

    return (uint16)(cm10 + 0.5f);
}

static sint8 app_vision_error_pct(float error)
{
    sint32 scaled = (sint32)(error * 100.0f);

    if (scaled > 127)
    {
        scaled = 127;
    }
    if (scaled < -127)
    {
        scaled = -127;
    }

    return (sint8)scaled;
}

static void app_teensy_link_mark_camera_stale(TeensyLinkCameraResult_t *camera)
{
    if (camera == NULL_PTR)
    {
        return;
    }

    camera->errorPct = 0;
    camera->status = (uint8)VISION_TRACK_LOST;
    camera->feature = (uint8)VISION_FEATURE_NONE;
    camera->confidence = 0U;
    camera->leftLineIdx = 255U;
    camera->rightLineIdx = 255U;
    camera->ageMs = 255U;
    camera->flags = (uint8)(TEENSY_LINK_CAMERA_FLAG_SOURCE_S32K |
                            TEENSY_LINK_CAMERA_FLAG_STALE);
}

static void app_teensy_link_set_camera(TeensyLinkCameraResult_t *camera,
                                       const VisionOutput_t *vision,
                                       boolean haveFrame,
                                       uint32 ageMs)
{
    if (camera == NULL_PTR)
    {
        return;
    }

    if ((vision == NULL_PTR) || (haveFrame != TRUE))
    {
        app_teensy_link_mark_camera_stale(camera);
        return;
    }

    camera->errorPct = app_vision_error_pct(vision->error);
    camera->status = (uint8)vision->status;
    camera->feature = (uint8)vision->feature;
    camera->confidence = vision->confidence;
    camera->leftLineIdx = vision->leftLineIdx;
    camera->rightLineIdx = vision->rightLineIdx;
    camera->ageMs = app_clip_u32_to_u8(ageMs);
    camera->flags = (uint8)(TEENSY_LINK_CAMERA_FLAG_SOURCE_S32K |
                            TEENSY_LINK_CAMERA_FLAG_VALID);
}

static void app_teensy_link_apply_ultra(TeensyLinkS32kInputs_t *in,
                                        boolean haveDistance,
                                        float distanceCm,
                                        uint32 lastDistanceMs,
                                        uint32 nowMs)
{
    if (in == NULL_PTR)
    {
        return;
    }

    if (haveDistance == TRUE)
    {
        in->ultrasonicDistanceCm10 = app_cm_to_cm10(distanceCm);
        in->ultrasonicAgeMs = app_clip_u32_to_u16(nowMs - lastDistanceMs);
        in->ultrasonicFlags = APP_TEENSY_LINK_ULTRA_FLAG_VALID;
    }
    else
    {
        in->ultrasonicDistanceCm10 = 0U;
        in->ultrasonicAgeMs = 65535U;
        in->ultrasonicFlags = 0U;
    }
}

static void app_teensy_link_apply_camservo(TeensyLinkS32kInputs_t *in,
                                           const CamServoState_t *st,
                                           uint32 nowMs)
{
    uint32 ageMs = 255U;

    if ((in == NULL_PTR) || (st == NULL_PTR))
    {
        return;
    }

    if ((st->haveValidVision == TRUE) && (st->lastFrameMs != 0U))
    {
        ageMs = nowMs - st->lastFrameMs;
    }
    else if (st->haveValidVision == TRUE)
    {
        ageMs = 0U;
    }

    app_teensy_link_set_camera(&in->camera[0], &st->result, st->haveValidVision, ageMs);
    in->steerRaw = st->steerRaw;
    in->steerFilt = st->steerFilt;
    in->steerOut = st->steerOut;
    in->servoCmd = st->steerOut;
}

static void app_teensy_link_apply_linear_test(TeensyLinkS32kInputs_t *in)
{
    if (in == NULL_PTR)
    {
        return;
    }

    app_teensy_link_set_camera(&in->camera[0],
                               &g_linearCameraTest.result,
                               g_linearCameraTest.haveValidVision,
                               0U);
    in->steerRaw = g_linearCameraTest.steerRaw;
    in->steerFilt = g_linearCameraTest.steerFilt;
    in->steerOut = g_linearCameraTest.steerOut;
    in->servoCmd = g_linearCameraTest.steerOut;
}

static void app_teensy_link_fill_defaults(TeensyLinkS32kInputs_t *in)
{
    if (in == NULL_PTR)
    {
        return;
    }

    (void)memset(in, 0, sizeof(*in));
    app_teensy_link_mark_camera_stale(&in->camera[0]);
    app_teensy_link_mark_camera_stale(&in->camera[1]);
    in->ultrasonicAgeMs = 65535U;
}

static void app_teensy_link_fill_mode_input(uint32 nowMs, TeensyLinkS32kInputs_t *in)
{
    AppBuildMode_t buildMode;

    app_teensy_link_fill_defaults(in);

    if (in == NULL_PTR)
    {
        return;
    }

    buildMode = App_GetSelectedBuildMode();
    in->controlLoopSeq = g_appBackground.controlSeq;
    in->controlDtUs = (uint16)((uint16)TEENSY_LINK_SERVICE_PERIOD_MS * 1000U);
    in->appMode = (uint8)buildMode;

    switch (buildMode)
    {
        case APP_BUILD_MODE_NXP_CUP:
            in->appState = (uint8)g_nxpCup.state;
            app_teensy_link_apply_camservo(in, &g_nxpCup.camSt, nowMs);
            in->targetSpeedPct = (sint16)g_nxpCup.autoSpeedPct;
            in->currentSpeedPct = (sint16)g_nxpCup.autoSpeedPct;
            in->escPrimaryLogical = (sint16)(-g_nxpCup.autoSpeedPct);
            in->escSecondaryLogical = (sint16)(-g_nxpCup.autoSpeedPct);
            app_teensy_link_apply_ultra(in,
                                        g_nxpCup.ultraSt.haveValidDistance,
                                        g_nxpCup.ultraSt.lastDistanceCm,
                                        g_nxpCup.ultraSt.lastDistanceMs,
                                        nowMs);
            break;

        case APP_BUILD_MODE_RACE_MODE:
            in->appState = (uint8)g_raceMode.phase;
            app_teensy_link_set_camera(&in->camera[0],
                                       &g_raceMode.result,
                                       g_raceMode.haveValidVision,
                                       0U);
            in->steerRaw = g_raceMode.steerRaw;
            in->steerFilt = g_raceMode.steerFilt;
            in->steerOut = g_raceMode.steerOut;
            in->targetSpeedPct = (sint16)g_raceMode.targetSpeedPct;
            in->currentSpeedPct = (sint16)g_raceMode.currentSpeedPct;
            in->escPrimaryLogical = (sint16)(-g_raceMode.currentSpeedPct);
            in->escSecondaryLogical = (sint16)(-g_raceMode.currentSpeedPct);
            in->servoCmd = g_raceMode.steerOut;
            app_teensy_link_apply_ultra(in,
                                        g_raceMode.haveValidDistance,
                                        g_raceMode.lastDistanceCm,
                                        g_raceMode.lastDistanceMs,
                                        nowMs);
            break;

        case APP_BUILD_MODE_HONOR_LAP:
            app_teensy_link_apply_linear_test(in);
            in->targetSpeedPct = (sint16)g_honorLap.commandedSpeedPct;
            in->currentSpeedPct = (sint16)g_honorLap.commandedSpeedPct;
            in->escPrimaryLogical = (sint16)(-g_honorLap.commandedSpeedPct);
            in->escSecondaryLogical = (sint16)(-g_honorLap.commandedSpeedPct);
            app_teensy_link_apply_ultra(in,
                                        g_honorLap.hasValidDistance,
                                        g_honorLap.lastDistanceCm,
                                        g_honorLap.lastDistanceMs,
                                        nowMs);
            break;

        case APP_BUILD_MODE_LINEAR_CAMERA_TEST:
            app_teensy_link_apply_linear_test(in);
            break;

        case APP_BUILD_MODE_SERVO_RATE_TEST:
            in->appState = (uint8)g_servoRateTest.freqId;
            in->steerRaw = g_servoRateTest.command;
            in->steerFilt = g_servoRateTest.command;
            in->steerOut = g_servoRateTest.command;
            in->servoCmd = g_servoRateTest.command;
            break;

        case APP_BUILD_MODE_TEENSY_LINK_TEST:
            in->appState = (uint8)RUNTIME_TEST_TEENSY_LINK;
            break;

        case APP_BUILD_MODE_NXP_CUP_TESTS:
            in->appState =
                (g_testsMenu.testActive == TRUE) ? (uint8)g_testsMenu.activeTest : g_testsMenu.selectedIndex;

            if (g_testsMenu.testActive == TRUE)
            {
                switch (g_testsMenu.activeTest)
                {
                    case RUNTIME_TEST_LINEAR_CAMERA:
                    case RUNTIME_TEST_CAMSERVO:
                        app_teensy_link_apply_linear_test(in);
                        break;

                    case RUNTIME_TEST_SERVO:
                        in->steerRaw = g_servoTest.steerRaw;
                        in->steerFilt = g_servoTest.steerFilt;
                        in->steerOut = g_servoTest.steerOut;
                        in->servoCmd = g_servoTest.steerOut;
                        break;

                    case RUNTIME_TEST_ESC:
                        in->targetSpeedPct = g_escTest.commandPct;
                        in->currentSpeedPct = g_escTest.commandPct;
                        in->escPrimaryLogical = g_escTest.primaryCmdPct;
                        in->escSecondaryLogical = g_escTest.secondaryCmdPct;
                        break;

                    case RUNTIME_TEST_ULTRASONIC:
                        app_teensy_link_apply_ultra(in,
                                                    g_ultrasonicTest.haveValidDistance,
                                                    g_ultrasonicTest.lastDistanceCm,
                                                    nowMs,
                                                    nowMs);
                        break;

                    case RUNTIME_TEST_ULTRA_ESC:
                        in->targetSpeedPct = (sint16)g_ultrasonicEscTest.commandedSpeedPct;
                        in->currentSpeedPct = (sint16)g_ultrasonicEscTest.commandedSpeedPct;
                        in->escPrimaryLogical = (sint16)g_ultrasonicEscTest.commandedSpeedPct;
                        in->escSecondaryLogical = (sint16)g_ultrasonicEscTest.commandedSpeedPct;
                        app_teensy_link_apply_ultra(in,
                                                    g_ultrasonicEscTest.hasValidDistance,
                                                    g_ultrasonicEscTest.lastDistanceCm,
                                                    g_ultrasonicEscTest.lastDistanceMs,
                                                    nowMs);
                        break;

                    case RUNTIME_TEST_SIMPLE_DRIVE:
                        app_teensy_link_apply_camservo(in, &g_simpleDriveTest.camSt, nowMs);
                        in->targetSpeedPct = (sint16)g_simpleDriveTest.autoSpeedPct;
                        in->currentSpeedPct = (sint16)g_simpleDriveTest.autoSpeedPct;
                        in->escPrimaryLogical = (sint16)(-g_simpleDriveTest.autoSpeedPct);
                        in->escSecondaryLogical = (sint16)(-g_simpleDriveTest.autoSpeedPct);
                        break;

                    case RUNTIME_TEST_VICTORY_LAP:
                        in->targetSpeedPct = (sint16)g_victoryLapTest.commandedSpeedPct;
                        in->currentSpeedPct = (sint16)g_victoryLapTest.commandedSpeedPct;
                        in->escPrimaryLogical = (sint16)g_victoryLapTest.commandedSpeedPct;
                        in->escSecondaryLogical = (sint16)g_victoryLapTest.commandedSpeedPct;
                        app_teensy_link_apply_ultra(in,
                                                    g_victoryLapTest.hasValidDistance,
                                                    g_victoryLapTest.lastDistanceCm,
                                                    g_victoryLapTest.lastDistanceMs,
                                                    nowMs);
                        break;

                    default:
                        break;
                }
            }
            break;

        default:
            break;
    }
}

void App_InitBackgroundServices(uint32 nowMs)
{
    (void)memset(&g_appBackground, 0, sizeof(g_appBackground));
    TeensyLink_Init();
    g_appBackground.nextServiceMs = nowMs;
    g_appBackground.initialized = TRUE;
}

void App_ServiceBackground(uint32 nowMs)
{
    uint8 catchup = APP_BACKGROUND_MAX_CATCHUP;

    if (g_appBackground.initialized != TRUE)
    {
        return;
    }

    while ((catchup != 0U) &&
           (time_reached(nowMs, g_appBackground.nextServiceMs) == TRUE))
    {
        g_appBackground.controlSeq++;
        app_teensy_link_fill_mode_input(nowMs, &g_appBackground.input);
        (void)TeensyLink_Service5ms(nowMs, &g_appBackground.input);
        g_appBackground.nextServiceMs += (uint32)TEENSY_LINK_SERVICE_PERIOD_MS;
        catchup--;
    }

    if (time_reached(nowMs, g_appBackground.nextServiceMs) == TRUE)
    {
        g_appBackground.nextServiceMs = nowMs + (uint32)TEENSY_LINK_SERVICE_PERIOD_MS;
    }
}

void App_InitRuntimeCore(void)
{
    RuntimeTune_InitDefaults();
    Board_InitDrivers();
    Timebase_Init();
    App_InitBackgroundServices(Timebase_GetMs());
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
