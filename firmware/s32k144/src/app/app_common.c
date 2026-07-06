#include "app_internal.h"

UltrasonicTestState_t g_ultrasonicTest;
UltrasonicEscTestState_t g_ultrasonicEscTest;
VictoryLapTestState_t g_victoryLapTest;
ServoTestState_t g_servoTest;
EscManualTestState_t g_escTest;
LinearCameraTestState_t g_linearCameraTest;
SimpleDriveState_t g_simpleDriveTest;
RuntimeTuneState_t g_runtimeTune;

static boolean g_appRuntimeCoreInitialized = FALSE;
static boolean g_appRuntimeDisplay_Initialized = FALSE;

const DriveTuneProfile_t g_speedModeProfiles[SPEED_MODE_COUNT] = {
    {SPEED_MODE_SUPERFAST_KP, SPEED_MODE_SUPERFAST_KD, SPEED_MODE_SUPERFAST_KI,
     SPEED_MODE_SUPERFAST_ITERM_CLAMP, SPEED_MODE_SUPERFAST_STEER_LPF_ALPHA,
     (sint16)SPEED_MODE_SUPERFAST_STEER_CLAMP, (sint16)SPEED_MODE_SUPERFAST_STEER_RATE_MAX,
     (uint8)SPEED_MODE_SUPERFAST_SPEED_PCT},
    {SPEED_MODE_5050_KP, SPEED_MODE_5050_KD, SPEED_MODE_5050_KI, SPEED_MODE_5050_ITERM_CLAMP,
     SPEED_MODE_5050_STEER_LPF_ALPHA, (sint16)SPEED_MODE_5050_STEER_CLAMP,
     (sint16)SPEED_MODE_5050_STEER_RATE_MAX, (uint8)SPEED_MODE_5050_SPEED_PCT},
    {SPEED_MODE_SLOW_KP, SPEED_MODE_SLOW_KD, SPEED_MODE_SLOW_KI, SPEED_MODE_SLOW_ITERM_CLAMP,
     SPEED_MODE_SLOW_STEER_LPF_ALPHA, (sint16)SPEED_MODE_SLOW_STEER_CLAMP,
     (sint16)SPEED_MODE_SLOW_STEER_RATE_MAX, (uint8)SPEED_MODE_SLOW_SPEED_PCT}};

SpeedModeSetting_t SpeedMode_FromPot(uint8 potLevel)
{
    if (potLevel < 85U)
    {
        return SPEED_MODE_SUPERFAST;
    }
    if (potLevel < 171U)
    {
        return SPEED_MODE_5050;
    }
    return SPEED_MODE_SLOW;
}

const char *SpeedMode_GetName(SpeedModeSetting_t setting)
{
    switch (setting)
    {
        case SPEED_MODE_SUPERFAST:
            return "SUPERFAST";
        case SPEED_MODE_5050:
            return "5050";
        case SPEED_MODE_SLOW:
            return "SLOW";
        default:
            return "INVALID";
    }
}

const DriveTuneProfile_t *SpeedMode_GetProfile(SpeedModeSetting_t setting)
{
    if ((uint8)setting >= (uint8)SPEED_MODE_COUNT)
    {
        return NULL_PTR;
    }
    return &g_speedModeProfiles[(uint8)setting];
}

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

Sw2Action_t ButtonTracker_Update(ButtonHoldTracker_t *st, ButtonId_t id, uint32 nowMs)
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

void StatusLed_Blue(void)
{
    RgbLed_ChangeColor((RgbLed_Color){.r = false, .g = false, .b = true});
}

void StatusLed_Green(void)
{
    RgbLed_ChangeColor((RgbLed_Color){.r = false, .g = true, .b = false});
}

void StatusLed_Cyan(void)
{
    RgbLed_ChangeColor((RgbLed_Color){.r = false, .g = true, .b = true});
}

void StatusLed_Yellow(void)
{
    RgbLed_ChangeColor((RgbLed_Color){.r = true, .g = true, .b = false});
}

void StatusLed_White(void)
{
    RgbLed_ChangeColor((RgbLed_Color){.r = true, .g = true, .b = true});
}

void StatusLed_Red(void)
{
    RgbLed_ChangeColor((RgbLed_Color){.r = true, .g = false, .b = false});
}

void StatusLed_Off(void)
{
    RgbLed_ChangeColor((RgbLed_Color){.r = false, .g = false, .b = false});
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

    g_runtimeTune.profile.kp = STEERING_DEFAULT_KP;
    g_runtimeTune.profile.kd = STEERING_DEFAULT_KD;
    g_runtimeTune.profile.ki = STEERING_DEFAULT_KI;
    g_runtimeTune.profile.iTermClamp = STEERING_DEFAULT_I_TERM_CLAMP;
    g_runtimeTune.profile.steerLpfAlpha = SERVO_TEST_LPF_ALPHA;
    g_runtimeTune.profile.steerClamp = (sint16)SERVO_TEST_CMD_CLAMP;
    g_runtimeTune.profile.steerRateMax = (sint16)SERVO_TEST_RATE_MAX;
    g_runtimeTune.profile.baseSpeedPct = 20U;
    g_runtimeTune.lineDetector.minContrast = VISION_LINEAR_MIN_CONTRAST;
    g_runtimeTune.lineDetector.edgeHighPct = VISION_LINEAR_EDGE_HIGH_PCT;
    g_runtimeTune.lineDetector.edgeLowPct = VISION_LINEAR_EDGE_LOW_PCT;
    g_runtimeTune.lineDetectorGeneration = 1U;
    g_runtimeTune.initialized = TRUE;
}

boolean RuntimeTune_SetLineDetectorParams(const LineDetectorParams_t *params)
{
    uint16 nextGeneration;

    if ((params == NULL_PTR) || (params->minContrast > 4095U) || (params->edgeHighPct < 1U) ||
        (params->edgeHighPct > 100U) || (params->edgeLowPct < 1U) || (params->edgeLowPct > 100U))
    {
        return FALSE;
    }

    RuntimeTune_InitDefaults();
    if ((params->minContrast == g_runtimeTune.lineDetector.minContrast) &&
        (params->edgeHighPct == g_runtimeTune.lineDetector.edgeHighPct) &&
        (params->edgeLowPct == g_runtimeTune.lineDetector.edgeLowPct))
    {
        return TRUE;
    }

    nextGeneration = (uint16)(g_runtimeTune.lineDetectorGeneration + 1U);
    if (nextGeneration == 0U)
    {
        nextGeneration = 1U;
    }

    g_runtimeTune.lineDetector = *params;
    g_runtimeTune.lineDetectorGeneration = nextGeneration;
    return TRUE;
}

void RuntimeTune_ApplySpeedMode(SpeedModeSetting_t setting)
{
    const DriveTuneProfile_t *profile = SpeedMode_GetProfile(setting);

    RuntimeTune_InitDefaults();
    if (profile != NULL_PTR)
    {
        g_runtimeTune.profile = *profile;
    }
}

void App_ApplySteeringProfile(SteeringControllerState_t *state, const DriveTuneProfile_t *profile)
{
    SteeringControllerConfig_t config;

    if ((state == NULL_PTR) || (profile == NULL_PTR))
    {
        return;
    }

    config.kp = profile->kp;
    config.ki = profile->ki;
    config.kd = profile->kd;
    config.iTermClamp = profile->iTermClamp;
    config.steerScale = 1.0f;
    SteeringController_ApplyConfig(state, &config);
}

static const char *App_GetSelectedBuildModeMacroName(void)
{
    switch (App_GetSelectedBuildMode())
    {
        case APP_BUILD_MODE_LINEAR_CAMERA_TEST:
            return "APP_TEST_LINEAR_CAMERA_TEST";

        case APP_BUILD_MODE_ESC_TEST:
            return "APP_TEST_ESC_TEST";

        case APP_BUILD_MODE_RACE_S32K_ONLY:
            return "APP_RACE_S32K_ONLY";

        case APP_BUILD_MODE_RACE:
            return "APP_RACE";

        case APP_BUILD_MODE_SERVO_RATE_TEST:
            return "APP_TEST_SERVO_RATE_TEST";

        case APP_BUILD_MODE_TEENSY_LINK_TEST:
            return "APP_TEST_TEENSY_LINK_TEST";

        case APP_BUILD_MODE_ESP_LINK_TEST:
            return "APP_TEST_ESP_LINK_TEST";

        case APP_BUILD_MODE_NXP_CUP_TESTS:
            return "APP_TEST_NXP_CUP_TESTS";

        default:
            return "APP_TEST_UNKNOWN";
    }
}

static const char *App_GetSelectedBuildModeDisplayName(void)
{
    switch (App_GetSelectedBuildMode())
    {
        case APP_BUILD_MODE_LINEAR_CAMERA_TEST:
            return "Camera-s32k";

        case APP_BUILD_MODE_ESC_TEST:
            return "ESC test";

        case APP_BUILD_MODE_RACE_S32K_ONLY:
            return "Race S32K";

        case APP_BUILD_MODE_RACE:
            return "Race Full";

        case APP_BUILD_MODE_SERVO_RATE_TEST:
            return "Servo Rate";

        case APP_BUILD_MODE_TEENSY_LINK_TEST:
            return "Teensy Link";

        case APP_BUILD_MODE_ESP_LINK_TEST:
            return "ESP Link";

        case APP_BUILD_MODE_NXP_CUP_TESTS:
            return "Bench Menu";

        default:
            return "Unknown";
    }
}

static const char *App_GetSelectedBuildModeShortName(void)
{
    switch (App_GetSelectedBuildMode())
    {
        case APP_BUILD_MODE_LINEAR_CAMERA_TEST:
            return "LINEAR_CAMERA";

        case APP_BUILD_MODE_ESC_TEST:
            return "ESC_TEST";

        case APP_BUILD_MODE_RACE_S32K_ONLY:
            return "RACE_S32K";

        case APP_BUILD_MODE_RACE:
            return "RACE_FULL";

        case APP_BUILD_MODE_SERVO_RATE_TEST:
            return "SERVO_RATE";

        case APP_BUILD_MODE_TEENSY_LINK_TEST:
            return "TEENSY_LINK";

        case APP_BUILD_MODE_ESP_LINK_TEST:
            return "ESP_LINK";

        case APP_BUILD_MODE_NXP_CUP_TESTS:
            return "NXP_CUP_TESTS";

        default:
            return "UNKNOWN";
    }
}

static void App_SetBootLedStep(uint32 step)
{
    switch (step % 5U)
    {
        case 0U:
            StatusLed_Red();
            break;

        case 1U:
            StatusLed_Yellow();
            break;

        case 2U:
            StatusLed_Green();
            break;

        case 3U:
            StatusLed_Cyan();
            break;

        default:
            StatusLed_Blue();
            break;
    }
}

void App_InitRuntimeCore(void)
{
    RuntimeTune_InitDefaults();

    if (g_appRuntimeCoreInitialized != TRUE)
    {
        Board_InitDrivers();
        Timebase_Init();
        OnboardPot_Init();
        g_appRuntimeCoreInitialized = TRUE;
    }

    StatusLed_Off();
}

void App_InitRuntimeCommon(void)
{
    App_InitRuntimeCore();

    if (g_appRuntimeDisplay_Initialized != TRUE)
    {
        display_power_stabilize_delay();
        Display_Init(0U, STD_ON);
        g_appRuntimeDisplay_Initialized = TRUE;
    }

    DisplayClear();
    Display_Refresh();
}

void App_RunBootBanner(void)
{
    const char *macroName;
    const char *displayName;
    const char *shortName;
    uint32 startMs;
    uint32 lastLedStep = 0xFFFFFFFFU;

    App_InitRuntimeCommon();

    macroName = App_GetSelectedBuildModeMacroName();
    displayName = App_GetSelectedBuildModeDisplayName();
    shortName = App_GetSelectedBuildModeShortName();

    UartHost_WriteLine("");
    UartHost_WriteLine("=== S32K BOOT ===");
    UartHost_WriteString("Build: ");
    UartHost_WriteLine(macroName);
    UartHost_WriteString("Mode: ");
    UartHost_WriteLine(displayName);

    DisplayClear();
    DisplayTextPadded(0U, "S32K BOOT");
    DisplayTextPadded(1U, displayName);
    DisplayTextPadded(2U, shortName);
    Display_Refresh();

    startMs = Timebase_GetMs();
    while ((uint32)(Timebase_GetMs() - startMs) < APP_BOOT_BANNER_MS)
    {
        uint32 nowMs = Timebase_GetMs();
        uint32 ledStep = (uint32)(nowMs - startMs) / APP_BOOT_LED_STEP_MS;

        if (ledStep != lastLedStep)
        {
            App_SetBootLedStep(ledStep);
            lastLedStep = ledStep;
        }

        UartHost_ServiceTx();
    }

    StatusLed_Off();
}

void App_ServiceRuntimeCore(uint32 nowMs)
{
    Servo_Service(nowMs);
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
