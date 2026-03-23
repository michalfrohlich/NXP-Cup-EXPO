#include "app_modes.h"
#include "main_types.h"
#include "car_config.h"
#include "board_init.h"
#include <string.h>

#include "timebase.h"
#include "buttons.h"
#include "onboard_pot.h"
#include "receiver.h"
#include "servo.h"
#include "esc.h"
#include "linear_camera.h"
#include "display.h"
#include "ultrasonic.h"

#include "../services/vision_linear_v2.h"
#include "rgb_led.h"
#include "../services/steering_control_linear.h"
#include "../services/steering_smoothing.h"
#include "vision_debug.h"

#define VDBG_WHITE_MAX_FULL_SCALE     (4095U)
#define VDBG_WHITE_MAX_MIN_ZOOM       (400U)

#define RECEIVER_REFRESH_MS           (50U)
#define TESTS_MENU_VISIBLE_LINES      (3U)
#define TESTS_MENU_POT_STABLE_MS      (90U)
#define TESTS_MENU_POT_HYSTERESIS     (6U)

typedef enum
{
    APP_BUILD_MODE_LINEAR_CAMERA_TEST = 0,
    APP_BUILD_MODE_NXP_CUP,
    APP_BUILD_MODE_RACE_MODE,
    APP_BUILD_MODE_FINAL_DUMMY,
    APP_BUILD_MODE_HONOR_LAP,
    APP_BUILD_MODE_NXP_CUP_TESTS
} AppBuildMode_t;

typedef enum
{
    RUNTIME_TEST_LINEAR_CAMERA = 0,
    RUNTIME_TEST_ESC,
    RUNTIME_TEST_SERVO,
    RUNTIME_TEST_ULTRASONIC,
    RUNTIME_TEST_CAMSERVO,
    RUNTIME_TEST_ULTRA_ESC,
    RUNTIME_TEST_RECEIVER,
    RUNTIME_TEST_COUNT
} RuntimeTestId_t;

typedef enum
{
    SW2_ACTION_NONE = 0,
    SW2_ACTION_CLICK,
    SW2_ACTION_HOLD
} Sw2Action_t;

typedef struct
{
    boolean wasPressed;
    boolean holdHandled;
    uint32 pressedSinceMs;
} Sw2Tracker_t;

typedef struct
{
    uint32 nextRefreshMs;
} ReceiverTestState_t;

typedef enum
{
    ULTRA_TEST_MODE_CLEAR = 0,
    ULTRA_TEST_MODE_STOP_HOLD,
    ULTRA_TEST_MODE_CRAWL,
    ULTRA_TEST_MODE_CUTOFF
} UltrasonicTestMode_t;

typedef struct
{
    uint32 nextTriggerMs;
    uint32 nextDisplayMs;
    uint32 ultraEnableMs;
    uint32 modeUntilMs;
    uint32 slowRangeSinceMs;
    uint32 stopRangeSinceMs;
    float lastDistanceCm;
    boolean haveValidDistance;
    UltrasonicTestMode_t mode;
} UltrasonicTestState_t;

typedef struct
{
    uint32 armDoneMs;
    uint32 nextUltraTrigMs;
    uint32 nextDisplayMs;
    uint32 lastDistanceMs;
    float lastDistanceCm;
    sint32 commandedSpeedPct;
    boolean hasValidDistance;
} UltrasonicEscTestState_t;

typedef struct
{
    boolean configured;
    boolean useSmoothing;
    uint32 nextDisplayMs;
    sint16 steerRaw;
    sint16 steerFilt;
    sint16 steerOut;
} ServoTestState_t;

typedef struct
{
    VisionDebug_State_t vdbg;
    SteeringLinearState_t ctrl;
    Sw2Tracker_t sw2Tracker;
    LinearCameraFrame processedFrame;
    VisionOutput_t result;
    VisionLinear_DebugOut_t dbg;
    uint16 filteredBuf[VISION_LINEAR_BUFFER_SIZE];
    sint16 gradientBuf[VISION_LINEAR_BUFFER_SIZE];
    sint16 steerRaw;
    sint16 steerFilt;
    sint16 steerOut;
    uint32 nextTickMs;
    uint32 nextSteerMs;
    uint32 nextDisplayMs;
    uint32 tickCount;
    boolean haveValidVision;
    boolean displayDirty;
    boolean paused;
    boolean servoEnabled;
    uint8 steeringBaseSpeed;
} LinearCameraTestState_t;

typedef struct
{
    CarMode_t mode;
    uint32 startGoMs;
    uint32 armDoneMs;
    uint32 nextDisplayMs;
    sint16 currentCmdPct;
    uint8 lastPotLevel;
} EscRunState_t;

typedef struct
{
    float kp;
    float kd;
    float ki;
    float iTermClamp;
    float steerLpfAlpha;
    sint16 steerClamp;
    sint16 steerRateMax;
    uint8 baseSpeedPct;
} CamTuneProfile_t;

typedef enum
{
    NXP_CUP_PROFILE_SUPERFAST = 0,
    NXP_CUP_PROFILE_5050,
    NXP_CUP_PROFILE_SLOW,
    NXP_CUP_PROFILE_COUNT
} NxpCupProfileId_t;

typedef struct
{
    VisionDebug_State_t vdbg;
    SteeringLinearState_t ctrl;
    LinearCameraFrame processedFrame;
    VisionOutput_t result;
    VisionLinear_DebugOut_t dbg;
    uint16 filteredBuf[VISION_LINEAR_BUFFER_SIZE];
    sint16 gradientBuf[VISION_LINEAR_BUFFER_SIZE];
    boolean haveValidVision;
    uint32 lastFrameMs;
    sint16 steerRaw;
    sint16 steerFilt;
    sint16 steerOut;
    uint32 nextTickMs;
    uint32 nextSteerMs;
    uint32 nextDisplayMs;
    uint32 tickCount;
    boolean displayDirty;
    CamTuneProfile_t activeTune;
} CamServoState_t;

typedef enum
{
    NXP_CUP_ULTRA_CLEAR = 0,
    NXP_CUP_ULTRA_STOP_HOLD,
    NXP_CUP_ULTRA_CRAWL,
    NXP_CUP_ULTRA_CUTOFF
} NxpCupUltraMode_t;

typedef struct
{
    uint32 nextTriggerMs;
    uint32 ultraEnableMs;
    uint32 modeUntilMs;
    uint32 lastDistanceMs;
    float lastDistanceCm;
    boolean haveValidDistance;
    NxpCupUltraMode_t mode;
} NxpCupUltraState_t;

typedef enum
{
    NXP_CUP_STATE_MENU = 0,
    NXP_CUP_STATE_READY,
    NXP_CUP_STATE_ESC_REARM,
    NXP_CUP_STATE_RUN
} NxpCupModeState_t;

typedef struct
{
    NxpCupModeState_t state;
    NxpCupProfileId_t profileId;
    NxpCupUltraState_t ultraSt;
    CamServoState_t camSt;
    sint32 autoSpeedPct;
    uint32 nextAutoSpeedMs;
    uint32 nextDisplayMs;
    uint32 escRearmDoneMs;
    boolean cameraStarted;
} NxpCupState_t;

typedef enum
{
    DUMMY_ESC = 0,
    DUMMY_CAM = 1
} DummyState_t;

typedef struct
{
    DummyState_t state;
    sint32 autoSpeedPct;
    uint32 nextAutoSpeedMs;
    EscRunState_t escSt;
    CamServoState_t camSt;
} FinalDummyState_t;

typedef struct
{
    uint8 selectedIndex;
    uint8 topIndex;
    uint32 lastPotMs;
    boolean testActive;
    RuntimeTestId_t activeTest;
} TestsMenuState_t;

typedef struct
{
    uint32 nextUltraTrigMs;
    uint32 nextDisplayMs;
    uint32 lastDistanceMs;
    float lastDistanceCm;
    sint32 commandedSpeedPct;
    boolean hasValidDistance;
} HonorLapState_t;

typedef enum
{
    RACE_PHASE_ESC_ARM = 0,
    RACE_PHASE_SPEED_RUN,
    RACE_PHASE_HONOR_RUN,
    RACE_PHASE_STOPPED,
    RACE_PHASE_FAULT
} RacePhase_t;

typedef struct
{
    RacePhase_t phase;
    SteeringLinearState_t ctrl;
    LinearCameraFrame processedFrame;
    VisionOutput_t result;
    uint32 nextVisionMs;
    uint32 nextControlMs;
    uint32 nextSpeedRampMs;
    uint32 nextUltraTrigMs;
    uint32 nextDisplayMs;
    uint32 armDoneMs;
    uint32 lostSinceMs;
    uint32 lastDistanceMs;
    float lastDistanceCm;
    sint16 steerRaw;
    sint16 steerFilt;
    sint16 steerOut;
    sint32 targetSpeedPct;
    sint32 currentSpeedPct;
    uint8 finishStreak;
    boolean haveValidVision;
    boolean haveValidDistance;
    boolean ultraError;
    boolean displayInitialized;
    boolean displayWasOn;
} RaceModeState_t;

static ReceiverTestState_t g_receiverTest;
static UltrasonicTestState_t g_ultrasonicTest;
static UltrasonicEscTestState_t g_ultrasonicEscTest;
static ServoTestState_t g_servoTest;
static EscRunState_t g_escTest;
static LinearCameraTestState_t g_linearCameraTest;
static FinalDummyState_t g_finalDummy;
static TestsMenuState_t g_testsMenu;
static HonorLapState_t g_honorLap;
static NxpCupState_t g_nxpCup;
static RaceModeState_t g_raceMode;

static const CamTuneProfile_t g_nxpCupProfiles[NXP_CUP_PROFILE_COUNT] =
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

static const char g_testsMenuItems[RUNTIME_TEST_COUNT][17] =
{
    "Camera         ",
    "ESC            ",
    "Servo          ",
    "Ultrasonic     ",
    "Cam+Servo      ",
    "Ultra+ESC      ",
    "Receiver - x   "
};

static boolean time_reached(uint32 nowMs, uint32 dueMs);
static uint16 VisionDebug_WhiteMaxFromPot(uint8 potLevel);
static Sw2Action_t Sw2Tracker_Update(Sw2Tracker_t *st, uint32 nowMs);
static void StatusLed_Blue(void);
static void StatusLed_Green(void);
static void StatusLed_Cyan(void);
static void StatusLed_Yellow(void);
static void StatusLed_Red(void);
static void StatusLed_Off(void);
static void Esc_StopNeutral(void);
static void App_InitRuntimeCore(void);
static void App_InitRuntimeCommon(void);
static AppBuildMode_t App_GetSelectedBuildMode(void);
static void DisplayTextPadded(uint16 displayLine, const char *text);
static void receiver_test_enter(uint32 nowMs);
static void receiver_test_update(uint32 nowMs);
static void receiver_test_exit(void);
static void ultrasonic_test_enter(uint32 nowMs);
static void ultrasonic_test_update(uint32 nowMs, boolean sw2Pressed);
static void ultrasonic_test_exit(void);
static void ultrasonic_esc_test_enter(uint32 nowMs);
static void ultrasonic_esc_test_update(uint32 nowMs);
static void ultrasonic_esc_test_exit(void);
static void servo_test_enter(uint32 nowMs);
static void servo_test_update(uint32 nowMs, boolean sw2Pressed, uint8 potLevel);
static void servo_test_exit(void);
static void esc_test_enter(uint32 nowMs);
static void esc_test_update(uint32 nowMs, boolean sw2Pressed, boolean sw3Pressed, uint8 potLevel);
static void esc_test_exit(void);
static void linear_camera_test_enter_common(uint32 nowMs, boolean servoEnabled);
static void linear_camera_test_enter(uint32 nowMs);
static void linear_camera_test_update(uint32 nowMs, boolean modeNextPressed, uint8 potLevel);
static void linear_camera_test_draw_waiting(const LinearCameraTestState_t *st);
static void linear_camera_test_exit(void);
static void runtime_test_enter(RuntimeTestId_t testId, uint32 nowMs);
static void runtime_test_update(RuntimeTestId_t testId,
                                uint32 nowMs,
                                boolean sw2Pressed,
                                boolean modeNextPressed,
                                uint8 potLevel);
static void runtime_test_exit(RuntimeTestId_t testId);
static void esc_enter(EscRunState_t *st, uint32 nowMs);
static void esc_update(EscRunState_t *st, uint32 nowMs, boolean sw2, boolean sw3, uint8 pot);
static void esc_test_draw(const EscRunState_t *st);
static void camservo_apply_profile(CamServoState_t *st, const CamTuneProfile_t *profile);
static void camservo_enter_with_profile(CamServoState_t *st, uint32 nowMs, const CamTuneProfile_t *profile);
static void camservo_enter(CamServoState_t *st, uint32 nowMs);
static void camservo_update(CamServoState_t *st, uint32 nowMs, boolean sw2);
static void camservo_exit(void);
static void final_dummy_enter(uint32 nowMs);
static void final_dummy_update(uint32 nowMs, boolean sw2Pressed, boolean sw3Pressed, uint8 potLevel);
static uint8 tests_menu_map_pot_to_index(uint8 pot, uint8 nItems);
static void tests_menu_select_index(TestsMenuState_t *st, uint8 pot);
static void tests_menu_draw(const TestsMenuState_t *st);
static void mode_nxp_cup_tests(void);
static void nxp_cup_idle_motor(void);
static void nxp_cup_obstacle_stop_motor(void);
static void nxp_cup_launch_motor(int logicalCmd);
static uint8 nxp_cup_profile_from_pot(uint8 pot);
static void nxp_cup_ultra_enter(NxpCupUltraState_t *st, uint32 nowMs);
static void nxp_cup_ultra_arm_for_run(NxpCupUltraState_t *st, uint32 nowMs);
static boolean nxp_cup_ultra_is_active(const NxpCupUltraState_t *st, uint32 nowMs);
static boolean nxp_cup_ultra_mode_is_obstacle(const NxpCupUltraState_t *st);
static void nxp_cup_ultra_task(NxpCupUltraState_t *st, uint32 nowMs);
static boolean nxp_cup_ultra_should_hold_servo(const NxpCupUltraState_t *st, uint32 nowMs);
static uint8 nxp_cup_ultra_target_speed_pct(const NxpCupUltraState_t *st, uint32 nowMs, uint8 defaultSpeedPct);
static void mode_nxp_cup(void);
static void mode_final_dummy(void);
static sint32 honor_speed_from_distance(boolean hasValidDistance, float distanceCm);
static void honor_lap_draw_overlay(void);
static void honor_lap_enter(uint32 nowMs);
static void honor_lap_update(uint32 nowMs, boolean modeNextPressed, uint8 potLevel);
static void mode_honor_lap(void);
static void race_mode_enter(uint32 nowMs);
static void race_mode_update_vision(RaceModeState_t *st, uint32 nowMs);
static void race_mode_update_ultrasonic(RaceModeState_t *st, uint32 nowMs);
static void race_mode_update_control(RaceModeState_t *st, uint32 nowMs, boolean stopPressed);
static void race_mode_ensure_display_ready(RaceModeState_t *st);
static void race_mode_draw(const RaceModeState_t *st);
static void mode_race_mode(void);

static boolean time_reached(uint32 nowMs, uint32 dueMs)
{
    return ((uint32)(nowMs - dueMs) < 0x80000000u) ? TRUE : FALSE;
}

static void display_power_stabilize_delay(void)
{
    uint32 t0 = Timebase_GetMs();

    while ((uint32)(Timebase_GetMs() - t0) < 200u)
    {
    }
}

static uint16 VisionDebug_WhiteMaxFromPot(uint8 potLevel)
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

static Sw2Action_t Sw2Tracker_Update(Sw2Tracker_t *st, uint32 nowMs)
{
    boolean pressedNow;

    if (st == NULL_PTR)
    {
        return SW2_ACTION_NONE;
    }

    pressedNow = Buttons_IsPressed(BUTTON_ID_SW2);

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

static void StatusLed_Blue(void)
{
    RgbLed_ChangeColor((RgbLed_Color){ .r = false, .g = false, .b = true });
}

static void StatusLed_Green(void)
{
    RgbLed_ChangeColor((RgbLed_Color){ .r = false, .g = true, .b = false });
}

static void StatusLed_Cyan(void)
{
    RgbLed_ChangeColor((RgbLed_Color){ .r = false, .g = true, .b = true });
}

static void StatusLed_Yellow(void)
{
    RgbLed_ChangeColor((RgbLed_Color){ .r = true, .g = true, .b = false });
}

static void StatusLed_Red(void)
{
    RgbLed_ChangeColor((RgbLed_Color){ .r = true, .g = false, .b = false });
}

static void StatusLed_Off(void)
{
    RgbLed_ChangeColor((RgbLed_Color){ .r = false, .g = false, .b = false });
}

static void Esc_StopNeutral(void)
{
    EscSetBrake(0U);
    EscSetSpeed(0);
}

static void App_InitRuntimeCore(void)
{
    Board_InitDrivers();
    Timebase_Init();
    OnboardPot_Init();
    StatusLed_Off();
}

static void App_InitRuntimeCommon(void)
{
    App_InitRuntimeCore();
    display_power_stabilize_delay();
    DisplayInit(0U, STD_ON);
    DisplayClear();
    DisplayRefresh();
}

static void DisplayTextPadded(uint16 displayLine, const char *text)
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

static AppBuildMode_t App_GetSelectedBuildMode(void)
{
#if APP_TEST_NXP_CUP
    return APP_BUILD_MODE_NXP_CUP;
#elif APP_TEST_NXP_CUP_TESTS
    return APP_BUILD_MODE_NXP_CUP_TESTS;
#elif APP_TEST_HONOR_LAP
    return APP_BUILD_MODE_HONOR_LAP;
#elif APP_TEST_RACE_MODE
    return APP_BUILD_MODE_RACE_MODE;
#elif APP_TEST_FINAL_DUMMY
    return APP_BUILD_MODE_FINAL_DUMMY;
#elif APP_TEST_LINEAR_CAMERA_TEST
    return APP_BUILD_MODE_LINEAR_CAMERA_TEST;
#else
#error "CONFIG ERROR: App_GetSelectedBuildMode has no active APP_TEST_* selection"
#endif
}

static void receiver_test_enter(uint32 nowMs)
{
    (void)memset(&g_receiverTest, 0, sizeof(g_receiverTest));

    ReceiverInit(0U, 0U, 11700U, 17700U, 23700U, 26000U);
    StatusLed_Blue();

    DisplayClear();
    DisplayTextPadded(0U, "Ch0:    Ch1:");
    DisplayTextPadded(1U, "Ch2:    Ch3:");
    DisplayTextPadded(2U, "Ch4:    Ch5:");
    DisplayTextPadded(3U, "Ch6:    Ch7:");
    DisplayRefresh();

    g_receiverTest.nextRefreshMs = nowMs;
}

static void receiver_test_update(uint32 nowMs)
{
    uint8 displayValueOffset;
    int receiverChannels[8];

    if (time_reached(nowMs, g_receiverTest.nextRefreshMs) != TRUE)
    {
        return;
    }

    g_receiverTest.nextRefreshMs = nowMs + RECEIVER_REFRESH_MS;

    for (uint8 i = 0U; i < 8U; i++)
    {
        receiverChannels[i] = GetReceiverChannel(i);
        displayValueOffset = (uint8)(4U + 8U * (i % 2U));
        DisplayValue(i / 2U, receiverChannels[i], 4U, displayValueOffset);
    }

    DisplayRefresh();
}

static void receiver_test_exit(void)
{
}

static void ultrasonic_test_enter(uint32 nowMs)
{
    (void)memset(&g_ultrasonicTest, 0, sizeof(g_ultrasonicTest));

    Ultrasonic_Init();
    g_ultrasonicTest.nextTriggerMs = nowMs;
    g_ultrasonicTest.nextDisplayMs = nowMs;
    g_ultrasonicTest.lastDistanceCm = 0.0f;
    g_ultrasonicTest.haveValidDistance = FALSE;
    g_ultrasonicTest.ultraEnableMs = nowMs + NXP_ULTRA_ENABLE_AFTER_RUN_MS;
    g_ultrasonicTest.mode = ULTRA_TEST_MODE_CLEAR;
    g_ultrasonicTest.modeUntilMs = 0U;
    g_ultrasonicTest.slowRangeSinceMs = 0U;
    g_ultrasonicTest.stopRangeSinceMs = 0U;
    StatusLed_Blue();
}

static void ultrasonic_test_update(uint32 nowMs, boolean sw2Pressed)
{
    Ultrasonic_StatusType st;
    boolean active;
    boolean inStopRange = FALSE;
    boolean inSlowRange = FALSE;

    if (sw2Pressed == TRUE)
    {
        Ultrasonic_Init();
        g_ultrasonicTest.nextTriggerMs = nowMs;
        g_ultrasonicTest.ultraEnableMs = nowMs + NXP_ULTRA_ENABLE_AFTER_RUN_MS;
        g_ultrasonicTest.haveValidDistance = FALSE;
        g_ultrasonicTest.lastDistanceCm = 0.0f;
        g_ultrasonicTest.mode = ULTRA_TEST_MODE_CLEAR;
        g_ultrasonicTest.modeUntilMs = 0U;
        g_ultrasonicTest.slowRangeSinceMs = 0U;
        g_ultrasonicTest.stopRangeSinceMs = 0U;
    }

    Ultrasonic_Task();
    st = Ultrasonic_GetStatus();

    if (Ultrasonic_GetDistanceCm(&g_ultrasonicTest.lastDistanceCm) == TRUE)
    {
        if (g_ultrasonicTest.lastDistanceCm > 0.0f)
        {
            g_ultrasonicTest.haveValidDistance = TRUE;
        }
    }

    if (time_reached(nowMs, g_ultrasonicTest.nextTriggerMs) == TRUE)
    {
        if (Ultrasonic_GetStatus() != ULTRA_STATUS_BUSY)
        {
            Ultrasonic_StartMeasurement();
            g_ultrasonicTest.nextTriggerMs = nowMs + (uint32)NXP_CUP_ULTRA_TRIGGER_PERIOD_MS;
        }
    }

    active = (time_reached(nowMs, g_ultrasonicTest.ultraEnableMs) == TRUE) ? TRUE : FALSE;

    if (active != TRUE)
    {
        g_ultrasonicTest.mode = ULTRA_TEST_MODE_CLEAR;
        g_ultrasonicTest.modeUntilMs = 0U;
    }
    else if (g_ultrasonicTest.mode != ULTRA_TEST_MODE_CUTOFF)
    {
        if (g_ultrasonicTest.haveValidDistance == TRUE)
        {
            if (g_ultrasonicTest.lastDistanceCm <= (float)NXP_CUP_ULTRA_CRAWL_STOP_CM)
            {
                g_ultrasonicTest.mode = ULTRA_TEST_MODE_CUTOFF;
            }
            else
            {
                switch (g_ultrasonicTest.mode)
                {
                    case ULTRA_TEST_MODE_STOP_HOLD:
                        if ((g_ultrasonicTest.modeUntilMs != 0U) &&
                            (time_reached(nowMs, g_ultrasonicTest.modeUntilMs) == TRUE))
                        {
                            if (g_ultrasonicTest.lastDistanceCm <= (float)NXP_CUP_ULTRA_STOP_CM)
                            {
                                g_ultrasonicTest.mode = ULTRA_TEST_MODE_CRAWL;
                            }
                            else
                            {
                                g_ultrasonicTest.mode = ULTRA_TEST_MODE_CLEAR;
                            }
                            g_ultrasonicTest.modeUntilMs = 0U;
                        }
                        break;

                    case ULTRA_TEST_MODE_CRAWL:
                        break;

                    case ULTRA_TEST_MODE_CLEAR:
                    default:
                        if (g_ultrasonicTest.lastDistanceCm <= (float)NXP_CUP_ULTRA_STOP_CM)
                        {
                            g_ultrasonicTest.mode = ULTRA_TEST_MODE_STOP_HOLD;
                            g_ultrasonicTest.modeUntilMs = nowMs + (uint32)NXP_CUP_ULTRA_STOP_HOLD_MS;
                        }
                        break;
                }
            }
        }
    }

    if ((active == TRUE) && (g_ultrasonicTest.haveValidDistance == TRUE))
    {
        inStopRange = (g_ultrasonicTest.lastDistanceCm <= (float)NXP_CUP_ULTRA_CRAWL_STOP_CM) ? TRUE : FALSE;
        inSlowRange = ((g_ultrasonicTest.lastDistanceCm <= (float)NXP_CUP_ULTRA_STOP_CM) &&
                       (inStopRange != TRUE)) ? TRUE : FALSE;

        if (inSlowRange == TRUE)
        {
            if (g_ultrasonicTest.slowRangeSinceMs == 0U)
            {
                g_ultrasonicTest.slowRangeSinceMs = nowMs;
            }
        }
        else
        {
            g_ultrasonicTest.slowRangeSinceMs = 0U;
        }

        if (inStopRange == TRUE)
        {
            if (g_ultrasonicTest.stopRangeSinceMs == 0U)
            {
                g_ultrasonicTest.stopRangeSinceMs = nowMs;
            }
        }
        else
        {
            g_ultrasonicTest.stopRangeSinceMs = 0U;
        }

        if ((g_ultrasonicTest.stopRangeSinceMs != 0U) &&
            (((uint32)(nowMs - g_ultrasonicTest.stopRangeSinceMs)) >= 10U))
        {
            StatusLed_Red();
        }
        else if ((g_ultrasonicTest.slowRangeSinceMs != 0U) &&
                 (((uint32)(nowMs - g_ultrasonicTest.slowRangeSinceMs)) >= 80U))
        {
            StatusLed_Yellow();
        }
        else
        {
            StatusLed_Green();
        }
    }
    else
    {
        g_ultrasonicTest.slowRangeSinceMs = 0U;
        g_ultrasonicTest.stopRangeSinceMs = 0U;

        if (st == ULTRA_STATUS_ERROR)
        {
            StatusLed_Red();
        }
        else if (active == TRUE)
        {
            StatusLed_Green();
        }
        else
        {
            StatusLed_Blue();
        }
    }

    if (time_reached(nowMs, g_ultrasonicTest.nextDisplayMs) != TRUE)
    {
        return;
    }

    g_ultrasonicTest.nextDisplayMs = nowMs + DISPLAY_PERIOD_MS;

    DisplayClear();
    DisplayText(0U, "ULTRA TEST", 10U, 0U);

    if (g_ultrasonicTest.haveValidDistance == TRUE)
    {
        DisplayText(1U, "CM:", 3U, 0U);
        DisplayValue(1U, (int)(g_ultrasonicTest.lastDistanceCm + 0.5f), 4U, 4U);
    }
    else
    {
        DisplayText(1U, "CM: ----", 8U, 0U);
    }

    if (active != TRUE)
    {
        DisplayText(2U, "MODE: WAIT", 10U, 0U);
    }
    else if (g_ultrasonicTest.haveValidDistance != TRUE)
    {
        DisplayText(2U, "MODE: SCAN", 10U, 0U);
    }
    else if ((g_ultrasonicTest.mode == ULTRA_TEST_MODE_STOP_HOLD) ||
             (g_ultrasonicTest.mode == ULTRA_TEST_MODE_CUTOFF))
    {
        DisplayText(2U, "MODE: STOP", 10U, 0U);
    }
    else if (g_ultrasonicTest.mode == ULTRA_TEST_MODE_CRAWL)
    {
        DisplayText(2U, "MODE: SLOW", 10U, 0U);
    }
    else
    {
        DisplayText(2U, "MODE: CLEAR", 11U, 0U);
    }

    switch (st)
    {
        case ULTRA_STATUS_BUSY:
            DisplayText(3U, "DRV: BUSY", 9U, 0U);
            break;
        case ULTRA_STATUS_NEW_DATA:
            DisplayText(3U, "DRV: NEW", 8U, 0U);
            break;
        case ULTRA_STATUS_ERROR:
            DisplayText(3U, "DRV: ERR", 8U, 0U);
            break;
        case ULTRA_STATUS_IDLE:
        default:
            DisplayText(3U, "DRV: IDLE", 9U, 0U);
            break;
    }

    DisplayRefresh();
}

static void ultrasonic_test_exit(void)
{
}

static void ultrasonic_esc_test_enter(uint32 nowMs)
{
    (void)memset(&g_ultrasonicEscTest, 0, sizeof(g_ultrasonicEscTest));

    EscInit(ESC_PWM_CH, ESC_DUTY_MIN, ESC_DUTY_MED, ESC_DUTY_MAX);
    Esc_StopNeutral();

    Ultrasonic_Init();
    g_ultrasonicEscTest.armDoneMs = nowMs + ESC_ARM_TIME_MS;
    g_ultrasonicEscTest.nextUltraTrigMs = nowMs;
    g_ultrasonicEscTest.nextDisplayMs = nowMs;
    g_ultrasonicEscTest.lastDistanceMs = nowMs;
    g_ultrasonicEscTest.lastDistanceCm = 0.0f;
    g_ultrasonicEscTest.commandedSpeedPct = 0;

    DisplayTextPadded(0U, "ULTRA+ESC TEST");
    DisplayTextPadded(1U, "ARM  D: ---cm");
    DisplayTextPadded(2U, "SPD:   0%");
    DisplayTextPadded(3U, "TRIG:  AGE:");
    DisplayRefresh();
    StatusLed_Blue();
}

static void ultrasonic_esc_test_update(uint32 nowMs)
{
    const uint32 ultraStaleMs =
        ((uint32)HONOR_ULTRA_TRIGGER_PERIOD_MS * 2U) + (uint32)ULTRA_TIMEOUT_MS;
    Ultrasonic_StatusType st;
    float distanceCm = 0.0f;
    const char *modeText = "RUN ";

    Ultrasonic_Task();

    if (time_reached(nowMs, g_ultrasonicEscTest.nextUltraTrigMs) == TRUE)
    {
        g_ultrasonicEscTest.nextUltraTrigMs = nowMs + HONOR_ULTRA_TRIGGER_PERIOD_MS;

        if (Ultrasonic_GetStatus() != ULTRA_STATUS_BUSY)
        {
            Ultrasonic_StartMeasurement();
        }
    }

    st = Ultrasonic_GetStatus();

    if (Ultrasonic_GetDistanceCm(&distanceCm) == TRUE)
    {
        g_ultrasonicEscTest.lastDistanceCm = distanceCm;
        g_ultrasonicEscTest.lastDistanceMs = nowMs;
        g_ultrasonicEscTest.hasValidDistance = TRUE;
        st = ULTRA_STATUS_NEW_DATA;
    }
    else if (st == ULTRA_STATUS_ERROR)
    {
        g_ultrasonicEscTest.hasValidDistance = FALSE;
    }
    else if ((g_ultrasonicEscTest.hasValidDistance == TRUE) &&
             ((uint32)(nowMs - g_ultrasonicEscTest.lastDistanceMs) > ultraStaleMs))
    {
        g_ultrasonicEscTest.hasValidDistance = FALSE;
    }

    if (time_reached(nowMs, g_ultrasonicEscTest.armDoneMs) != TRUE)
    {
        g_ultrasonicEscTest.commandedSpeedPct = 0;
        Esc_StopNeutral();
        modeText = "ARM ";
    }
    else
    {
        g_ultrasonicEscTest.commandedSpeedPct =
            honor_speed_from_distance(g_ultrasonicEscTest.hasValidDistance,
                                      g_ultrasonicEscTest.lastDistanceCm);
        EscSetBrake(0U);
        EscSetSpeed((int)(-g_ultrasonicEscTest.commandedSpeedPct));
    }

    if (time_reached(nowMs, g_ultrasonicEscTest.nextDisplayMs) != TRUE)
    {
        return;
    }

    g_ultrasonicEscTest.nextDisplayMs = nowMs + DISPLAY_PERIOD_MS;

    DisplayTextPadded(0U, "ULTRA+ESC TEST");
    DisplayTextPadded(1U, "    D: ---cm");
    DisplayText(1U, modeText, 4U, 0U);
    if (g_ultrasonicEscTest.hasValidDistance == TRUE)
    {
        DisplayValue(1U, (int)(g_ultrasonicEscTest.lastDistanceCm + 0.5f), 3U, 8U);
        DisplayText(1U, "cm", 2U, 11U);
    }
    else if (st == ULTRA_STATUS_ERROR)
    {
        DisplayText(1U, "ERR", 3U, 8U);
    }

    DisplayTextPadded(2U, "SPD:   0%");
    DisplayValue(2U, (int)g_ultrasonicEscTest.commandedSpeedPct, 3U, 4U);
    DisplayText(2U, "%", 1U, 7U);

    DisplayTextPadded(3U, "TRIG:  AGE:");
    DisplayValue(3U, (int)HONOR_ULTRA_TRIGGER_PERIOD_MS, 3U, 5U);
    DisplayValue(3U, (int)(nowMs - g_ultrasonicEscTest.lastDistanceMs), 4U, 12U);
    DisplayRefresh();

    if (time_reached(nowMs, g_ultrasonicEscTest.armDoneMs) != TRUE)
    {
        StatusLed_Blue();
    }
    else if (st == ULTRA_STATUS_ERROR)
    {
        StatusLed_Red();
    }
    else if (g_ultrasonicEscTest.hasValidDistance == TRUE)
    {
        StatusLed_Green();
    }
    else
    {
        StatusLed_Yellow();
    }
}

static void ultrasonic_esc_test_exit(void)
{
    Esc_StopNeutral();
}

static void servo_test_enter(uint32 nowMs)
{
    (void)memset(&g_servoTest, 0, sizeof(g_servoTest));

    ServoInit(SERVO_PWM_CH, SERVO_DUTY_MAX, SERVO_DUTY_MIN, SERVO_DUTY_MED);
    SteerStraight();

    g_servoTest.configured = FALSE;
    g_servoTest.useSmoothing = TRUE;
    g_servoTest.nextDisplayMs = nowMs;

    StatusLed_Blue();
}

static void servo_test_update(uint32 nowMs, boolean sw2Pressed, uint8 potLevel)
{
    if (g_servoTest.configured != TRUE)
    {
        g_servoTest.useSmoothing = (potLevel >= 128U) ? TRUE : FALSE;

        DisplayText(0U, "SERVO MODE", 10U, 0U);
        if (g_servoTest.useSmoothing == TRUE)
        {
            DisplayText(1U, " RAW", 4U, 0U);
            DisplayText(2U, ">SMOOTH", 7U, 0U);
        }
        else
        {
            DisplayText(1U, ">RAW", 4U, 0U);
            DisplayText(2U, " SMOOTH", 7U, 0U);
        }
        DisplayText(3U, "SW2 ENTER", 9U, 0U);
        DisplayRefresh();

        if (sw2Pressed == TRUE)
        {
            g_servoTest.configured = TRUE;
            g_servoTest.nextDisplayMs = nowMs;
        }
        return;
    }

    if ((uint32)(nowMs - g_servoTest.nextDisplayMs) < SERVO_TEST_UPDATE_MS)
    {
        return;
    }
    g_servoTest.nextDisplayMs += SERVO_TEST_UPDATE_MS;

    g_servoTest.steerRaw = (sint16)((((sint32)potLevel * 200) / 255) - 100);
    if (g_servoTest.steerRaw > (sint16)SERVO_TEST_CMD_CLAMP)
    {
        g_servoTest.steerRaw = (sint16)SERVO_TEST_CMD_CLAMP;
    }
    if (g_servoTest.steerRaw < (sint16)(-SERVO_TEST_CMD_CLAMP))
    {
        g_servoTest.steerRaw = (sint16)(-SERVO_TEST_CMD_CLAMP);
    }

    if (sw2Pressed == TRUE)
    {
        g_servoTest.steerRaw = 0;
        g_servoTest.steerFilt = 0;
        g_servoTest.steerOut = 0;
        SteerStraight();
    }
    else
    {
        if (g_servoTest.useSmoothing == TRUE)
        {
            sint16 rateMax;
            sint16 absFilt;

            g_servoTest.steerRaw = SteeringSmooth_DeadzoneS16(g_servoTest.steerRaw,
                                                              (sint16)SERVO_TEST_DEADBAND,
                                                              (sint16)SERVO_TEST_CMD_CLAMP);
            g_servoTest.steerFilt = SteeringSmooth_IirS16(g_servoTest.steerFilt,
                                                          g_servoTest.steerRaw,
                                                          (float)SERVO_TEST_LPF_ALPHA);

            absFilt = (g_servoTest.steerFilt < 0) ? (sint16)(-g_servoTest.steerFilt) : g_servoTest.steerFilt;
            rateMax = (sint16)SERVO_TEST_RATE_MAX;
            rateMax = (sint16)(rateMax + (absFilt / 6));
            if (rateMax > (sint16)SERVO_TEST_CMD_CLAMP)
            {
                rateMax = (sint16)SERVO_TEST_CMD_CLAMP;
            }

            g_servoTest.steerOut = SteeringSmooth_RateLimitS16(g_servoTest.steerOut,
                                                               g_servoTest.steerFilt,
                                                               rateMax,
                                                               (sint16)SERVO_TEST_CMD_CLAMP,
                                                               0,
                                                               0);
            g_servoTest.steerOut = SteeringSmooth_ClampS16(g_servoTest.steerOut,
                                                           (sint16)(-SERVO_TEST_CMD_CLAMP),
                                                           (sint16)SERVO_TEST_CMD_CLAMP);
        }
        else
        {
            g_servoTest.steerFilt = g_servoTest.steerRaw;
            g_servoTest.steerOut = g_servoTest.steerRaw;
        }

        Steer((int)g_servoTest.steerOut);
    }

    if (g_servoTest.useSmoothing == TRUE)
    {
        DisplayText(0U, "SERVO SMOOTH", 12U, 0U);
    }
    else
    {
        DisplayText(0U, "SERVO RAW", 9U, 0U);
    }
    DisplayText(1U, "RAW:", 4U, 0U);
    DisplayValue(1U, (int)g_servoTest.steerRaw, 4U, 5U);
    DisplayText(2U, "OUT:", 4U, 0U);
    DisplayValue(2U, (int)g_servoTest.steerOut, 4U, 5U);
    DisplayText(3U, "LIM:", 4U, 0U);
    DisplayValue(3U, (int)SERVO_TEST_CMD_CLAMP, 4U, 5U);
    DisplayRefresh();

    StatusLed_Blue();
}

static void servo_test_exit(void)
{
    SteerStraight();
}

static void esc_test_enter(uint32 nowMs)
{
    (void)memset(&g_escTest, 0, sizeof(g_escTest));
    esc_enter(&g_escTest, nowMs);
    esc_test_draw(&g_escTest);
}

static void esc_test_update(uint32 nowMs, boolean sw2Pressed, boolean sw3Pressed, uint8 potLevel)
{
    esc_update(&g_escTest, nowMs, sw2Pressed, sw3Pressed, potLevel);

    if (time_reached(nowMs, g_escTest.nextDisplayMs) == TRUE)
    {
        g_escTest.nextDisplayMs = nowMs + DISPLAY_PERIOD_MS;
        esc_test_draw(&g_escTest);
    }
}

static void esc_test_exit(void)
{
    Esc_StopNeutral();
}

static void linear_camera_test_enter_common(uint32 nowMs, boolean servoEnabled)
{
    (void)memset(&g_linearCameraTest, 0, sizeof(g_linearCameraTest));

    g_linearCameraTest.servoEnabled = servoEnabled;
    g_linearCameraTest.steeringBaseSpeed = 20U;
    LinearCameraInit(CAM_CLK_PWM_CH, CAM_SHUTTER_GPT_CH, CAM_ADC_GROUP, CAM_SHUTTER_PCR);
    LinearCameraSetFrameIntervalTicks(CAM_FRAME_INTERVAL_TICKS);
    VisionLinear_InitV2();
    VisionDebug_Init(&g_linearCameraTest.vdbg, 3200U);

    g_linearCameraTest.nextTickMs = nowMs;
    g_linearCameraTest.nextSteerMs = nowMs + STEER_UPDATE_MS;
    g_linearCameraTest.nextDisplayMs = nowMs;

    if (servoEnabled == TRUE)
    {
        ServoInit(SERVO_PWM_CH, SERVO_DUTY_MAX, SERVO_DUTY_MIN, SERVO_DUTY_MED);
        SteerStraight();
        SteeringLinear_Init(&g_linearCameraTest.ctrl);
        SteeringLinear_Reset(&g_linearCameraTest.ctrl);
    }

    /* Push the first visible OLED frame before enabling the camera ISR load.
       The display path is synchronous, while camera start immediately begins
       GPT/PWM/ADC activity that can disturb the first refresh. */
    linear_camera_test_draw_waiting(&g_linearCameraTest);
    (void)LinearCameraStartStream();
}

static void linear_camera_test_enter(uint32 nowMs)
{
    linear_camera_test_enter_common(nowMs, FALSE);
}

static void linear_camera_test_draw_waiting(const LinearCameraTestState_t *st)
{
    LinearCameraDebugCounters counters;

    if (st == NULL_PTR)
    {
        return;
    }

    (void)memset(&counters, 0, sizeof(counters));
    LinearCameraGetDebugCounters(&counters);

    DisplayTextPadded(0U, "CAM WAIT FRAME");
    DisplayTextPadded(1U, "Req:    SI:    ");
    DisplayTextPadded(2U, "Frm:    Evt:   ");
    DisplayTextPadded(3U, "Drop:   P: B:  ");

    DisplayValue(1U, (int)counters.frameRequestCount, 4U, 4U);
    DisplayValue(1U, (int)counters.frameStartCount, 4U, 11U);
    DisplayValue(2U, (int)counters.frameCompleteCount, 4U, 4U);
    DisplayValue(2U, (int)counters.captureEventCount, 4U, 11U);
    DisplayValue(3U, (int)counters.droppedFrameCount, 4U, 5U);
    DisplayText(3U, (st->paused == TRUE) ? "Y" : "N", 1U, 12U);
    DisplayText(3U, (LinearCameraIsBusy() == TRUE) ? "Y" : "N", 1U, 15U);
    DisplayRefresh();
}

static void linear_camera_test_update(uint32 nowMs, boolean modeNextPressed, uint8 potLevel)
{
    const uint32 loopPeriodMs = 5U;
    Sw2Action_t sw2Action;
    boolean sw2ClickHandled = FALSE;
    boolean screenTogglePressed;

    if ((uint32)(nowMs - g_linearCameraTest.nextTickMs) < loopPeriodMs)
    {
        return;
    }

    g_linearCameraTest.nextTickMs += loopPeriodMs;
    g_linearCameraTest.tickCount++;

    sw2Action = Sw2Tracker_Update(&g_linearCameraTest.sw2Tracker, nowMs);

    if ((sw2Action == SW2_ACTION_HOLD) && (g_linearCameraTest.paused != TRUE))
    {
        if (LinearCameraIsBusy() == TRUE)
        {
            LinearCameraStopStream();
        }
        g_linearCameraTest.paused = TRUE;
        g_linearCameraTest.vdbg.cfg.paused = TRUE;
    }
    else if ((sw2Action == SW2_ACTION_CLICK) && (g_linearCameraTest.paused == TRUE))
    {
        g_linearCameraTest.paused = FALSE;
        g_linearCameraTest.vdbg.cfg.paused = FALSE;
        g_linearCameraTest.vdbg.screen = g_linearCameraTest.vdbg.debugEntryScreen;
        (void)LinearCameraStartStream();
        sw2ClickHandled = TRUE;
    }

    if (g_linearCameraTest.paused == TRUE)
    {
        g_linearCameraTest.vdbg.cfg.whiteSat = VisionDebug_WhiteMaxFromPot(potLevel);
        g_linearCameraTest.displayDirty = TRUE;
    }

    screenTogglePressed = (boolean)((sw2Action == SW2_ACTION_CLICK) &&
                                    (sw2ClickHandled != TRUE) &&
                                    (g_linearCameraTest.paused != TRUE));
    VisionDebug_OnTick(&g_linearCameraTest.vdbg,
                       screenTogglePressed,
                       modeNextPressed);

    if ((screenTogglePressed == TRUE) || (modeNextPressed == TRUE))
    {
        g_linearCameraTest.displayDirty = TRUE;
    }

    if (g_linearCameraTest.paused != TRUE)
    {
        const LinearCameraFrame *latestFrame = NULL_PTR;

        if (LinearCameraGetLatestFrame(&latestFrame) == TRUE)
        {
            g_linearCameraTest.dbg.mask = (uint32)VLIN_DBG_NONE;
            g_linearCameraTest.dbg.filteredOut = NULL_PTR;
            g_linearCameraTest.dbg.gradientOut = NULL_PTR;

            if (VisionDebug_WantsVisionDebugData(&g_linearCameraTest.vdbg) == TRUE)
            {
                VisionDebug_PrepareVisionDbg(&g_linearCameraTest.vdbg,
                                             &g_linearCameraTest.dbg,
                                             g_linearCameraTest.filteredBuf,
                                             g_linearCameraTest.gradientBuf);
            }

            (void)memcpy(g_linearCameraTest.processedFrame.Values,
                         &latestFrame->Values[CAM_TRIM_LEFT_PX],
                         ((size_t)VISION_LINEAR_BUFFER_SIZE *
                          sizeof(g_linearCameraTest.processedFrame.Values[0])));
            VisionLinear_ProcessFrameEx(g_linearCameraTest.processedFrame.Values,
                                        &g_linearCameraTest.result,
                                        &g_linearCameraTest.dbg);

            g_linearCameraTest.haveValidVision = TRUE;
            g_linearCameraTest.displayDirty = TRUE;
        }
    }

    if ((g_linearCameraTest.servoEnabled == TRUE) &&
        (time_reached(nowMs, g_linearCameraTest.nextSteerMs) == TRUE))
    {
        g_linearCameraTest.nextSteerMs += STEER_UPDATE_MS;

        if ((g_linearCameraTest.paused == TRUE) || (g_linearCameraTest.haveValidVision != TRUE))
        {
            g_linearCameraTest.steerRaw = 0;
            g_linearCameraTest.steerFilt = 0;
            g_linearCameraTest.steerOut = 0;
            SteerStraight();
        }
        else
        {
            const float dt = ((float)STEER_UPDATE_MS) * 0.001f;
            SteeringOutput_t out =
                SteeringLinear_UpdateV2(&g_linearCameraTest.ctrl,
                                        &g_linearCameraTest.result,
                                        dt,
                                        g_linearCameraTest.steeringBaseSpeed);

            g_linearCameraTest.steerRaw = (sint16)out.steer_cmd;

            if (((g_linearCameraTest.steerRaw < 0) ? (sint16)(-g_linearCameraTest.steerRaw) : g_linearCameraTest.steerRaw) <= 2)
            {
                g_linearCameraTest.steerRaw = 0;
            }

            g_linearCameraTest.steerFilt = SteeringSmooth_IirS16(g_linearCameraTest.steerFilt,
                                                                 g_linearCameraTest.steerRaw,
                                                                 0.45f);

            {
                const sint16 steerRateMax = 8;
                sint16 delta = (sint16)(g_linearCameraTest.steerFilt - g_linearCameraTest.steerOut);

                delta = SteeringSmooth_ClampS16(delta, (sint16)(-steerRateMax), (sint16)(+steerRateMax));
                g_linearCameraTest.steerOut = (sint16)(g_linearCameraTest.steerOut + delta);
            }

            g_linearCameraTest.steerOut = SteeringSmooth_ClampS16(g_linearCameraTest.steerOut,
                                                                  (sint16)(-STEER_CMD_CLAMP),
                                                                  (sint16)(+STEER_CMD_CLAMP));

            Steer((int)g_linearCameraTest.steerOut);
        }
    }

    if (((g_linearCameraTest.displayDirty == TRUE) ||
         (g_linearCameraTest.haveValidVision != TRUE)) &&
        (time_reached(nowMs, g_linearCameraTest.nextDisplayMs) == TRUE))
    {
        g_linearCameraTest.nextDisplayMs = nowMs + CAM_DEBUG_DISPLAY_PERIOD_MS;

        if (g_linearCameraTest.haveValidVision == TRUE)
        {
            const uint16 *pFiltered =
                (g_linearCameraTest.dbg.filteredOut != NULL_PTR) ? g_linearCameraTest.filteredBuf : NULL_PTR;
            const sint16 *pGradient =
                (g_linearCameraTest.dbg.gradientOut != NULL_PTR) ? g_linearCameraTest.gradientBuf : NULL_PTR;
            const VisionLinear_DebugOut_t *pDbg =
                (g_linearCameraTest.dbg.mask != (uint32)VLIN_DBG_NONE) ? &g_linearCameraTest.dbg : NULL_PTR;

            g_linearCameraTest.displayDirty = FALSE;
            VisionDebug_Draw(&g_linearCameraTest.vdbg,
                             g_linearCameraTest.processedFrame.Values,
                             pFiltered,
                             pGradient,
                             &g_linearCameraTest.result,
                             pDbg);
        }
        else
        {
            linear_camera_test_draw_waiting(&g_linearCameraTest);
        }
    }

}

static void linear_camera_test_exit(void)
{
    if (g_linearCameraTest.servoEnabled == TRUE)
    {
        SteerStraight();
    }

    if (LinearCameraIsBusy() == TRUE)
    {
        LinearCameraStopStream();
    }

    StatusLed_Blue();
}

static void runtime_test_enter(RuntimeTestId_t testId, uint32 nowMs)
{
    switch (testId)
    {
        case RUNTIME_TEST_ESC:
            esc_test_enter(nowMs);
            break;

        case RUNTIME_TEST_SERVO:
            servo_test_enter(nowMs);
            break;

        case RUNTIME_TEST_ULTRASONIC:
            ultrasonic_test_enter(nowMs);
            break;

        case RUNTIME_TEST_CAMSERVO:
            linear_camera_test_enter_common(nowMs, TRUE);
            break;

        case RUNTIME_TEST_ULTRA_ESC:
            ultrasonic_esc_test_enter(nowMs);
            break;

        case RUNTIME_TEST_RECEIVER:
            receiver_test_enter(nowMs);
            break;

        case RUNTIME_TEST_LINEAR_CAMERA:
        default:
            linear_camera_test_enter(nowMs);
            break;
    }
}

static void runtime_test_update(RuntimeTestId_t testId,
                                uint32 nowMs,
                                boolean sw2Pressed,
                                boolean modeNextPressed,
                                uint8 potLevel)
{
    switch (testId)
    {
        case RUNTIME_TEST_ESC:
            esc_test_update(nowMs, sw2Pressed, modeNextPressed, potLevel);
            break;

        case RUNTIME_TEST_SERVO:
            servo_test_update(nowMs, sw2Pressed, potLevel);
            break;

        case RUNTIME_TEST_ULTRASONIC:
            ultrasonic_test_update(nowMs, sw2Pressed);
            break;

        case RUNTIME_TEST_CAMSERVO:
            linear_camera_test_update(nowMs, FALSE, potLevel);
            break;

        case RUNTIME_TEST_ULTRA_ESC:
            ultrasonic_esc_test_update(nowMs);
            break;

        case RUNTIME_TEST_RECEIVER:
            receiver_test_update(nowMs);
            break;

        case RUNTIME_TEST_LINEAR_CAMERA:
        default:
            linear_camera_test_update(nowMs, modeNextPressed, potLevel);
            break;
    }
}

static void runtime_test_exit(RuntimeTestId_t testId)
{
    switch (testId)
    {
        case RUNTIME_TEST_ESC:
            esc_test_exit();
            break;

        case RUNTIME_TEST_SERVO:
            servo_test_exit();
            break;

        case RUNTIME_TEST_ULTRASONIC:
            ultrasonic_test_exit();
            break;

        case RUNTIME_TEST_CAMSERVO:
            linear_camera_test_exit();
            break;

        case RUNTIME_TEST_ULTRA_ESC:
            ultrasonic_esc_test_exit();
            break;

        case RUNTIME_TEST_RECEIVER:
            receiver_test_exit();
            break;

        case RUNTIME_TEST_LINEAR_CAMERA:
        default:
            linear_camera_test_exit();
            break;
    }
}

static void esc_enter(EscRunState_t *st, uint32 nowMs)
{
    st->mode = CAR_INIT;
    st->startGoMs = 0u;
    st->armDoneMs = nowMs + ESC_ARM_TIME_MS;
    st->nextDisplayMs = nowMs;
    st->currentCmdPct = 0;
    st->lastPotLevel = (uint8)POT_CENTER_RAW;

    EscInit(ESC_PWM_CH, ESC_DUTY_MIN, ESC_DUTY_MED, ESC_DUTY_MAX);
    Esc_StopNeutral();
}

static void esc_update(EscRunState_t *st, uint32 nowMs, boolean sw2, boolean sw3, uint8 pot)
{
    st->lastPotLevel = pot;

    if (time_reached(nowMs, st->armDoneMs) != TRUE)
    {
        st->currentCmdPct = 0;
        Esc_StopNeutral();
        StatusLed_Blue();
        return;
    }

    if (st->mode == CAR_INIT)
    {
        st->mode = CAR_IDLE;
    }

    if (sw3 == TRUE)
    {
        st->mode = CAR_IDLE;
        st->currentCmdPct = 0;
        Esc_StopNeutral();
        StatusLed_Blue();
        return;
    }

    if ((st->mode == CAR_IDLE) && (sw2 == TRUE))
    {
        st->mode = CAR_ARMING;
        st->startGoMs = nowMs + START_DELAY_MS;

        EscSetSpeed(0);
        EscSetBrake(0U);
    }

    if ((st->mode == CAR_ARMING) && (time_reached(nowMs, st->startGoMs) == TRUE))
    {
        st->mode = CAR_RUN;
    }

    if (st->mode != CAR_RUN)
    {
        st->currentCmdPct = 0;
        Esc_StopNeutral();
    }
    else
    {
        sint32 cmdPct;

        if (pot <= (uint8)POT_CENTER_RAW)
        {
            uint16 denom = (uint16)(POT_CENTER_RAW - POT_LEFT_RAW);

            if (denom == 0u)
            {
                denom = 1u;
            }

            cmdPct = -((sint32)((uint8)POT_CENTER_RAW - pot) * 100) / (sint32)denom;
        }
        else
        {
            uint16 denom = (uint16)(POT_RIGHT_RAW - POT_CENTER_RAW);

            if (denom == 0u)
            {
                denom = 1u;
            }

            cmdPct = ((sint32)(pot - (uint8)POT_CENTER_RAW) * 100) / (sint32)denom;
        }

        if ((cmdPct < (sint32)MOTOR_DEADBAND_PCT) &&
            (cmdPct > -((sint32)MOTOR_DEADBAND_PCT)))
        {
            cmdPct = 0;
        }

        st->currentCmdPct = (sint16)cmdPct;
        EscSetBrake(0U);
        EscSetSpeed((int)cmdPct);
    }

    if (st->mode == CAR_RUN)
    {
        StatusLed_Green();
    }
    else if (st->mode == CAR_ARMING)
    {
        StatusLed_Cyan();
    }
    else if (st->mode == CAR_ERROR)
    {
        StatusLed_Red();
    }
    else
    {
        StatusLed_Blue();
    }
}

static void esc_test_draw(const EscRunState_t *st)
{
    const char *modeText = "UNKNOWN";
    char percentText[17];

    if (st == NULL_PTR)
    {
        return;
    }

    switch (st->mode)
    {
        case CAR_INIT:
            modeText = "State: INIT";
            break;

        case CAR_IDLE:
            modeText = "State: IDLE";
            break;

        case CAR_ARMING:
            modeText = "State: ARMING";
            break;

        case CAR_RUN:
            modeText = "State: RUN";
            break;

        default:
            modeText = "State: OTHER";
            break;
    }

    DisplayTextPadded(0U, "ESC TEST");
    DisplayTextPadded(1U, modeText);

    DisplayTextPadded(2U, "Pot:");
    DisplayValue(2U, (int)st->lastPotLevel, 3U, 4U);

    DisplayTextPadded(3U, "Cmd:");
    DisplayValue(3U, (int)st->currentCmdPct, 4U, 4U);
    (void)memset(percentText, ' ', 16U);
    percentText[0] = '%';
    percentText[16] = '\0';
    DisplayText(3U, percentText, 1U, 8U);

    DisplayRefresh();
}

static void camservo_apply_profile(CamServoState_t *st, const CamTuneProfile_t *profile)
{
    if ((st == NULL_PTR) || (profile == NULL_PTR))
    {
        return;
    }

    st->activeTune = *profile;

    SteeringLinear_SetTunings(&st->ctrl,
                              profile->kp,
                              profile->kd,
                              1.0f);
    st->ctrl.ki = profile->ki;
    st->ctrl.iTermClamp = profile->iTermClamp;
    SteeringLinear_Reset(&st->ctrl);

    st->steerRaw = 0;
    st->steerFilt = 0;
    st->steerOut = 0;
    SteerStraight();
}

static void camservo_enter_with_profile(CamServoState_t *st, uint32 nowMs, const CamTuneProfile_t *profile)
{
    ServoInit(SERVO_PWM_CH, SERVO_DUTY_MAX, SERVO_DUTY_MIN, SERVO_DUTY_MED);
    SteerStraight();

    if (LinearCameraIsBusy() == TRUE)
    {
        LinearCameraStopStream();
    }

    LinearCameraInit(CAM_CLK_PWM_CH, CAM_SHUTTER_GPT_CH, CAM_ADC_GROUP, CAM_SHUTTER_PCR);
    LinearCameraSetFrameIntervalTicks(CAM_FRAME_INTERVAL_TICKS);
    VisionLinear_InitV2();
    VisionDebug_Init(&st->vdbg, (uint8)V2_WHITE_SAT_U8);

    SteeringLinear_Init(&st->ctrl);
    SteeringLinear_Reset(&st->ctrl);

    (void)memset(&st->processedFrame, 0, sizeof(st->processedFrame));
    (void)memset(&st->result, 0, sizeof(st->result));
    (void)memset(&st->dbg, 0, sizeof(st->dbg));
    (void)memset(st->filteredBuf, 0, sizeof(st->filteredBuf));
    (void)memset(st->gradientBuf, 0, sizeof(st->gradientBuf));

    st->haveValidVision = FALSE;
    st->lastFrameMs = nowMs;
    st->nextTickMs = nowMs;
    st->nextSteerMs = nowMs + STEER_UPDATE_MS;
    st->nextDisplayMs = nowMs;
    st->tickCount = 0u;
    st->displayDirty = FALSE;

    camservo_apply_profile(st, profile);

    (void)LinearCameraStartStream();
}

static void camservo_enter(CamServoState_t *st, uint32 nowMs)
{
    const CamTuneProfile_t defaultProfile =
    {
        KP,
        KD,
        KI,
        ITERM_CLAMP,
        0.45f,
        (sint16)STEER_CMD_CLAMP,
        8,
        20U
    };

    camservo_enter_with_profile(st, nowMs, &defaultProfile);
}

static void camservo_update(CamServoState_t *st, uint32 nowMs, boolean sw2)
{
    const uint32 loopMs = (uint32)V2_LOOP_PERIOD_MS;
    const LinearCameraFrame *latestFrame = NULL_PTR;

    if ((uint32)(nowMs - st->nextTickMs) < loopMs)
    {
        return;
    }

    st->nextTickMs += loopMs;
    st->tickCount++;

    VisionDebug_OnTick(&st->vdbg, sw2, FALSE);
    if (sw2 == TRUE)
    {
        st->displayDirty = TRUE;
    }

    if (LinearCameraGetLatestFrame(&latestFrame) == TRUE)
    {
        st->dbg.mask = (uint32)VLIN_DBG_NONE;
        st->dbg.filteredOut = NULL_PTR;
        st->dbg.gradientOut = NULL_PTR;

        if (VisionDebug_WantsVisionDebugData(&st->vdbg) == TRUE)
        {
            VisionDebug_PrepareVisionDbg(&st->vdbg, &st->dbg, st->filteredBuf, st->gradientBuf);
        }

        (void)memcpy(st->processedFrame.Values,
                     &latestFrame->Values[CAM_TRIM_LEFT_PX],
                     ((size_t)VISION_LINEAR_BUFFER_SIZE * sizeof(st->processedFrame.Values[0])));
        VisionLinear_ProcessFrameEx(st->processedFrame.Values, &st->result, &st->dbg);
        st->haveValidVision = TRUE;
        st->lastFrameMs = nowMs;
        st->displayDirty = TRUE;
    }

    if (time_reached(nowMs, st->nextSteerMs) == TRUE)
    {
        st->nextSteerMs += STEER_UPDATE_MS;

        if (st->haveValidVision != TRUE)
        {
            st->steerRaw = 0;
            st->steerFilt = 0;
            st->steerOut = 0;
            SteerStraight();
            return;
        }

        {
            const float dt = ((float)STEER_UPDATE_MS) * 0.001f;
            SteeringOutput_t out = SteeringLinear_UpdateV2(&st->ctrl,
                                                           &st->result,
                                                           dt,
                                                           st->activeTune.baseSpeedPct);

            st->steerRaw = (sint16)out.steer_cmd;
        }

        if (((st->steerRaw < 0) ? (sint16)(-st->steerRaw) : st->steerRaw) <= 2)
        {
            st->steerRaw = 0;
        }

        st->steerFilt = SteeringSmooth_IirS16(st->steerFilt,
                                              st->steerRaw,
                                              st->activeTune.steerLpfAlpha);

        {
            sint16 delta = (sint16)(st->steerFilt - st->steerOut);

            delta = SteeringSmooth_ClampS16(delta,
                                            (sint16)(-st->activeTune.steerRateMax),
                                            (sint16)(+st->activeTune.steerRateMax));
            st->steerOut = (sint16)(st->steerOut + delta);
        }

        st->steerOut = SteeringSmooth_ClampS16(st->steerOut,
                                               (sint16)(-st->activeTune.steerClamp),
                                               (sint16)(+st->activeTune.steerClamp));

        Steer((int)st->steerOut);
    }

    if ((st->haveValidVision == TRUE) &&
        (st->displayDirty == TRUE) &&
        (time_reached(nowMs, st->nextDisplayMs) == TRUE))
    {
        const uint16 *pFiltered = (st->dbg.filteredOut != NULL_PTR) ? st->filteredBuf : NULL_PTR;
        const sint16 *pGradient = (st->dbg.gradientOut != NULL_PTR) ? st->gradientBuf : NULL_PTR;
        const VisionLinear_DebugOut_t *pDbg =
            (st->dbg.mask != (uint32)VLIN_DBG_NONE) ? &st->dbg : NULL_PTR;

        st->nextDisplayMs = nowMs + CAM_DEBUG_DISPLAY_PERIOD_MS;
        st->displayDirty = FALSE;
        VisionDebug_Draw(&st->vdbg, st->processedFrame.Values, pFiltered, pGradient, &st->result, pDbg);
    }
}

static void camservo_exit(void)
{
    SteerStraight();

    if (LinearCameraIsBusy() == TRUE)
    {
        LinearCameraStopStream();
    }
}

static void final_dummy_enter(uint32 nowMs)
{
    (void)memset(&g_finalDummy, 0, sizeof(g_finalDummy));

    g_finalDummy.state = DUMMY_ESC;
    esc_enter(&g_finalDummy.escSt, nowMs);
    StatusLed_Blue();
}

static void final_dummy_update(uint32 nowMs, boolean sw2Pressed, boolean sw3Pressed, uint8 potLevel)
{
    if (sw3Pressed == TRUE)
    {
        if (g_finalDummy.state == DUMMY_ESC)
        {
            Esc_StopNeutral();
            camservo_enter(&g_finalDummy.camSt, nowMs);
            g_finalDummy.state = DUMMY_CAM;
            StatusLed_Cyan();

            g_finalDummy.autoSpeedPct = 0;
            g_finalDummy.nextAutoSpeedMs = nowMs;
            EscSetBrake(0U);
            EscSetSpeed(0);
        }
        else
        {
            camservo_exit();
            esc_enter(&g_finalDummy.escSt, nowMs);
            g_finalDummy.state = DUMMY_ESC;
            StatusLed_Blue();

            g_finalDummy.autoSpeedPct = 0;
            g_finalDummy.nextAutoSpeedMs = 0u;
        }

        sw3Pressed = FALSE;
    }

    if (g_finalDummy.state == DUMMY_ESC)
    {
        esc_update(&g_finalDummy.escSt, nowMs, sw2Pressed, sw3Pressed, potLevel);
    }
    else
    {
        camservo_update(&g_finalDummy.camSt, nowMs, sw2Pressed);

        if (time_reached(nowMs, g_finalDummy.nextAutoSpeedMs) == TRUE)
        {
            g_finalDummy.nextAutoSpeedMs = nowMs + FULL_AUTO_RAMP_PERIOD_MS;

            if (g_finalDummy.autoSpeedPct < (sint32)FULL_AUTO_SPEED_PCT)
            {
                g_finalDummy.autoSpeedPct += (sint32)FULL_AUTO_RAMP_STEP_PCT;
                if (g_finalDummy.autoSpeedPct > (sint32)FULL_AUTO_SPEED_PCT)
                {
                    g_finalDummy.autoSpeedPct = (sint32)FULL_AUTO_SPEED_PCT;
                }
            }

            if (g_finalDummy.autoSpeedPct < 0)
            {
                g_finalDummy.autoSpeedPct = 0;
            }
            if (g_finalDummy.autoSpeedPct > 100)
            {
                g_finalDummy.autoSpeedPct = 100;
            }

            EscSetBrake(0U);
            EscSetSpeed((int)(-g_finalDummy.autoSpeedPct));
        }

        if ((g_finalDummy.camSt.haveValidVision == TRUE) &&
            (g_finalDummy.camSt.result.status != VISION_TRACK_LOST))
        {
            StatusLed_Cyan();
        }
        else
        {
            StatusLed_Yellow();
        }
    }
}

static uint8 tests_menu_map_pot_to_index(uint8 pot, uint8 nItems)
{
    uint16 scaled = (uint16)pot * (uint16)nItems;
    uint8 idx = (uint8)(scaled / 256u);

    if (idx >= nItems)
    {
        idx = (uint8)(nItems - 1u);
    }

    return idx;
}

static void tests_menu_select_index(TestsMenuState_t *st, uint8 pot)
{
    uint32 nowMs;
    uint8 newIdx;

    if (st == NULL_PTR)
    {
        return;
    }

    nowMs = Timebase_GetMs();
    if ((uint32)(nowMs - st->lastPotMs) < TESTS_MENU_POT_STABLE_MS)
    {
        return;
    }
    st->lastPotMs = nowMs;

    newIdx = tests_menu_map_pot_to_index(pot, (uint8)RUNTIME_TEST_COUNT);

    if (newIdx != st->selectedIndex)
    {
        uint16 regionW = 256u / (uint16)RUNTIME_TEST_COUNT;
        uint16 newCenter = (uint16)newIdx * regionW + (regionW / 2u);
        uint16 potU = (uint16)pot;
        uint16 diff = (potU > newCenter) ? (potU - newCenter) : (newCenter - potU);

        if (diff >= TESTS_MENU_POT_HYSTERESIS)
        {
            st->selectedIndex = newIdx;
        }
    }

    if (st->selectedIndex < st->topIndex)
    {
        st->topIndex = st->selectedIndex;
    }
    else if (st->selectedIndex >= (uint8)(st->topIndex + TESTS_MENU_VISIBLE_LINES))
    {
        st->topIndex = (uint8)(st->selectedIndex - (TESTS_MENU_VISIBLE_LINES - 1u));
    }

    if (st->topIndex > (uint8)(RUNTIME_TEST_COUNT - TESTS_MENU_VISIBLE_LINES))
    {
        st->topIndex = (uint8)(RUNTIME_TEST_COUNT - TESTS_MENU_VISIBLE_LINES);
    }
}

static void tests_menu_draw(const TestsMenuState_t *st)
{
    if (st == NULL_PTR)
    {
        return;
    }

    DisplayClear();
    DisplayTextPadded(0U, "NXP CUP TESTS");

    for (uint8 line = 0u; line < TESTS_MENU_VISIBLE_LINES; line++)
    {
        uint8 idx = (uint8)(st->topIndex + line);
        uint8 row = (uint8)(1u + line);

        DisplayTextPadded(row, "");
        if (idx == st->selectedIndex)
        {
            DisplayTextPadded(row, ">");
        }
        DisplayText(row, g_testsMenuItems[idx], 15U, 1U);
    }

    DisplayRefresh();
    StatusLed_Blue();
}

static void mode_nxp_cup_tests(void)
{
    uint32 nextButtonsMs;

    App_InitRuntimeCommon();
    (void)memset(&g_testsMenu, 0, sizeof(g_testsMenu));
    g_testsMenu.activeTest = RUNTIME_TEST_LINEAR_CAMERA;

    nextButtonsMs = Timebase_GetMs();

    for (;;)
    {
        uint32 nowMs = Timebase_GetMs();
        boolean sw2Pressed = FALSE;
        boolean modeSwitchOn;
        uint8 potLevel;

        while (time_reached(nowMs, nextButtonsMs) == TRUE)
        {
            Buttons_Update();
            nextButtonsMs += BUTTONS_PERIOD_MS;
        }

        potLevel = OnboardPot_ReadLevelFiltered();

        modeSwitchOn = Buttons_IsOn(BUTTON_ID_SWPCB);

        if (g_testsMenu.testActive != TRUE)
        {
            (void)Buttons_WasPressed(BUTTON_ID_SW2);
            (void)Buttons_WasPressed(BUTTON_ID_SW3);
            tests_menu_select_index(&g_testsMenu, potLevel);
            tests_menu_draw(&g_testsMenu);

            if (modeSwitchOn == TRUE)
            {
                g_testsMenu.activeTest = (RuntimeTestId_t)g_testsMenu.selectedIndex;
                runtime_test_enter(g_testsMenu.activeTest, nowMs);
                g_testsMenu.testActive = TRUE;
            }
        }
        else
        {
            if (modeSwitchOn != TRUE)
            {
                runtime_test_exit(g_testsMenu.activeTest);
                g_testsMenu.testActive = FALSE;
                tests_menu_draw(&g_testsMenu);
            }
            else
            {
                sw2Pressed = Buttons_WasPressed(BUTTON_ID_SW2);
                runtime_test_update(g_testsMenu.activeTest, nowMs, sw2Pressed, FALSE, potLevel);
            }
        }
    }
}

static void nxp_cup_idle_motor(void)
{
    Esc_StopNeutral();
}

static void nxp_cup_obstacle_stop_motor(void)
{
    Esc_StopNeutral();
}

static void nxp_cup_launch_motor(int logicalCmd)
{
    EscSetBrake(0U);
    EscSetSpeed(logicalCmd);
}

static uint8 nxp_cup_profile_from_pot(uint8 pot)
{
    if (pot < 85U)
    {
        return (uint8)NXP_CUP_PROFILE_SUPERFAST;
    }

    if (pot < 171U)
    {
        return (uint8)NXP_CUP_PROFILE_5050;
    }

    return (uint8)NXP_CUP_PROFILE_SLOW;
}

static void nxp_cup_ultra_enter(NxpCupUltraState_t *st, uint32 nowMs)
{
    if (st == NULL_PTR)
    {
        return;
    }

    (void)memset(st, 0, sizeof(*st));
    Ultrasonic_Init();
    st->nextTriggerMs = nowMs;
    st->mode = NXP_CUP_ULTRA_CLEAR;
}

static void nxp_cup_ultra_arm_for_run(NxpCupUltraState_t *st, uint32 nowMs)
{
    if (st == NULL_PTR)
    {
        return;
    }

    st->ultraEnableMs = nowMs + NXP_ULTRA_ENABLE_AFTER_RUN_MS;
}

static boolean nxp_cup_ultra_is_active(const NxpCupUltraState_t *st, uint32 nowMs)
{
    if ((st == NULL_PTR) || (NXP_CUP_ULTRASONIC_ENABLE != 1))
    {
        return FALSE;
    }

    return (boolean)((st->ultraEnableMs != 0U) &&
                     (time_reached(nowMs, st->ultraEnableMs) == TRUE));
}

static boolean nxp_cup_ultra_mode_is_obstacle(const NxpCupUltraState_t *st)
{
    if (st == NULL_PTR)
    {
        return FALSE;
    }

    return (boolean)((st->mode == NXP_CUP_ULTRA_STOP_HOLD) ||
                     (st->mode == NXP_CUP_ULTRA_CRAWL) ||
                     (st->mode == NXP_CUP_ULTRA_CUTOFF));
}

static void nxp_cup_ultra_task(NxpCupUltraState_t *st, uint32 nowMs)
{
    const uint32 ultraStaleMs =
        ((uint32)NXP_CUP_ULTRA_TRIGGER_PERIOD_MS * 2U) + (uint32)ULTRA_TIMEOUT_MS;
    float distanceCm = 0.0f;

    if (st == NULL_PTR)
    {
        return;
    }

    Ultrasonic_Task();

    if (time_reached(nowMs, st->nextTriggerMs) == TRUE)
    {
        st->nextTriggerMs = nowMs + NXP_CUP_ULTRA_TRIGGER_PERIOD_MS;

        if (Ultrasonic_GetStatus() != ULTRA_STATUS_BUSY)
        {
            Ultrasonic_StartMeasurement();
        }
    }

    if (Ultrasonic_GetDistanceCm(&distanceCm) == TRUE)
    {
        st->lastDistanceCm = distanceCm;
        st->lastDistanceMs = nowMs;
        st->haveValidDistance = (boolean)(distanceCm > 0.0f);
    }
    else if ((st->haveValidDistance == TRUE) &&
             ((uint32)(nowMs - st->lastDistanceMs) > ultraStaleMs))
    {
        st->haveValidDistance = FALSE;
    }

    if (nxp_cup_ultra_is_active(st, nowMs) != TRUE)
    {
        st->mode = NXP_CUP_ULTRA_CLEAR;
        st->modeUntilMs = 0U;
        return;
    }

    if (st->haveValidDistance != TRUE)
    {
        return;
    }

    if (st->lastDistanceCm <= (float)NXP_CUP_ULTRA_CRAWL_STOP_CM)
    {
        st->mode = NXP_CUP_ULTRA_CUTOFF;
        st->modeUntilMs = 0U;
        return;
    }

    switch (st->mode)
    {
        case NXP_CUP_ULTRA_STOP_HOLD:
            if ((st->modeUntilMs != 0U) &&
                (time_reached(nowMs, st->modeUntilMs) == TRUE))
            {
                st->mode = (st->lastDistanceCm <= (float)NXP_CUP_ULTRA_STOP_CM) ?
                    NXP_CUP_ULTRA_CRAWL : NXP_CUP_ULTRA_CLEAR;
                st->modeUntilMs = 0U;
            }
            break;

        case NXP_CUP_ULTRA_CRAWL:
            if (st->lastDistanceCm > (float)NXP_CUP_ULTRA_STOP_CM)
            {
                st->mode = NXP_CUP_ULTRA_CLEAR;
            }
            break;

        case NXP_CUP_ULTRA_CUTOFF:
            break;

        case NXP_CUP_ULTRA_CLEAR:
        default:
            if (st->lastDistanceCm <= (float)NXP_CUP_ULTRA_STOP_CM)
            {
                st->mode = NXP_CUP_ULTRA_STOP_HOLD;
                st->modeUntilMs = nowMs + NXP_CUP_ULTRA_STOP_HOLD_MS;
            }
            break;
    }
}

static boolean nxp_cup_ultra_should_hold_servo(const NxpCupUltraState_t *st, uint32 nowMs)
{
    (void)nowMs;
    return nxp_cup_ultra_mode_is_obstacle(st);
}

static uint8 nxp_cup_ultra_target_speed_pct(const NxpCupUltraState_t *st, uint32 nowMs, uint8 defaultSpeedPct)
{
    if ((st == NULL_PTR) || (nxp_cup_ultra_is_active(st, nowMs) != TRUE))
    {
        return defaultSpeedPct;
    }

    switch (st->mode)
    {
        case NXP_CUP_ULTRA_STOP_HOLD:
        case NXP_CUP_ULTRA_CUTOFF:
            return 0U;

        case NXP_CUP_ULTRA_CRAWL:
            return (uint8)((NXP_CUP_ULTRA_CRAWL_LOGICAL_CMD < 0) ?
                (-NXP_CUP_ULTRA_CRAWL_LOGICAL_CMD) :
                NXP_CUP_ULTRA_CRAWL_LOGICAL_CMD);

        case NXP_CUP_ULTRA_CLEAR:
        default:
            return defaultSpeedPct;
    }
}

static void mode_nxp_cup(void)
{
    uint32 nextButtonsMs;

    App_InitRuntimeCommon();
    (void)memset(&g_nxpCup, 0, sizeof(g_nxpCup));

    g_nxpCup.state = NXP_CUP_STATE_MENU;
    g_nxpCup.profileId = (NxpCupProfileId_t)NXP_CUP_DEFAULT_PROFILE;
    g_nxpCup.nextDisplayMs = Timebase_GetMs();
    nextButtonsMs = Timebase_GetMs();

    nxp_cup_idle_motor();
    nxp_cup_ultra_enter(&g_nxpCup.ultraSt, Timebase_GetMs());
    StatusLed_Blue();

    for (;;)
    {
        uint32 nowMs = Timebase_GetMs();
        boolean sw2Pressed;
        boolean sw3Pressed;
        boolean systemBad = FALSE;
        uint8 potLevel;

        while (time_reached(nowMs, nextButtonsMs) == TRUE)
        {
            Buttons_Update();
            nextButtonsMs += BUTTONS_PERIOD_MS;
        }

        sw2Pressed = Buttons_WasPressed(BUTTON_ID_SW2);
        sw3Pressed = Buttons_WasPressed(BUTTON_ID_SW3);
        potLevel = OnboardPot_ReadLevelFiltered();

        if ((g_nxpCup.state == NXP_CUP_STATE_RUN) &&
            (g_nxpCup.cameraStarted == TRUE) &&
            (nxp_cup_ultra_mode_is_obstacle(&g_nxpCup.ultraSt) != TRUE) &&
            ((g_nxpCup.camSt.haveValidVision != TRUE) ||
             ((uint32)(nowMs - g_nxpCup.camSt.lastFrameMs) > CAM_STEER_HOLD_MS) ||
             (g_nxpCup.camSt.result.status == VISION_TRACK_LOST)))
        {
            systemBad = TRUE;
        }

        if ((g_nxpCup.state == NXP_CUP_STATE_RUN) &&
            ((g_nxpCup.ultraSt.mode == NXP_CUP_ULTRA_STOP_HOLD) ||
             (g_nxpCup.ultraSt.mode == NXP_CUP_ULTRA_CUTOFF)))
        {
            StatusLed_Red();
        }
        else if ((g_nxpCup.state == NXP_CUP_STATE_RUN) &&
                 (g_nxpCup.ultraSt.mode == NXP_CUP_ULTRA_CRAWL))
        {
            StatusLed_Yellow();
        }
        else if (systemBad == TRUE)
        {
            StatusLed_Red();
            g_nxpCup.autoSpeedPct = 0;
            nxp_cup_idle_motor();
        }
        else if ((g_nxpCup.state == NXP_CUP_STATE_RUN) ||
                 (g_nxpCup.state == NXP_CUP_STATE_ESC_REARM))
        {
            StatusLed_Green();
        }
        else
        {
            StatusLed_Blue();
        }

        if (time_reached(nowMs, g_nxpCup.nextDisplayMs) == TRUE)
        {
            g_nxpCup.nextDisplayMs = nowMs + DISPLAY_PERIOD_MS;
            DisplayClear();

            switch (g_nxpCup.state)
            {
                case NXP_CUP_STATE_MENU:
                    g_nxpCup.profileId = (NxpCupProfileId_t)nxp_cup_profile_from_pot(potLevel);
                    DisplayTextPadded(0U, "NXP CUP MENU");
                    if (g_nxpCup.profileId == NXP_CUP_PROFILE_SUPERFAST)
                    {
                        DisplayTextPadded(1U, ">SUPERFAST");
                        DisplayTextPadded(2U, " 5050");
                        DisplayTextPadded(3U, " SLOW SW2 ENTER");
                    }
                    else if (g_nxpCup.profileId == NXP_CUP_PROFILE_5050)
                    {
                        DisplayTextPadded(1U, " SUPERFAST");
                        DisplayTextPadded(2U, ">5050");
                        DisplayTextPadded(3U, " SLOW SW2 ENTER");
                    }
                    else
                    {
                        DisplayTextPadded(1U, " SUPERFAST");
                        DisplayTextPadded(2U, " 5050");
                        DisplayTextPadded(3U, ">SLOW SW2 ENTER");
                    }
                    break;

                case NXP_CUP_STATE_READY:
                    DisplayTextPadded(0U, "NXP CUP READY");
                    if (g_nxpCup.profileId == NXP_CUP_PROFILE_SUPERFAST)
                    {
                        DisplayTextPadded(1U, "PROFILE:SUPER");
                    }
                    else if (g_nxpCup.profileId == NXP_CUP_PROFILE_5050)
                    {
                        DisplayTextPadded(1U, "PROFILE:5050");
                    }
                    else
                    {
                        DisplayTextPadded(1U, "PROFILE:SLOW");
                    }
                    DisplayTextPadded(2U, "SW3 START");
                    DisplayTextPadded(3U, "SW2 BACK");
                    break;

                case NXP_CUP_STATE_ESC_REARM:
                    DisplayTextPadded(0U, "NXP CUP ARM");
                    DisplayTextPadded(1U, "ESC re-arming");
                    DisplayTextPadded(2U, "SW3 ready");
                    DisplayTextPadded(3U, "SW2 menu");
                    break;

                case NXP_CUP_STATE_RUN:
                default:
                    DisplayTextPadded(0U, "NXP CUP RUN");
                    DisplayTextPadded(1U, "SPD:    D:--- ");
                    DisplayValue(1U, (int)g_nxpCup.autoSpeedPct, 3U, 4U);
                    if (g_nxpCup.ultraSt.haveValidDistance == TRUE)
                    {
                        DisplayValue(1U, (int)(g_nxpCup.ultraSt.lastDistanceCm + 0.5f), 3U, 11U);
                    }

                    if (systemBad == TRUE)
                    {
                        DisplayTextPadded(2U, "MODE: FAULT");
                    }
                    else if (g_nxpCup.ultraSt.mode == NXP_CUP_ULTRA_STOP_HOLD)
                    {
                        DisplayTextPadded(2U, "MODE: STOP");
                    }
                    else if (g_nxpCup.ultraSt.mode == NXP_CUP_ULTRA_CRAWL)
                    {
                        DisplayTextPadded(2U, "MODE: CRAWL");
                    }
                    else if (g_nxpCup.ultraSt.mode == NXP_CUP_ULTRA_CUTOFF)
                    {
                        DisplayTextPadded(2U, "MODE: CUTOFF");
                    }
                    else if ((g_nxpCup.camSt.haveValidVision == TRUE) &&
                             (g_nxpCup.camSt.result.status != VISION_TRACK_LOST))
                    {
                        DisplayTextPadded(2U, "MODE: TRACK");
                    }
                    else
                    {
                        DisplayTextPadded(2U, "MODE: WAIT");
                    }

                    DisplayTextPadded(3U, "SW3 READY SW2 M");
                    break;
            }

            DisplayRefresh();
        }

        if (g_nxpCup.state == NXP_CUP_STATE_MENU)
        {
            if (g_nxpCup.cameraStarted == TRUE)
            {
                camservo_exit();
                g_nxpCup.cameraStarted = FALSE;
            }

            nxp_cup_idle_motor();
            SteerStraight();
            g_nxpCup.autoSpeedPct = 0;
            g_nxpCup.nextAutoSpeedMs = 0U;
            g_nxpCup.escRearmDoneMs = 0U;

            if (sw2Pressed == TRUE)
            {
                g_nxpCup.state = NXP_CUP_STATE_READY;
            }

            continue;
        }

        if (g_nxpCup.state == NXP_CUP_STATE_READY)
        {
            if (g_nxpCup.cameraStarted == TRUE)
            {
                camservo_exit();
                g_nxpCup.cameraStarted = FALSE;
            }

            nxp_cup_idle_motor();
            SteerStraight();
            g_nxpCup.autoSpeedPct = 0;
            g_nxpCup.nextAutoSpeedMs = 0U;
            g_nxpCup.escRearmDoneMs = 0U;

            if (sw2Pressed == TRUE)
            {
                g_nxpCup.state = NXP_CUP_STATE_MENU;
                continue;
            }

            if (sw3Pressed == TRUE)
            {
                camservo_enter_with_profile(&g_nxpCup.camSt,
                                            nowMs,
                                            &g_nxpCupProfiles[g_nxpCup.profileId]);
                g_nxpCup.cameraStarted = TRUE;
                g_nxpCup.autoSpeedPct = 0;
                g_nxpCup.nextAutoSpeedMs = 0U;

                nxp_cup_ultra_enter(&g_nxpCup.ultraSt, nowMs);
                EscInit(ESC_PWM_CH, ESC_DUTY_MIN, ESC_DUTY_MED, ESC_DUTY_MAX);
                g_nxpCup.escRearmDoneMs = nowMs + ESC_ARM_TIME_MS + NXP_ESC_EXTRA_SETTLE_MS;
                g_nxpCup.state = NXP_CUP_STATE_ESC_REARM;
                continue;
            }

            continue;
        }

        if (g_nxpCup.state == NXP_CUP_STATE_ESC_REARM)
        {
            nxp_cup_ultra_task(&g_nxpCup.ultraSt, nowMs);

            if (g_nxpCup.cameraStarted == TRUE)
            {
                camservo_update(&g_nxpCup.camSt, nowMs, FALSE);
            }

            if (sw2Pressed == TRUE)
            {
                nxp_cup_idle_motor();
                if (g_nxpCup.cameraStarted == TRUE)
                {
                    camservo_exit();
                    g_nxpCup.cameraStarted = FALSE;
                }
                SteerStraight();
                g_nxpCup.autoSpeedPct = 0;
                g_nxpCup.nextAutoSpeedMs = 0U;
                g_nxpCup.escRearmDoneMs = 0U;
                g_nxpCup.state = NXP_CUP_STATE_MENU;
                continue;
            }

            if (sw3Pressed == TRUE)
            {
                nxp_cup_idle_motor();
                if (g_nxpCup.cameraStarted == TRUE)
                {
                    camservo_exit();
                    g_nxpCup.cameraStarted = FALSE;
                }
                SteerStraight();
                g_nxpCup.autoSpeedPct = 0;
                g_nxpCup.nextAutoSpeedMs = 0U;
                g_nxpCup.escRearmDoneMs = 0U;
                g_nxpCup.state = NXP_CUP_STATE_READY;
                continue;
            }

            nxp_cup_idle_motor();

            if (time_reached(nowMs, g_nxpCup.escRearmDoneMs) == TRUE)
            {
                int logicalCmd;

                g_nxpCup.autoSpeedPct = (sint32)g_nxpCup.camSt.activeTune.baseSpeedPct;
                if (g_nxpCup.autoSpeedPct < 0)
                {
                    g_nxpCup.autoSpeedPct = 0;
                }
                if (g_nxpCup.autoSpeedPct > 100)
                {
                    g_nxpCup.autoSpeedPct = 100;
                }

                nxp_cup_ultra_arm_for_run(&g_nxpCup.ultraSt, nowMs);
                nxp_cup_ultra_task(&g_nxpCup.ultraSt, nowMs);

                if ((g_nxpCup.ultraSt.mode == NXP_CUP_ULTRA_STOP_HOLD) ||
                    (g_nxpCup.ultraSt.mode == NXP_CUP_ULTRA_CUTOFF))
                {
                    g_nxpCup.autoSpeedPct = 0;
                    nxp_cup_obstacle_stop_motor();
                    SteerStraight();
                }
                else
                {
                    if (g_nxpCup.ultraSt.mode == NXP_CUP_ULTRA_CRAWL)
                    {
                        logicalCmd = (int)NXP_CUP_ULTRA_CRAWL_LOGICAL_CMD;
                        g_nxpCup.autoSpeedPct = (logicalCmd < 0) ? -logicalCmd : logicalCmd;
                    }
                    else
                    {
                        g_nxpCup.autoSpeedPct =
                            (sint32)nxp_cup_ultra_target_speed_pct(&g_nxpCup.ultraSt,
                                                                   nowMs,
                                                                   g_nxpCup.camSt.activeTune.baseSpeedPct);
                        logicalCmd = (int)(-g_nxpCup.autoSpeedPct);
                    }

                    nxp_cup_launch_motor(logicalCmd);
                }

                g_nxpCup.nextAutoSpeedMs = nowMs + FULL_AUTO_RAMP_PERIOD_MS;
                g_nxpCup.state = NXP_CUP_STATE_RUN;
            }

            continue;
        }

        nxp_cup_ultra_task(&g_nxpCup.ultraSt, nowMs);

        if (g_nxpCup.cameraStarted == TRUE)
        {
            if (nxp_cup_ultra_should_hold_servo(&g_nxpCup.ultraSt, nowMs) == TRUE)
            {
                SteerStraight();
            }
            else
            {
                camservo_update(&g_nxpCup.camSt, nowMs, FALSE);
            }
        }

        if ((g_nxpCup.ultraSt.mode == NXP_CUP_ULTRA_STOP_HOLD) ||
            (g_nxpCup.ultraSt.mode == NXP_CUP_ULTRA_CUTOFF))
        {
            g_nxpCup.autoSpeedPct = 0;
            nxp_cup_obstacle_stop_motor();
        }
        else if (g_nxpCup.ultraSt.mode == NXP_CUP_ULTRA_CRAWL)
        {
            int crawlCmd = (int)NXP_CUP_ULTRA_CRAWL_LOGICAL_CMD;

            g_nxpCup.autoSpeedPct = (crawlCmd < 0) ? -crawlCmd : crawlCmd;
            nxp_cup_launch_motor(crawlCmd);
        }

        if (sw2Pressed == TRUE)
        {
            nxp_cup_idle_motor();
            if (g_nxpCup.cameraStarted == TRUE)
            {
                camservo_exit();
                g_nxpCup.cameraStarted = FALSE;
            }
            SteerStraight();
            g_nxpCup.autoSpeedPct = 0;
            g_nxpCup.nextAutoSpeedMs = 0U;
            g_nxpCup.escRearmDoneMs = 0U;
            g_nxpCup.state = NXP_CUP_STATE_MENU;
            continue;
        }

        if (sw3Pressed == TRUE)
        {
            nxp_cup_idle_motor();
            if (g_nxpCup.cameraStarted == TRUE)
            {
                camservo_exit();
                g_nxpCup.cameraStarted = FALSE;
            }
            SteerStraight();
            g_nxpCup.autoSpeedPct = 0;
            g_nxpCup.nextAutoSpeedMs = 0U;
            g_nxpCup.escRearmDoneMs = 0U;
            g_nxpCup.state = NXP_CUP_STATE_READY;
            continue;
        }

        if (time_reached(nowMs, g_nxpCup.nextAutoSpeedMs) == TRUE)
        {
            sint32 targetSpeedPct;
            int logicalCmd;

            g_nxpCup.nextAutoSpeedMs = nowMs + FULL_AUTO_RAMP_PERIOD_MS;
            targetSpeedPct =
                (sint32)nxp_cup_ultra_target_speed_pct(&g_nxpCup.ultraSt,
                                                       nowMs,
                                                       g_nxpCup.camSt.activeTune.baseSpeedPct);

            if (g_nxpCup.ultraSt.mode == NXP_CUP_ULTRA_CRAWL)
            {
                logicalCmd = (int)NXP_CUP_ULTRA_CRAWL_LOGICAL_CMD;
                g_nxpCup.autoSpeedPct = (logicalCmd < 0) ? -logicalCmd : logicalCmd;
            }
            else if (g_nxpCup.autoSpeedPct < targetSpeedPct)
            {
                g_nxpCup.autoSpeedPct += (sint32)FULL_AUTO_RAMP_STEP_PCT;
                if (g_nxpCup.autoSpeedPct > targetSpeedPct)
                {
                    g_nxpCup.autoSpeedPct = targetSpeedPct;
                }
                logicalCmd = (int)(-g_nxpCup.autoSpeedPct);
            }
            else if (g_nxpCup.autoSpeedPct > targetSpeedPct)
            {
                g_nxpCup.autoSpeedPct -= (sint32)NXP_CUP_RAMP_DOWN_STEP_PCT;
                if (g_nxpCup.autoSpeedPct < targetSpeedPct)
                {
                    g_nxpCup.autoSpeedPct = targetSpeedPct;
                }
                logicalCmd = (int)(-g_nxpCup.autoSpeedPct);
            }
            else
            {
                logicalCmd = (int)(-g_nxpCup.autoSpeedPct);
            }

            if (g_nxpCup.autoSpeedPct < 0)
            {
                g_nxpCup.autoSpeedPct = 0;
            }
            if (g_nxpCup.autoSpeedPct > 100)
            {
                g_nxpCup.autoSpeedPct = 100;
            }

            nxp_cup_launch_motor(logicalCmd);
        }
    }
}

static void mode_linear_camera_test(void)
{
    uint32 nextButtonsMs;

    App_InitRuntimeCommon();
    runtime_test_enter(RUNTIME_TEST_LINEAR_CAMERA, Timebase_GetMs());

    nextButtonsMs = Timebase_GetMs();

    for (;;)
    {
        uint32 nowMs = Timebase_GetMs();
        boolean sw3Pressed;
        uint8 potLevel;

        while (time_reached(nowMs, nextButtonsMs) == TRUE)
        {
            Buttons_Update();
            nextButtonsMs += BUTTONS_PERIOD_MS;
        }

        sw3Pressed = Buttons_WasPressed(BUTTON_ID_SW3);
        (void)Buttons_WasPressed(BUTTON_ID_SW2);
        potLevel = OnboardPot_ReadLevelFiltered();

        runtime_test_update(RUNTIME_TEST_LINEAR_CAMERA, nowMs, FALSE, sw3Pressed, potLevel);
    }
}

static void mode_final_dummy(void)
{
    uint32 nextButtonsMs;

    App_InitRuntimeCommon();
    final_dummy_enter(Timebase_GetMs());

    nextButtonsMs = Timebase_GetMs();

    for (;;)
    {
        uint32 nowMs = Timebase_GetMs();
        boolean sw2Pressed;
        boolean sw3Pressed;
        uint8 potLevel;

        while (time_reached(nowMs, nextButtonsMs) == TRUE)
        {
            Buttons_Update();
            nextButtonsMs += BUTTONS_PERIOD_MS;
        }

        sw2Pressed = Buttons_WasPressed(BUTTON_ID_SW2);
        sw3Pressed = Buttons_WasPressed(BUTTON_ID_SW3);
        potLevel = OnboardPot_ReadLevelFiltered();

        final_dummy_update(nowMs, sw2Pressed, sw3Pressed, potLevel);
    }
}

static sint32 honor_speed_from_distance(boolean hasValidDistance, float distanceCm)
{
    if (hasValidDistance == TRUE)
    {
        if (distanceCm <= HONOR_STOP_DISTANCE_CM)
        {
            return (sint32)HONOR_STOP_SPEED_PCT;
        }

        if (distanceCm <= HONOR_SLOW2_DISTANCE_CM)
        {
            return (sint32)HONOR_SLOW2_SPEED_PCT;
        }

        if (distanceCm <= HONOR_SLOW1_DISTANCE_CM)
        {
            return (sint32)HONOR_SLOW1_SPEED_PCT;
        }
    }

    return (sint32)HONOR_BASE_SPEED_PCT;
}

static void honor_lap_draw_overlay(void)
{
    DisplayTextPadded(3U, "V:     D:      ");
    DisplayValue(3U, (int)g_honorLap.commandedSpeedPct, 3U, 2U);
    DisplayText(3U, "%", 1U, 5U);

    if (g_honorLap.hasValidDistance == TRUE)
    {
        DisplayValue(3U, (int)(g_honorLap.lastDistanceCm + 0.5f), 3U, 10U);
        DisplayText(3U, "cm", 2U, 13U);
    }
    else
    {
        DisplayText(3U, "---cm", 5U, 10U);
    }

    DisplayRefresh();
}

static void honor_lap_enter(uint32 nowMs)
{
    uint32 armStartMs;

    (void)memset(&g_honorLap, 0, sizeof(g_honorLap));

    EscInit(ESC_PWM_CH, ESC_DUTY_MIN, ESC_DUTY_MED, ESC_DUTY_MAX);
    Esc_StopNeutral();

    armStartMs = Timebase_GetMs();
    while ((uint32)(Timebase_GetMs() - armStartMs) < ESC_ARM_TIME_MS)
    {
        Esc_StopNeutral();
    }

    linear_camera_test_enter_common(nowMs, TRUE);
    g_linearCameraTest.ctrl.kp = HONOR_KP;
    g_linearCameraTest.ctrl.kd = HONOR_KD;
    g_linearCameraTest.ctrl.ki = HONOR_KI;
    g_linearCameraTest.ctrl.iTermClamp = ITERM_CLAMP;
    g_linearCameraTest.ctrl.steerScale = HONOR_STEER_SCALE;
    g_linearCameraTest.steeringBaseSpeed = HONOR_FAKE_SPEED;

    Ultrasonic_Init();
    (void)Ultrasonic_GetDistanceCm(&g_honorLap.lastDistanceCm);
    Ultrasonic_StartMeasurement();

    g_honorLap.nextUltraTrigMs = nowMs + HONOR_ULTRA_TRIGGER_PERIOD_MS;
    g_honorLap.nextDisplayMs = nowMs;
    g_honorLap.commandedSpeedPct = 0;

    StatusLed_Cyan();
}

static void honor_lap_update(uint32 nowMs, boolean modeNextPressed, uint8 potLevel)
{
    const uint32 ultraStaleMs =
        ((uint32)HONOR_ULTRA_TRIGGER_PERIOD_MS * 2U) + (uint32)ULTRA_TIMEOUT_MS;
    float distanceCm = 0.0f;
    Ultrasonic_StatusType ultraStatus;

    linear_camera_test_update(nowMs, modeNextPressed, potLevel);

    Ultrasonic_Task();

    if (time_reached(nowMs, g_honorLap.nextUltraTrigMs) == TRUE)
    {
        g_honorLap.nextUltraTrigMs = nowMs + HONOR_ULTRA_TRIGGER_PERIOD_MS;

        if (Ultrasonic_GetStatus() != ULTRA_STATUS_BUSY)
        {
            Ultrasonic_StartMeasurement();
        }
    }

    ultraStatus = Ultrasonic_GetStatus();

    if (Ultrasonic_GetDistanceCm(&distanceCm) == TRUE)
    {
        g_honorLap.lastDistanceCm = distanceCm;
        g_honorLap.lastDistanceMs = nowMs;
        g_honorLap.hasValidDistance = TRUE;
    }
    else if (ultraStatus == ULTRA_STATUS_ERROR)
    {
        g_honorLap.hasValidDistance = FALSE;
    }
    else if ((g_honorLap.hasValidDistance == TRUE) &&
             ((uint32)(nowMs - g_honorLap.lastDistanceMs) > ultraStaleMs))
    {
        g_honorLap.hasValidDistance = FALSE;
    }

    g_honorLap.commandedSpeedPct =
        honor_speed_from_distance(g_honorLap.hasValidDistance, g_honorLap.lastDistanceCm);

    if ((g_linearCameraTest.haveValidVision != TRUE) ||
        (g_linearCameraTest.paused == TRUE) ||
        (g_linearCameraTest.result.status == VISION_TRACK_LOST))
    {
        g_honorLap.commandedSpeedPct = 0;
    }

    EscSetBrake(0U);
    EscSetSpeed((int)(-g_honorLap.commandedSpeedPct));

    if (time_reached(nowMs, g_honorLap.nextDisplayMs) == TRUE)
    {
        g_honorLap.nextDisplayMs = nowMs + DISPLAY_PERIOD_MS;
        honor_lap_draw_overlay();
    }

    if (ultraStatus == ULTRA_STATUS_ERROR)
    {
        StatusLed_Red();
    }
    else if ((g_linearCameraTest.haveValidVision != TRUE) ||
        (g_linearCameraTest.paused == TRUE) ||
        (g_linearCameraTest.result.status == VISION_TRACK_LOST) ||
        (g_honorLap.hasValidDistance != TRUE))
    {
        StatusLed_Yellow();
    }
    else
    {
        StatusLed_Cyan();
    }
}

static void mode_honor_lap(void)
{
    uint32 nextButtonsMs;

    App_InitRuntimeCommon();
    honor_lap_enter(Timebase_GetMs());

    nextButtonsMs = Timebase_GetMs();

    for (;;)
    {
        uint32 nowMs = Timebase_GetMs();
        boolean sw3Pressed = FALSE;
        uint8 potLevel;

        while (time_reached(nowMs, nextButtonsMs) == TRUE)
        {
            Buttons_Update();
            nextButtonsMs += BUTTONS_PERIOD_MS;
        }

        (void)Buttons_WasPressed(BUTTON_ID_SW2);
        sw3Pressed = Buttons_WasPressed(BUTTON_ID_SW3);
        potLevel = OnboardPot_ReadLevelFiltered();

        honor_lap_update(nowMs, sw3Pressed, potLevel);
    }
}

static void race_mode_enter(uint32 nowMs)
{
    (void)memset(&g_raceMode, 0, sizeof(g_raceMode));

    /* Race mode owns all actuators directly. Initialize them once, then keep
       the runtime loop strictly ordered: vision -> ultrasonic -> control. */
    g_raceMode.phase = RACE_PHASE_ESC_ARM;
    g_raceMode.nextVisionMs = nowMs;
    g_raceMode.nextControlMs = nowMs;
    g_raceMode.nextSpeedRampMs = nowMs;
    g_raceMode.nextUltraTrigMs = nowMs;
    g_raceMode.nextDisplayMs = nowMs;
    g_raceMode.armDoneMs = nowMs + ESC_ARM_TIME_MS;
    g_raceMode.lastDistanceMs = nowMs;

    EscInit(ESC_PWM_CH, ESC_DUTY_MIN, ESC_DUTY_MED, ESC_DUTY_MAX);
    Esc_StopNeutral();

    ServoInit(SERVO_PWM_CH, SERVO_DUTY_MAX, SERVO_DUTY_MIN, SERVO_DUTY_MED);
    SteerStraight();

    if (LinearCameraIsBusy() == TRUE)
    {
        LinearCameraStopStream();
    }

    LinearCameraInit(CAM_CLK_PWM_CH, CAM_SHUTTER_GPT_CH, CAM_ADC_GROUP, CAM_SHUTTER_PCR);
    LinearCameraSetFrameIntervalTicks(CAM_FRAME_INTERVAL_TICKS);
    VisionLinear_InitV2();

    SteeringLinear_Init(&g_raceMode.ctrl);
    SteeringLinear_Reset(&g_raceMode.ctrl);

    Ultrasonic_Init();

    (void)LinearCameraStartStream();
    StatusLed_Blue();
}

static void race_mode_update_vision(RaceModeState_t *st, uint32 nowMs)
{
    const LinearCameraFrame *latestFrame = NULL_PTR;

    if ((st == NULL_PTR) || (time_reached(nowMs, st->nextVisionMs) != TRUE))
    {
        return;
    }

    /* At most one vision update is executed per main-loop pass. This avoids
       catch-up bursts if an optional blocking display refresh was requested. */
    st->nextVisionMs = nowMs + CAMERA_PERIOD_MS;

    if (LinearCameraGetLatestFrame(&latestFrame) != TRUE)
    {
        return;
    }

    (void)memcpy(st->processedFrame.Values,
                 &latestFrame->Values[CAM_TRIM_LEFT_PX],
                 ((size_t)VISION_LINEAR_BUFFER_SIZE * sizeof(st->processedFrame.Values[0])));
    VisionLinear_ProcessFrame(st->processedFrame.Values, &st->result);
    st->haveValidVision = TRUE;
}

static void race_mode_update_ultrasonic(RaceModeState_t *st, uint32 nowMs)
{
    const uint32 ultraStaleMs =
        ((uint32)HONOR_ULTRA_TRIGGER_PERIOD_MS * 2U) + (uint32)ULTRA_TIMEOUT_MS;
    float distanceCm = 0.0f;
    Ultrasonic_StatusType ultraStatus;

    if (st == NULL_PTR)
    {
        return;
    }

    /* Keep ultrasonic service independent from the main control phase. */
    Ultrasonic_Task();

    if (time_reached(nowMs, st->nextUltraTrigMs) == TRUE)
    {
        st->nextUltraTrigMs = nowMs + HONOR_ULTRA_TRIGGER_PERIOD_MS;

        if (Ultrasonic_GetStatus() != ULTRA_STATUS_BUSY)
        {
            Ultrasonic_StartMeasurement();
        }
    }

    ultraStatus = Ultrasonic_GetStatus();

    if (Ultrasonic_GetDistanceCm(&distanceCm) == TRUE)
    {
        st->lastDistanceCm = distanceCm;
        st->lastDistanceMs = nowMs;
        st->haveValidDistance = TRUE;
        st->ultraError = FALSE;
    }
    else if (ultraStatus == ULTRA_STATUS_ERROR)
    {
        st->haveValidDistance = FALSE;
        st->ultraError = TRUE;
    }
    else if ((st->haveValidDistance == TRUE) &&
             ((uint32)(nowMs - st->lastDistanceMs) > ultraStaleMs))
    {
        st->haveValidDistance = FALSE;
        st->ultraError = FALSE;
    }
    else if (ultraStatus != ULTRA_STATUS_ERROR)
    {
        st->ultraError = FALSE;
    }
}

static void race_mode_update_control(RaceModeState_t *st, uint32 nowMs, boolean stopPressed)
{
    uint8 controllerBaseSpeed;

    if ((st == NULL_PTR) || (time_reached(nowMs, st->nextControlMs) != TRUE))
    {
        return;
    }

    st->nextControlMs = nowMs + STEER_UPDATE_MS;

    if (stopPressed == TRUE)
    {
        st->phase = RACE_PHASE_STOPPED;
    }

    if ((st->haveValidVision == TRUE) && (st->result.status != VISION_TRACK_LOST))
    {
        st->lostSinceMs = 0U;
    }
    else if (st->lostSinceMs == 0U)
    {
        st->lostSinceMs = nowMs;
    }
    else if ((st->phase == RACE_PHASE_SPEED_RUN) || (st->phase == RACE_PHASE_HONOR_RUN))
    {
        if ((uint32)(nowMs - st->lostSinceMs) >= LINE_LOST_COAST_MS)
        {
            st->phase = RACE_PHASE_FAULT;
        }
    }

    if ((st->phase == RACE_PHASE_ESC_ARM) && (time_reached(nowMs, st->armDoneMs) == TRUE))
    {
        st->phase = RACE_PHASE_SPEED_RUN;
        st->finishStreak = 0U;
        st->lostSinceMs =
            ((st->haveValidVision == TRUE) && (st->result.status != VISION_TRACK_LOST)) ? 0U : nowMs;
    }

    if ((st->phase == RACE_PHASE_SPEED_RUN) &&
        (st->haveValidVision == TRUE) &&
        (st->result.feature == VISION_FEATURE_FINISH_LINE) &&
        (st->result.confidence >= (uint8)RACE_FINISH_MIN_CONFIDENCE))
    {
        if (st->finishStreak < 255U)
        {
            st->finishStreak++;
        }

        st->phase = RACE_PHASE_HONOR_RUN;
    }
    else if (st->phase == RACE_PHASE_SPEED_RUN)
    {
        st->finishStreak = 0U;
    }

    /* Steering is updated from the latest accepted vision result only inside
       the active race phases. Other phases stay neutral and centered. */
    if ((st->phase == RACE_PHASE_SPEED_RUN) || (st->phase == RACE_PHASE_HONOR_RUN))
    {
        if ((st->haveValidVision == TRUE) && (st->result.status != VISION_TRACK_LOST))
        {
            float dt = ((float)STEER_UPDATE_MS) * 0.001f;
            SteeringOutput_t out;

            controllerBaseSpeed = (st->currentSpeedPct > 0) ? (uint8)st->currentSpeedPct : FULL_AUTO_SPEED_PCT;
            out = SteeringLinear_UpdateV2(&st->ctrl, &st->result, dt, controllerBaseSpeed);
            st->steerRaw = (sint16)out.steer_cmd;

            if (((st->steerRaw < 0) ? (sint16)(-st->steerRaw) : st->steerRaw) <= 2)
            {
                st->steerRaw = 0;
            }

            st->steerFilt = SteeringSmooth_IirS16(st->steerFilt, st->steerRaw, 0.45f);

            {
                const sint16 steerRateMax = 8;
                sint16 delta = (sint16)(st->steerFilt - st->steerOut);

                delta = SteeringSmooth_ClampS16(delta, (sint16)(-steerRateMax), (sint16)(+steerRateMax));
                st->steerOut = (sint16)(st->steerOut + delta);
            }

            st->steerOut = SteeringSmooth_ClampS16(st->steerOut,
                                                   (sint16)(-STEER_CMD_CLAMP),
                                                   (sint16)(+STEER_CMD_CLAMP));
            Steer((int)st->steerOut);
        }
        else
        {
            st->steerRaw = 0;
            st->steerFilt = 0;
            st->steerOut = 0;
            SteerStraight();
        }
    }
    else
    {
        st->steerRaw = 0;
        st->steerFilt = 0;
        st->steerOut = 0;
        SteerStraight();
    }

    if (st->phase == RACE_PHASE_SPEED_RUN)
    {
        sint32 steerSlowPct = ((sint32)((st->steerOut < 0) ? (sint16)(-st->steerOut) : st->steerOut) * (sint32)SPEED_SLOW_PER_STEER) /
                              (sint32)STEER_CMD_CLAMP;

        st->targetSpeedPct = (sint32)FULL_AUTO_SPEED_PCT - steerSlowPct;

        if ((st->haveValidVision == TRUE) && (st->result.status != VISION_TRACK_BOTH))
        {
            if (st->targetSpeedPct > (sint32)FULLMODE_SLOW_SPEED_PCT)
            {
                st->targetSpeedPct = (sint32)FULLMODE_SLOW_SPEED_PCT;
            }
        }

        if ((st->haveValidVision != TRUE) || (st->result.status == VISION_TRACK_LOST))
        {
            st->targetSpeedPct = (sint32)SPEED_LOST_LINE;
        }
    }
    else if (st->phase == RACE_PHASE_HONOR_RUN)
    {
        st->targetSpeedPct = honor_speed_from_distance(st->haveValidDistance, st->lastDistanceCm);

        if ((st->haveValidVision != TRUE) || (st->result.status == VISION_TRACK_LOST))
        {
            st->targetSpeedPct = 0;
        }
    }
    else
    {
        st->targetSpeedPct = 0;
    }

    if (st->targetSpeedPct < 0)
    {
        st->targetSpeedPct = 0;
    }
    if (st->targetSpeedPct > 100)
    {
        st->targetSpeedPct = 100;
    }

    /* Speed commands are rate-limited to keep ESC updates deterministic. */
    if (time_reached(nowMs, st->nextSpeedRampMs) == TRUE)
    {
        st->nextSpeedRampMs = nowMs + FULL_AUTO_RAMP_PERIOD_MS;

        if (st->currentSpeedPct < st->targetSpeedPct)
        {
            st->currentSpeedPct += (sint32)FULL_AUTO_RAMP_STEP_PCT;
            if (st->currentSpeedPct > st->targetSpeedPct)
            {
                st->currentSpeedPct = st->targetSpeedPct;
            }
        }
        else if (st->currentSpeedPct > st->targetSpeedPct)
        {
            st->currentSpeedPct -= (sint32)FULL_AUTO_RAMP_STEP_PCT;
            if (st->currentSpeedPct < st->targetSpeedPct)
            {
                st->currentSpeedPct = st->targetSpeedPct;
            }
        }

        if (st->currentSpeedPct <= 0)
        {
            st->currentSpeedPct = 0;
            Esc_StopNeutral();
        }
        else
        {
            EscSetBrake(0U);
            EscSetSpeed((int)(-st->currentSpeedPct));
        }
    }

    if (st->phase == RACE_PHASE_SPEED_RUN)
    {
        StatusLed_Green();
    }
    else if (st->phase == RACE_PHASE_HONOR_RUN)
    {
        StatusLed_Cyan();
    }
    else if ((st->phase == RACE_PHASE_FAULT) || (st->ultraError == TRUE))
    {
        StatusLed_Red();
    }
    else
    {
        StatusLed_Blue();
    }
}

static void race_mode_ensure_display_ready(RaceModeState_t *st)
{
    if ((st == NULL_PTR) || (st->displayInitialized == TRUE))
    {
        return;
    }

    /* The OLED path is blocking, so race mode keeps it completely out of the
       hot path until the dedicated debug switch explicitly requests it. */
    display_power_stabilize_delay();
    DisplayInit(0U, STD_ON);
    DisplayClear();
    DisplayRefresh();
    st->displayInitialized = TRUE;
}

static void race_mode_draw(const RaceModeState_t *st)
{
    const char *phaseText = "UNKNOWN";

    if (st == NULL_PTR)
    {
        return;
    }

    switch (st->phase)
    {
        case RACE_PHASE_ESC_ARM:
            phaseText = "ARM";
            break;

        case RACE_PHASE_SPEED_RUN:
            phaseText = "RUN";
            break;

        case RACE_PHASE_HONOR_RUN:
            phaseText = "HONOR";
            break;

        case RACE_PHASE_STOPPED:
            phaseText = "STOP";
            break;

        case RACE_PHASE_FAULT:
            phaseText = "FAULT";
            break;

        default:
            break;
    }

    DisplayTextPadded(0U, "RACE MODE");
    DisplayTextPadded(1U, "State:");
    DisplayText(1U, phaseText, (uint16)strlen(phaseText), 7U);
    DisplayTextPadded(2U, "Spd:    D:");
    DisplayValue(2U, (int)st->currentSpeedPct, 3U, 4U);

    if (st->haveValidDistance == TRUE)
    {
        DisplayValue(2U, (int)(st->lastDistanceCm + 0.5f), 3U, 11U);
    }
    else
    {
        DisplayText(2U, "---", 3U, 11U);
    }

    DisplayTextPadded(3U, "V:      F:");
    if ((st->haveValidVision == TRUE) && (st->result.status != VISION_TRACK_LOST))
    {
        DisplayValue(3U, (int)st->result.confidence, 3U, 2U);
    }
    else
    {
        DisplayText(3U, "LST", 3U, 2U);
    }
    DisplayValue(3U, (int)st->finishStreak, 2U, 12U);
    DisplayRefresh();
}

static void mode_race_mode(void)
{
    uint32 nextButtonsMs;

    App_InitRuntimeCore();
    race_mode_enter(Timebase_GetMs());
    nextButtonsMs = Timebase_GetMs();

    for (;;)
    {
        uint32 nowMs = Timebase_GetMs();
        boolean stopPressed;
        boolean displaySwitchOn;

        while (time_reached(nowMs, nextButtonsMs) == TRUE)
        {
            Buttons_Update();
            nextButtonsMs += BUTTONS_PERIOD_MS;
        }

        /* SW2 events are drained even though production race mode autostarts
           after ESC arm. This keeps button latches from accumulating. */
        (void)Buttons_WasPressed(BUTTON_ID_SW2);
        stopPressed = Buttons_WasPressed(BUTTON_ID_SW3);
        displaySwitchOn = Buttons_IsOn(BUTTON_ID_SWPCB);

        race_mode_update_vision(&g_raceMode, nowMs);
        race_mode_update_ultrasonic(&g_raceMode, nowMs);
        race_mode_update_control(&g_raceMode, nowMs, stopPressed);

        if ((displaySwitchOn == TRUE) &&
            (g_raceMode.displayInitialized != TRUE) &&
            (g_raceMode.phase == RACE_PHASE_ESC_ARM))
        {
            race_mode_ensure_display_ready(&g_raceMode);
            nowMs = Timebase_GetMs();
            g_raceMode.nextDisplayMs = nowMs;
        }

        if ((displaySwitchOn == TRUE) &&
            (g_raceMode.displayInitialized == TRUE) &&
            ((g_raceMode.displayWasOn != TRUE) || (time_reached(nowMs, g_raceMode.nextDisplayMs) == TRUE)))
        {
            g_raceMode.nextDisplayMs = nowMs + RACE_DISPLAY_PERIOD_MS;
            race_mode_draw(&g_raceMode);
        }

        g_raceMode.displayWasOn = displaySwitchOn;
    }
}

void App_RunSelectedMode(void)
{
    switch (App_GetSelectedBuildMode())
    {
        case APP_BUILD_MODE_LINEAR_CAMERA_TEST:
            mode_linear_camera_test();
            break;

        case APP_BUILD_MODE_NXP_CUP:
            mode_nxp_cup();
            break;

        case APP_BUILD_MODE_RACE_MODE:
            mode_race_mode();
            break;

        case APP_BUILD_MODE_FINAL_DUMMY:
            mode_final_dummy();
            break;

        case APP_BUILD_MODE_HONOR_LAP:
            mode_honor_lap();
            break;

        case APP_BUILD_MODE_NXP_CUP_TESTS:
            mode_nxp_cup_tests();
            break;

        default:
            while (1)
            {
            }
    }
}
