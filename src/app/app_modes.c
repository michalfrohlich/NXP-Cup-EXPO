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
    APP_BUILD_MODE_RECEIVER_TEST,
    APP_BUILD_MODE_SERVO_TEST,
    APP_BUILD_MODE_ESC_TEST,
    APP_BUILD_MODE_FINAL_DUMMY,
    APP_BUILD_MODE_HONOR_LAP,
    APP_BUILD_MODE_NXP_CUP_TESTS
} AppBuildMode_t;

typedef enum
{
    RUNTIME_TEST_LINEAR_CAMERA = 0,
    RUNTIME_TEST_RECEIVER,
    RUNTIME_TEST_SERVO,
    RUNTIME_TEST_ESC,
    RUNTIME_TEST_CAMSERVO,
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

typedef struct
{
    uint32 nextDisplayMs;
    uint8 potLevel;
    sint16 steerCmd;
    uint32 pwmCmd;
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
    uint32 tickCount;
    boolean haveValidVision;
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
    VisionDebug_State_t vdbg;
    SteeringLinearState_t ctrl;
    LinearCameraFrame processedFrame;
    VisionOutput_t result;
    VisionLinear_DebugOut_t dbg;
    uint16 filteredBuf[VISION_LINEAR_BUFFER_SIZE];
    sint16 gradientBuf[VISION_LINEAR_BUFFER_SIZE];
    boolean haveValidVision;
    sint16 steerRaw;
    sint16 steerFilt;
    sint16 steerOut;
    uint32 nextTickMs;
    uint32 nextSteerMs;
    uint32 tickCount;
} CamServoState_t;

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

static ReceiverTestState_t g_receiverTest;
static ServoTestState_t g_servoTest;
static EscRunState_t g_escTest;
static LinearCameraTestState_t g_linearCameraTest;
static FinalDummyState_t g_finalDummy;
static TestsMenuState_t g_testsMenu;
static HonorLapState_t g_honorLap;

static const char g_testsMenuItems[RUNTIME_TEST_COUNT][17] =
{
    "Camera Debug   ",
    "Receiver       ",
    "Servo          ",
    "ESC            ",
    "Cam Servo      "
};

static boolean time_reached(uint32 nowMs, uint32 dueMs);
static uint16 VisionDebug_WhiteMaxFromPot(uint8 potLevel);
static Sw2Action_t Sw2Tracker_Update(Sw2Tracker_t *st, uint32 nowMs);
static void StatusLed_Blue(void);
static void StatusLed_Green(void);
static void StatusLed_Off(void);
static void Esc_StopNeutral(void);
static void App_InitRuntimeCommon(void);
static AppBuildMode_t App_GetSelectedBuildMode(void);
static void DisplayTextPadded(uint16 displayLine, const char *text);
static void receiver_test_enter(uint32 nowMs);
static void receiver_test_update(uint32 nowMs);
static void receiver_test_exit(void);
static void servo_test_enter(uint32 nowMs);
static void servo_test_update(uint32 nowMs, uint8 potLevel);
static void servo_test_exit(void);
static void esc_test_enter(uint32 nowMs);
static void esc_test_update(uint32 nowMs, boolean sw2Pressed, boolean sw3Pressed, uint8 potLevel);
static void esc_test_exit(void);
static void linear_camera_test_enter_common(uint32 nowMs, boolean servoEnabled);
static void linear_camera_test_enter(uint32 nowMs);
static void linear_camera_test_update(uint32 nowMs, boolean modeNextPressed, uint8 potLevel);
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
static sint16 abs_s16(sint16 x);
static sint16 clamp_s16(sint16 x, sint16 lo, sint16 hi);
static sint16 iir_s16(sint16 y_prev, sint16 x, float alpha);
static void camservo_enter(CamServoState_t *st, uint32 nowMs);
static void camservo_update(CamServoState_t *st, uint32 nowMs, boolean sw2);
static void camservo_exit(void);
static void final_dummy_enter(uint32 nowMs);
static void final_dummy_update(uint32 nowMs, boolean sw2Pressed, boolean sw3Pressed, uint8 potLevel);
static uint8 tests_menu_map_pot_to_index(uint8 pot, uint8 nItems);
static void tests_menu_select_index(TestsMenuState_t *st, uint8 pot);
static void tests_menu_draw(const TestsMenuState_t *st);
static void mode_nxp_cup_tests(void);
static void mode_single_runtime_test(RuntimeTestId_t testId);
static void mode_final_dummy(void);
static sint32 honor_speed_from_distance(boolean hasValidDistance, float distanceCm);
static void honor_lap_draw_overlay(void);
static void honor_lap_enter(uint32 nowMs);
static void honor_lap_update(uint32 nowMs, boolean modeNextPressed, uint8 potLevel);
static void mode_honor_lap(void);

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

static void StatusLed_Off(void)
{
    RgbLed_ChangeColor((RgbLed_Color){ .r = false, .g = false, .b = false });
}

static void Esc_StopNeutral(void)
{
    EscSetBrake(0U);
    EscSetSpeed(0);
}

static void App_InitRuntimeCommon(void)
{
    Board_InitDrivers();
    Timebase_Init();
    OnboardPot_Init();

    display_power_stabilize_delay();
    DisplayInit(0U, STD_ON);
    DisplayClear();
    DisplayRefresh();
    StatusLed_Off();
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
#if APP_TEST_NXP_CUP_TESTS
    return APP_BUILD_MODE_NXP_CUP_TESTS;
#elif APP_TEST_HONOR_LAP
    return APP_BUILD_MODE_HONOR_LAP;
#elif APP_TEST_FINAL_DUMMY
    return APP_BUILD_MODE_FINAL_DUMMY;
#elif APP_TEST_LINEAR_CAMERA_TEST
    return APP_BUILD_MODE_LINEAR_CAMERA_TEST;
#elif APP_TEST_RECEIVER_TEST
    return APP_BUILD_MODE_RECEIVER_TEST;
#elif APP_TEST_SERVO_TEST
    return APP_BUILD_MODE_SERVO_TEST;
#elif APP_TEST_ESC_TEST
    return APP_BUILD_MODE_ESC_TEST;
#else
    return APP_BUILD_MODE_LINEAR_CAMERA_TEST;
#endif
}

static void receiver_test_enter(uint32 nowMs)
{
    (void)memset(&g_receiverTest, 0, sizeof(g_receiverTest));

    ReceiverInit(0U, 0U, 11700U, 17700U, 23700U, 26000U);

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

static void servo_test_enter(uint32 nowMs)
{
    (void)memset(&g_servoTest, 0, sizeof(g_servoTest));

    ServoInit(SERVO_PWM_CH, SERVO_DUTY_MAX, SERVO_DUTY_MIN, SERVO_DUTY_MED);
    SteerStraight();

    g_servoTest.nextDisplayMs = nowMs;

    DisplayTextPadded(0U, "SERVO MANUAL");
    DisplayTextPadded(1U, "POT:");
    DisplayTextPadded(2U, "STEER:");
    DisplayTextPadded(3U, "PWM:");
    DisplayRefresh();
}

static void servo_test_update(uint32 nowMs, uint8 potLevel)
{
    sint32 steerCmd;

    g_servoTest.potLevel = potLevel;

    if (potLevel <= (uint8)POT_CENTER_RAW)
    {
        uint16 denom = (uint16)(POT_CENTER_RAW - POT_LEFT_RAW);

        if (denom == 0U)
        {
            denom = 1U;
        }

        steerCmd = -((sint32)((uint8)POT_CENTER_RAW - potLevel) * 100) / (sint32)denom;
    }
    else
    {
        uint16 denom = (uint16)(POT_RIGHT_RAW - POT_CENTER_RAW);

        if (denom == 0U)
        {
            denom = 1U;
        }

        steerCmd = ((sint32)(potLevel - (uint8)POT_CENTER_RAW) * 100) / (sint32)denom;
    }

    if (steerCmd > 100)
    {
        steerCmd = 100;
    }
    if (steerCmd < -100)
    {
        steerCmd = -100;
    }

    g_servoTest.steerCmd = (sint16)steerCmd;
    Steer((int)g_servoTest.steerCmd);

    if (g_servoTest.steerCmd >= 0)
    {
        g_servoTest.pwmCmd = (uint32)SERVO_DUTY_MED +
                             (((uint32)g_servoTest.steerCmd *
                               ((uint32)SERVO_DUTY_MAX - (uint32)SERVO_DUTY_MED)) / 100U);
    }
    else
    {
        g_servoTest.pwmCmd = (uint32)SERVO_DUTY_MED -
                             (((uint32)(-g_servoTest.steerCmd) *
                               ((uint32)SERVO_DUTY_MED - (uint32)SERVO_DUTY_MIN)) / 100U);
    }

    if (time_reached(nowMs, g_servoTest.nextDisplayMs) == TRUE)
    {
        g_servoTest.nextDisplayMs = nowMs + 50U;

        DisplayTextPadded(1U, "POT:");
        DisplayValue(1U, (int)g_servoTest.potLevel, 3U, 5U);

        DisplayTextPadded(2U, "STEER:");
        DisplayValue(2U, (int)g_servoTest.steerCmd, 4U, 7U);

        DisplayTextPadded(3U, "PWM:");
        DisplayValue(3U, (int)g_servoTest.pwmCmd, 4U, 4U);

        DisplayRefresh();
    }
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

    if (servoEnabled == TRUE)
    {
        ServoInit(SERVO_PWM_CH, SERVO_DUTY_MAX, SERVO_DUTY_MIN, SERVO_DUTY_MED);
        SteerStraight();
        SteeringLinear_Init(&g_linearCameraTest.ctrl);
        SteeringLinear_Reset(&g_linearCameraTest.ctrl);
    }

    (void)LinearCameraStartStream();
}

static void linear_camera_test_enter(uint32 nowMs)
{
    linear_camera_test_enter_common(nowMs, FALSE);
}

static void linear_camera_test_update(uint32 nowMs, boolean modeNextPressed, uint8 potLevel)
{
    const uint32 loopPeriodMs = 5U;
    const uint32 displayTicks = ((uint32)DISPLAY_PERIOD_MS / loopPeriodMs);
    Sw2Action_t sw2Action;
    boolean sw2ClickHandled = FALSE;

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
    }

    VisionDebug_OnTick(&g_linearCameraTest.vdbg,
                       (boolean)((sw2Action == SW2_ACTION_CLICK) &&
                                 (sw2ClickHandled != TRUE) &&
                                 (g_linearCameraTest.paused != TRUE)),
                       modeNextPressed);

    RgbLed_ChangeColor((RgbLed_Color){ .r = true, .g = false, .b = false });

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

            RgbLed_ChangeColor((RgbLed_Color){ .r = false, .g = false, .b = true });

            (void)memcpy(g_linearCameraTest.processedFrame.Values,
                         &latestFrame->Values[CAM_TRIM_LEFT_PX],
                         ((size_t)VISION_LINEAR_BUFFER_SIZE *
                          sizeof(g_linearCameraTest.processedFrame.Values[0])));
            VisionLinear_ProcessFrameEx(g_linearCameraTest.processedFrame.Values,
                                        &g_linearCameraTest.result,
                                        &g_linearCameraTest.dbg);

            g_linearCameraTest.haveValidVision = TRUE;
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

            if (abs_s16(g_linearCameraTest.steerRaw) <= 2)
            {
                g_linearCameraTest.steerRaw = 0;
            }

            g_linearCameraTest.steerFilt = iir_s16(g_linearCameraTest.steerFilt,
                                                   g_linearCameraTest.steerRaw,
                                                   0.45f);

            {
                const sint16 steerRateMax = 8;
                sint16 delta = (sint16)(g_linearCameraTest.steerFilt - g_linearCameraTest.steerOut);

                delta = clamp_s16(delta, (sint16)(-steerRateMax), (sint16)(+steerRateMax));
                g_linearCameraTest.steerOut = (sint16)(g_linearCameraTest.steerOut + delta);
            }

            g_linearCameraTest.steerOut = clamp_s16(g_linearCameraTest.steerOut,
                                                    (sint16)(-STEER_CMD_CLAMP),
                                                    (sint16)(+STEER_CMD_CLAMP));

            Steer((int)g_linearCameraTest.steerOut);
        }
    }

    if ((displayTicks != 0U) &&
        ((g_linearCameraTest.tickCount % displayTicks) == 0U) &&
        (g_linearCameraTest.haveValidVision == TRUE))
    {
        const uint16 *pFiltered =
            (g_linearCameraTest.dbg.filteredOut != NULL_PTR) ? g_linearCameraTest.filteredBuf : NULL_PTR;
        const sint16 *pGradient =
            (g_linearCameraTest.dbg.gradientOut != NULL_PTR) ? g_linearCameraTest.gradientBuf : NULL_PTR;
        const VisionLinear_DebugOut_t *pDbg =
            (g_linearCameraTest.dbg.mask != (uint32)VLIN_DBG_NONE) ? &g_linearCameraTest.dbg : NULL_PTR;

        VisionDebug_Draw(&g_linearCameraTest.vdbg,
                         g_linearCameraTest.processedFrame.Values,
                         pFiltered,
                         pGradient,
                         &g_linearCameraTest.result,
                         pDbg);
    }

    StatusLed_Off();
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

    StatusLed_Off();
}

static void runtime_test_enter(RuntimeTestId_t testId, uint32 nowMs)
{
    switch (testId)
    {
        case RUNTIME_TEST_RECEIVER:
            receiver_test_enter(nowMs);
            break;

        case RUNTIME_TEST_SERVO:
            servo_test_enter(nowMs);
            break;

        case RUNTIME_TEST_ESC:
            esc_test_enter(nowMs);
            break;

        case RUNTIME_TEST_CAMSERVO:
            linear_camera_test_enter_common(nowMs, TRUE);
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
        case RUNTIME_TEST_RECEIVER:
            receiver_test_update(nowMs);
            break;

        case RUNTIME_TEST_SERVO:
            servo_test_update(nowMs, potLevel);
            break;

        case RUNTIME_TEST_ESC:
            esc_test_update(nowMs, sw2Pressed, modeNextPressed, potLevel);
            break;

        case RUNTIME_TEST_CAMSERVO:
            linear_camera_test_update(nowMs, FALSE, potLevel);
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
        case RUNTIME_TEST_RECEIVER:
            receiver_test_exit();
            break;

        case RUNTIME_TEST_SERVO:
            servo_test_exit();
            break;

        case RUNTIME_TEST_ESC:
            esc_test_exit();
            break;

        case RUNTIME_TEST_CAMSERVO:
            linear_camera_test_exit();
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

static sint16 abs_s16(sint16 x)
{
    return (x < 0) ? (sint16)(-x) : x;
}

static sint16 clamp_s16(sint16 x, sint16 lo, sint16 hi)
{
    if (x < lo)
    {
        return lo;
    }
    if (x > hi)
    {
        return hi;
    }
    return x;
}

static sint16 iir_s16(sint16 y_prev, sint16 x, float alpha)
{
    float y = (float)y_prev + alpha * ((float)x - (float)y_prev);

    if (y > 32767.0f)
    {
        y = 32767.0f;
    }
    if (y < -32768.0f)
    {
        y = -32768.0f;
    }

    return (sint16)(y);
}

static void camservo_enter(CamServoState_t *st, uint32 nowMs)
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

    st->steerRaw = 0;
    st->steerFilt = 0;
    st->steerOut = 0;
    st->haveValidVision = FALSE;
    st->nextTickMs = nowMs;
    st->nextSteerMs = nowMs + STEER_UPDATE_MS;
    st->tickCount = 0u;

    (void)LinearCameraStartStream();
}

static void camservo_update(CamServoState_t *st, uint32 nowMs, boolean sw2)
{
    const uint32 loopMs = (uint32)V2_LOOP_PERIOD_MS;
    const uint32 displayTicks = ((uint32)DISPLAY_PERIOD_MS / loopMs);
    boolean doDisplay;
    const LinearCameraFrame *latestFrame = NULL_PTR;

    if ((uint32)(nowMs - st->nextTickMs) < loopMs)
    {
        return;
    }

    st->nextTickMs += loopMs;
    st->tickCount++;

    doDisplay = ((displayTicks != 0U) && ((st->tickCount % displayTicks) == 0U));

    VisionDebug_OnTick(&st->vdbg, sw2, FALSE);

    if (LinearCameraGetLatestFrame(&latestFrame) == TRUE)
    {
        st->dbg.mask = (uint32)VLIN_DBG_NONE;
        st->dbg.filteredOut = NULL_PTR;
        st->dbg.gradientOut = NULL_PTR;

        if ((doDisplay == TRUE) && (VisionDebug_WantsVisionDebugData(&st->vdbg) == TRUE))
        {
            VisionDebug_PrepareVisionDbg(&st->vdbg, &st->dbg, st->filteredBuf, st->gradientBuf);
        }

        (void)memcpy(st->processedFrame.Values,
                     &latestFrame->Values[CAM_TRIM_LEFT_PX],
                     ((size_t)VISION_LINEAR_BUFFER_SIZE * sizeof(st->processedFrame.Values[0])));
        VisionLinear_ProcessFrameEx(st->processedFrame.Values, &st->result, &st->dbg);
        st->haveValidVision = TRUE;
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
            const uint8 fakeSpeed = 20U;
            SteeringOutput_t out = SteeringLinear_UpdateV2(&st->ctrl, &st->result, dt, fakeSpeed);

            st->steerRaw = (sint16)out.steer_cmd;
        }

        if (abs_s16(st->steerRaw) <= 2)
        {
            st->steerRaw = 0;
        }

        st->steerFilt = iir_s16(st->steerFilt, st->steerRaw, 0.45f);

        {
            const sint16 steerRateMax = 8;
            sint16 delta = (sint16)(st->steerFilt - st->steerOut);

            delta = clamp_s16(delta, (sint16)(-steerRateMax), (sint16)(+steerRateMax));
            st->steerOut = (sint16)(st->steerOut + delta);
        }

        st->steerOut = clamp_s16(st->steerOut,
                                 (sint16)(-STEER_CMD_CLAMP),
                                 (sint16)(+STEER_CMD_CLAMP));

        Steer((int)st->steerOut);
    }

    if ((doDisplay == TRUE) && (st->haveValidVision == TRUE))
    {
        const uint16 *pFiltered = (st->dbg.filteredOut != NULL_PTR) ? st->filteredBuf : NULL_PTR;
        const sint16 *pGradient = (st->dbg.gradientOut != NULL_PTR) ? st->gradientBuf : NULL_PTR;
        const VisionLinear_DebugOut_t *pDbg =
            (st->dbg.mask != (uint32)VLIN_DBG_NONE) ? &st->dbg : NULL_PTR;

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
            StatusLed_Green();

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

static void mode_single_runtime_test(RuntimeTestId_t testId)
{
    uint32 nextButtonsMs;

    App_InitRuntimeCommon();
    runtime_test_enter(testId, Timebase_GetMs());

    nextButtonsMs = Timebase_GetMs();

    for (;;)
    {
        uint32 nowMs = Timebase_GetMs();
        boolean sw2Pressed = FALSE;
        boolean sw3Pressed = FALSE;
        uint8 potLevel;

        while (time_reached(nowMs, nextButtonsMs) == TRUE)
        {
            Buttons_Update();
            nextButtonsMs += BUTTONS_PERIOD_MS;
        }

        potLevel = OnboardPot_ReadLevelFiltered();

        if ((testId == RUNTIME_TEST_LINEAR_CAMERA) || (testId == RUNTIME_TEST_ESC))
        {
            sw3Pressed = Buttons_WasPressed(BUTTON_ID_SW3);
        }
        else
        {
            (void)Buttons_WasPressed(BUTTON_ID_SW3);
        }

        if (testId == RUNTIME_TEST_ESC)
        {
            sw2Pressed = Buttons_WasPressed(BUTTON_ID_SW2);
        }
        else
        {
            (void)Buttons_WasPressed(BUTTON_ID_SW2);
        }

        runtime_test_update(testId, nowMs, sw2Pressed, sw3Pressed, potLevel);
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
    g_linearCameraTest.ctrl.steerScale = HONOR_STEER_SCALE;
    g_linearCameraTest.steeringBaseSpeed = HONOR_FAKE_SPEED;

    Ultrasonic_Init();
    (void)Ultrasonic_GetDistanceCm(&g_honorLap.lastDistanceCm);
    Ultrasonic_StartMeasurement();

    g_honorLap.nextUltraTrigMs = nowMs + HONOR_ULTRA_TRIGGER_PERIOD_MS;
    g_honorLap.nextDisplayMs = nowMs;
    g_honorLap.commandedSpeedPct = 0;

    StatusLed_Green();
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

    StatusLed_Green();
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

void App_RunSelectedMode(void)
{
    switch (App_GetSelectedBuildMode())
    {
        case APP_BUILD_MODE_RECEIVER_TEST:
            mode_single_runtime_test(RUNTIME_TEST_RECEIVER);
            break;

        case APP_BUILD_MODE_SERVO_TEST:
            mode_single_runtime_test(RUNTIME_TEST_SERVO);
            break;

        case APP_BUILD_MODE_ESC_TEST:
            mode_single_runtime_test(RUNTIME_TEST_ESC);
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

        case APP_BUILD_MODE_LINEAR_CAMERA_TEST:
        default:
            mode_single_runtime_test(RUNTIME_TEST_LINEAR_CAMERA);
            break;
    }
}
