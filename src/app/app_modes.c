#include "app_modes.h"
#include "main_types.h"
#include "car_config.h"
#include "board_init.h"
#include <string.h>

/* Hardware */
#include "timebase.h"
#include "buttons.h"
#include "onboard_pot.h"
#include "receiver.h"
#include "servo.h"
#include "esc.h"
#include "linear_camera.h"
#include "display.h"
#include "ultrasonic.h"

/* Vision V2 + debug */
#include "../services/vision_linear_v2.h"
#include "rgb_led.h"
#include "../services/steering_control_linear.h"

/* UI (do not fight it now; just compile with project) */
#include "user_interface.h"
#include "vision_debug.h"

/* =========================================================
   Helpers
========================================================= */
#define VDBG_WHITE_MAX_FULL_SCALE     (4095U)
#define VDBG_WHITE_MAX_MIN_ZOOM       (400U)

/* Your measured true neutral */
#define ESC_TRUE_NEUTRAL_CMD          (-6)

/* Extra settle after SW3 so ESC finishes the last beep */
#define NXP_ESC_EXTRA_SETTLE_MS       (1500u)

static void busy_delay(volatile uint32 ticks)
{
    while (ticks != 0U)
    {
        ticks--;
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

static int esc_apply_neutral_offset(int logicalCmd)
{
    int physicalCmd = logicalCmd + ESC_TRUE_NEUTRAL_CMD;

    if (physicalCmd > 100)  { physicalCmd = 100; }
    if (physicalCmd < -100) { physicalCmd = -100; }

    return physicalCmd;
}

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

static Sw2Action_t Sw2Tracker_Update(Sw2Tracker_t *st, uint32 nowMs)
{
    boolean pressedNow;

    if (st == NULL)
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

/* =========================================================
   Basic test modes
========================================================= */
static void mode_receiver_test(void)
{
    int receiverChannels[8];
    uint8 displayValueOffset;

    Board_InitDrivers();
    DisplayInit(0U, STD_ON);

    ReceiverInit(0U, 0U, 11700U, 17700U, 23700U, 26000U);

    DisplayText(0U, "Ch0:    Ch1:", 12U, 0U);
    DisplayText(1U, "Ch2:    Ch3:", 12U, 0U);
    DisplayText(2U, "Ch4:    Ch5:", 12U, 0U);
    DisplayText(3U, "Ch6:    Ch7:", 12U, 0U);

    for (;;)
    {
        uint8 i;
        for (i = 0U; i < 8U; i++)
        {
            receiverChannels[i] = GetReceiverChannel(i);
            displayValueOffset = (uint8)(4U + 8U * (i % 2U));
            DisplayValue(i / 2U, receiverChannels[i], 4U, displayValueOffset);
        }
        DisplayRefresh();
    }
}

static void mode_servo_test(void)
{
    uint32 nextUpdateMs;
    sint16 steerRaw = 0;
    sint16 steerFilt = 0;
    sint16 steerOut = 0;

    Board_InitDrivers();
    Timebase_Init();
    OnboardPot_Init();
    DisplayInit(0U, STD_ON);
    ServoInit(SERVO_PWM_CH, SERVO_DUTY_MAX, SERVO_DUTY_MIN, SERVO_DUTY_MED);
    SteerStraight();
    nextUpdateMs = Timebase_GetMs();

    for (;;)
    {
        uint32 nowMs = Timebase_GetMs();
        uint8 pot;
        float filt;

        if ((uint32)(nowMs - nextUpdateMs) < SERVO_TEST_UPDATE_MS)
        {
            continue;
        }
        nextUpdateMs += SERVO_TEST_UPDATE_MS;

        Buttons_Update();
        pot = OnboardPot_ReadLevelFiltered();
        steerRaw = (sint16)((((sint32)pot * 200) / 255) - 100);

        if (steerRaw > (sint16)SERVO_TEST_CMD_CLAMP)
        {
            steerRaw = (sint16)SERVO_TEST_CMD_CLAMP;
        }
        if (steerRaw < (sint16)(-SERVO_TEST_CMD_CLAMP))
        {
            steerRaw = (sint16)(-SERVO_TEST_CMD_CLAMP);
        }

        if ((steerRaw <= (sint16)SERVO_TEST_DEADBAND) &&
            (steerRaw >= (sint16)(-SERVO_TEST_DEADBAND)))
        {
            steerRaw = 0;
        }

        if (Buttons_WasPressed(BUTTON_ID_SW2))
        {
            steerRaw = 0;
            steerFilt = 0;
            steerOut = 0;
            SteerStraight();
        }
        else
        {
            sint16 delta;
            sint16 rateMax;

            filt = (float)steerFilt +
                   ((float)SERVO_TEST_LPF_ALPHA * ((float)steerRaw - (float)steerFilt));

            if (filt > 32767.0f)  { filt = 32767.0f; }
            if (filt < -32768.0f) { filt = -32768.0f; }
            steerFilt = (sint16)filt;

            delta = (sint16)(steerFilt - steerOut);
            sint16 absFilt = (steerFilt < 0) ? (sint16)(-steerFilt) : steerFilt;

            rateMax = (sint16)SERVO_TEST_RATE_MAX;
            rateMax = (sint16)(rateMax + (absFilt / 6));
            if (rateMax > (sint16)SERVO_TEST_CMD_CLAMP)
            {
                rateMax = (sint16)SERVO_TEST_CMD_CLAMP;
            }

            if (delta > rateMax)
            {
                delta = rateMax;
            }
            if (delta < (sint16)(-rateMax))
            {
                delta = (sint16)(-rateMax);
            }

            steerOut = (sint16)(steerOut + delta);

            if (steerOut > (sint16)SERVO_TEST_CMD_CLAMP)
            {
                steerOut = (sint16)SERVO_TEST_CMD_CLAMP;
            }
            if (steerOut < (sint16)(-SERVO_TEST_CMD_CLAMP))
            {
                steerOut = (sint16)(-SERVO_TEST_CMD_CLAMP);
            }

            Steer((int)steerOut);
        }

        DisplayText(0U, "SERVO TEST", 10U, 0U);
        DisplayText(1U, "RAW:", 4U, 0U);
        DisplayValue(1U, (int)steerRaw, 4U, 5U);
        DisplayText(2U, "OUT:", 4U, 0U);
        DisplayValue(2U, (int)steerOut, 4U, 5U);
        DisplayText(3U, "LIM:", 4U, 0U);
        DisplayValue(3U, (int)SERVO_TEST_CMD_CLAMP, 4U, 5U);
        DisplayRefresh();
    }
}

static void mode_esc_test(void)
{
    int logicalCmd;
    int physicalCmd;
    uint8 pot;

    Board_InitDrivers();
    Timebase_Init();
    OnboardPot_Init();
    DisplayInit(0U, STD_ON);

    EscInit(ESC_PWM_CH, ESC_DUTY_MIN, ESC_DUTY_MED, ESC_DUTY_MAX);

    {
        uint32 t0 = Timebase_GetMs();
        while ((uint32)(Timebase_GetMs() - t0) < ESC_ARM_TIME_MS)
        {
            EscSetBrake(0U);
            EscSetSpeed(ESC_TRUE_NEUTRAL_CMD);
        }
    }

    for (;;)
    {
        Buttons_Update();
        pot = OnboardPot_ReadLevelFiltered();

        logicalCmd = ((int)pot * 200 / 255) - 100;

        if ((logicalCmd < (int)MOTOR_DEADBAND_PCT) &&
            (logicalCmd > -((int)MOTOR_DEADBAND_PCT)))
        {
            logicalCmd = 0;
        }

        if (Buttons_WasPressed(BUTTON_ID_SW2))
        {
            logicalCmd = 0;
        }

        physicalCmd = esc_apply_neutral_offset(logicalCmd);

        EscSetBrake(0U);
        EscSetSpeed(physicalCmd);

        DisplayText(0U, "ESC POT TEST", 12U, 0U);
        DisplayText(1U, "POT:", 4U, 0U);
        DisplayValue(1U, (int)pot, 3U, 5U);
        DisplayText(2U, "LOG:", 4U, 0U);
        DisplayValue(2U, logicalCmd, 4U, 5U);
        DisplayText(3U, "PHY:", 4U, 0U);
        DisplayValue(3U, physicalCmd, 4U, 5U);
        DisplayRefresh();
    }
}

/* =========================================================
   Shared runtime helpers for FINAL_DUMMY / NXP_CUP
========================================================= */
#if APP_TEST_FINAL_DUMMY || APP_TEST_NXP_CUP

static boolean time_reached(uint32 nowMs, uint32 dueMs)
{
    return ((uint32)(nowMs - dueMs) < 0x80000000u) ? TRUE : FALSE;
}

static void display_power_stabilize_delay(void)
{
    uint32 t0 = Timebase_GetMs();
    while ((uint32)(Timebase_GetMs() - t0) < 200u) { }
}

static void StatusLed_Blue(void)
{
    RgbLed_ChangeColor((RgbLed_Color){ .r = false, .g = false, .b = true });
}

static void StatusLed_Green(void)
{
    RgbLed_ChangeColor((RgbLed_Color){ .r = false, .g = true, .b = false });
}

static void StatusLed_Red(void)
{
    RgbLed_ChangeColor((RgbLed_Color){ .r = true, .g = false, .b = false });
}

static void steer_center_safe(void)
{
#if SERVO_OUTPUT_ENABLE
    SteerStraight();
#endif
}

static void steer_apply_safe(int steerCmd)
{
#if SERVO_OUTPUT_ENABLE
    Steer(steerCmd);
#else
    (void)steerCmd;
    SteerStraight();
#endif
}

static void Esc_StopNeutral(void)
{
    EscSetBrake(0U);
    EscSetSpeed(ESC_TRUE_NEUTRAL_CMD);
}

/* =========================================================
   ESC-only logic
========================================================= */
typedef struct
{
    CarMode_t mode;
    uint32 startGoMs;
} EscRunState_t;

static void esc_enter(EscRunState_t *st)
{
    st->mode = CAR_IDLE;
    st->startGoMs = 0u;

    EscInit(ESC_PWM_CH, ESC_DUTY_MIN, ESC_DUTY_MED, ESC_DUTY_MAX);

    EscSetBrake(0U);
    EscSetSpeed(ESC_TRUE_NEUTRAL_CMD);

    {
        uint32 t0 = Timebase_GetMs();
        while ((uint32)(Timebase_GetMs() - t0) < ESC_ARM_TIME_MS)
        {
            EscSetBrake(0U);
            EscSetSpeed(ESC_TRUE_NEUTRAL_CMD);
        }
    }

    Esc_StopNeutral();
}

static void esc_update(EscRunState_t *st, uint32 nowMs, boolean sw2, boolean sw3, uint8 pot)
{
    if (sw3)
    {
        st->mode = CAR_IDLE;
        Esc_StopNeutral();
        return;
    }

    if ((st->mode == CAR_IDLE) && sw2)
    {
        st->mode = CAR_ARMING;
        st->startGoMs = nowMs + START_DELAY_MS;

        EscSetSpeed(ESC_TRUE_NEUTRAL_CMD);
        EscSetBrake(0U);
    }

    if ((st->mode == CAR_ARMING) && time_reached(nowMs, st->startGoMs))
    {
        st->mode = CAR_RUN;
    }

    if (st->mode != CAR_RUN)
    {
        Esc_StopNeutral();
    }
    else
    {
        sint32 logicalCmd;

        if (pot <= (uint8)POT_CENTER_RAW)
        {
            uint16 denom = (uint16)(POT_CENTER_RAW - POT_LEFT_RAW);
            if (denom == 0u) { denom = 1u; }
            logicalCmd = -((sint32)((uint8)POT_CENTER_RAW - pot) * 100) / (sint32)denom;
        }
        else
        {
            uint16 denom = (uint16)(POT_RIGHT_RAW - POT_CENTER_RAW);
            if (denom == 0u) { denom = 1u; }
            logicalCmd = ((sint32)(pot - (uint8)POT_CENTER_RAW) * 100) / (sint32)denom;
        }

        if ((logicalCmd < (sint32)MOTOR_DEADBAND_PCT) &&
            (logicalCmd > -((sint32)MOTOR_DEADBAND_PCT)))
        {
            logicalCmd = 0;
        }

        EscSetBrake(0U);
        EscSetSpeed(esc_apply_neutral_offset((int)logicalCmd));
    }
}

/* =========================================================
   Shared camera + servo path
========================================================= */
typedef struct
{
    float kp;
    float kd;
    float ki;
    float steerLpfAlpha;
    sint16 steerClamp;
    sint16 steerRateMax;
    uint8 baseSpeedPct;
} CamTuneProfile_t;

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
    uint32 lastServoApplyMs;

    sint16 steerRaw;
    sint16 steerFilt;
    sint16 steerOut;

    uint32 nextTickMs;
    uint32 nextSteerMs;
    uint32 tickCount;

    CamTuneProfile_t activeTune;
} CamServoState_t;

static sint16 abs_s16(sint16 x)
{
    return (x < 0) ? (sint16)(-x) : x;
}

static sint16 clamp_s16(sint16 x, sint16 lo, sint16 hi)
{
    if (x < lo) { return lo; }
    if (x > hi) { return hi; }
    return x;
}

static sint16 iir_s16(sint16 y_prev, sint16 x, float alpha)
{
    float y;

    if (alpha < 0.0f) { alpha = 0.0f; }
    if (alpha > 1.0f) { alpha = 1.0f; }

    y = (float)y_prev + alpha * ((float)x - (float)y_prev);

    if (y > 32767.0f)  { y = 32767.0f; }
    if (y < -32768.0f) { y = -32768.0f; }

    return (sint16)y;
}

static sint16 steer_shape_s16(sint16 cmd, sint16 clamp)
{
    sint32 absCmd;
    sint32 quad;
    sint32 shaped;
    sint32 blendPct = (sint32)STEER_CMD_SHAPE_BLEND_PCT;

    if (clamp <= 0)
    {
        return 0;
    }

    absCmd = (sint32)abs_s16(cmd);
    if (absCmd > (sint32)clamp)
    {
        absCmd = (sint32)clamp;
    }

    quad = (absCmd * absCmd) / (sint32)clamp;
    shaped = (((100L - blendPct) * absCmd) + (blendPct * quad)) / 100L;

    if (shaped > absCmd)
    {
        shaped = absCmd;
    }

    return (cmd < 0) ? (sint16)(-shaped) : (sint16)shaped;
}

static sint16 steer_rate_limit_s16(sint16 current,
                                   sint16 target,
                                   sint16 baseRateMax,
                                   sint16 clamp)
{
    sint16 boost;
    sint16 rateMax;
    sint16 delta;

    if (baseRateMax < 0)
    {
        baseRateMax = 0;
    }

    boost = (sint16)(abs_s16(target) / (sint16)STEER_RATE_BOOST_DIV);
    if (boost > (sint16)STEER_RATE_BOOST_MAX)
    {
        boost = (sint16)STEER_RATE_BOOST_MAX;
    }

    rateMax = (sint16)(baseRateMax + boost);
    if (rateMax > clamp)
    {
        rateMax = clamp;
    }

    delta = (sint16)(target - current);
    delta = clamp_s16(delta, (sint16)(-rateMax), rateMax);

    return (sint16)(current + delta);
}

static void camservo_apply_profile(CamServoState_t *st, const CamTuneProfile_t *profile)
{
    if ((st == NULL) || (profile == NULL))
    {
        return;
    }

    st->activeTune = *profile;

    st->ctrl.kp = profile->kp;
    st->ctrl.kd = profile->kd;
    st->ctrl.ki = profile->ki;
    st->ctrl.steerScale = 1.0f;

    SteeringLinear_Reset(&st->ctrl);

    st->steerRaw = 0;
    st->steerFilt = 0;
    st->steerOut = 0;
    steer_center_safe();
    st->lastServoApplyMs = Timebase_GetMs();
}

static void camservo_enter(CamServoState_t *st, uint32 nowMs, const CamTuneProfile_t *profile)
{
    ServoInit(SERVO_PWM_CH, SERVO_DUTY_MAX, SERVO_DUTY_MIN, SERVO_DUTY_MED);
    steer_center_safe();

    if (LinearCameraIsBusy() == TRUE)
    {
        LinearCameraStopStream();
    }

    LinearCameraInit(CAM_CLK_PWM_CH,
                     CAM_SHUTTER_GPT_CH,
                     CAM_ADC_GROUP,
                     CAM_SHUTTER_PCR);
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
    st->lastServoApplyMs = nowMs;

    st->steerRaw = 0;
    st->steerFilt = 0;
    st->steerOut = 0;

    st->nextTickMs = nowMs;
    st->nextSteerMs = nowMs + STEER_UPDATE_MS;
    st->tickCount = 0u;

    camservo_apply_profile(st, profile);

    (void)LinearCameraStartStream();
}

static void camservo_update(CamServoState_t *st, uint32 nowMs, boolean allowSw2DebugUi)
{
    const uint32 LOOP_MS    = (uint32)V2_LOOP_PERIOD_MS;
    const uint32 DISP_MS    = (uint32)DISPLAY_PERIOD_MS;
    const uint32 DISP_TICKS = (DISP_MS / LOOP_MS);
    boolean doDisplay;
    boolean gotNewFrame = FALSE;

    if ((uint32)(nowMs - st->nextTickMs) < LOOP_MS)
    {
        if ((uint32)(nowMs - st->lastServoApplyMs) >= SERVO_REFRESH_MS)
        {
            steer_apply_safe((int)st->steerOut);
            st->lastServoApplyMs = nowMs;
        }
        return;
    }

    st->nextTickMs += LOOP_MS;
    st->tickCount++;

    doDisplay = ((DISP_TICKS != 0U) && ((st->tickCount % DISP_TICKS) == 0U));

    VisionDebug_OnTick(&st->vdbg, allowSw2DebugUi, FALSE);

    {
        LinearCameraFrame latestFrame;

        if (LinearCameraCopyLatestFrame(&latestFrame) == TRUE)
        {
            st->dbg.mask = (uint32)VLIN_DBG_NONE;
            st->dbg.filteredOut = (uint16*)0;
            st->dbg.gradientOut = (sint16*)0;

            if ((doDisplay == TRUE) &&
                (VisionDebug_WantsVisionDebugData(&st->vdbg) == TRUE))
            {
                VisionDebug_PrepareVisionDbg(&st->vdbg,
                                             &st->dbg,
                                             st->filteredBuf,
                                             st->gradientBuf);
            }

            (void)memcpy(st->processedFrame.Values,
                         &latestFrame.Values[CAM_TRIM_LEFT_PX],
                         ((size_t)VISION_LINEAR_BUFFER_SIZE * sizeof(st->processedFrame.Values[0])));

            VisionLinear_ProcessFrameEx(st->processedFrame.Values, &st->result, &st->dbg);

            st->haveValidVision = TRUE;
            st->lastFrameMs = nowMs;
            gotNewFrame = TRUE;
        }
    }

    if (time_reached(nowMs, st->nextSteerMs))
    {
        boolean frameHardStale;

        st->nextSteerMs += STEER_UPDATE_MS;
        frameHardStale = ((uint32)(nowMs - st->lastFrameMs) > CAM_STEER_HOLD_MS) ? TRUE : FALSE;

        if ((st->haveValidVision != TRUE) ||
                 (frameHardStale == TRUE))
        {
            st->steerRaw = 0;
            st->steerFilt = 0;
            st->steerOut = 0;
            steer_center_safe();
            st->lastServoApplyMs = nowMs;
        }
        else
        {
            if (gotNewFrame == TRUE)
            {
                const float dt = ((float)STEER_UPDATE_MS) * 0.001f;
                SteeringOutput_t out;

                out = SteeringLinear_UpdateV2(&st->ctrl,
                                              &st->result,
                                              dt,
                                              st->activeTune.baseSpeedPct);

                if (out.brake != TRUE)
                {
                    st->steerRaw = (sint16)out.steer_cmd;

                    if (abs_s16(st->steerRaw) <= (sint16)STEER_CMD_DEADBAND)
                    {
                        st->steerRaw = 0;
                    }
                    else
                    {
                        st->steerRaw = steer_shape_s16(st->steerRaw,
                                                       st->activeTune.steerClamp);
                    }

                    st->steerFilt = iir_s16(st->steerFilt,
                                            st->steerRaw,
                                            st->activeTune.steerLpfAlpha);

                    st->steerOut = steer_rate_limit_s16(st->steerOut,
                                                        st->steerFilt,
                                                        st->activeTune.steerRateMax,
                                                        st->activeTune.steerClamp);

                    st->steerOut = clamp_s16(st->steerOut,
                                             (sint16)(-st->activeTune.steerClamp),
                                             (sint16)(+st->activeTune.steerClamp));
                }
                else
                {
                    /* A fresh frame reported track-lost/brake. Do not keep
                       driving the servo with the last non-zero command. */
                    st->steerRaw = 0;
                    st->steerFilt = iir_s16(st->steerFilt, 0, st->activeTune.steerLpfAlpha);
                    st->steerOut = steer_rate_limit_s16(st->steerOut,
                                                        0,
                                                        st->activeTune.steerRateMax,
                                                        st->activeTune.steerClamp);
                }
            }

            steer_apply_safe((int)st->steerOut);
            st->lastServoApplyMs = nowMs;
        }
    }
    else if (((uint32)(nowMs - st->lastServoApplyMs) >= SERVO_REFRESH_MS) &&
             ((gotNewFrame == TRUE) ||
              ((uint32)(nowMs - st->lastFrameMs) <= CAM_STEER_HOLD_MS)))
    {
        steer_apply_safe((int)st->steerOut);
        st->lastServoApplyMs = nowMs;
    }

    if ((doDisplay == TRUE) && (st->haveValidVision == TRUE))
    {
        const uint16 *pFiltered =
            (st->dbg.filteredOut != (uint16*)0) ? st->filteredBuf : (const uint16*)0;
        const sint16 *pGradient =
            (st->dbg.gradientOut != (sint16*)0) ? st->gradientBuf : (const sint16*)0;
        const VisionLinear_DebugOut_t *pDbg =
            (st->dbg.mask != (uint32)VLIN_DBG_NONE) ? &st->dbg : (const VisionLinear_DebugOut_t*)0;

        VisionDebug_Draw(&st->vdbg,
                         st->processedFrame.Values,
                         pFiltered,
                         pGradient,
                         &st->result,
                         pDbg);
    }
}

#endif /* APP_TEST_FINAL_DUMMY || APP_TEST_NXP_CUP */

/* =========================================================
   FINAL DUMMY
========================================================= */
#if APP_TEST_FINAL_DUMMY

static void mode_final_dummy(void)
{
    typedef enum
    {
        DUMMY_ESC = 0,
        DUMMY_CAM = 1
    } DummyState_t;

    static const CamTuneProfile_t finalDummyProfile =
    {
        KP,
        KD,
        KI,
        STEER_LPF_ALPHA,
        (sint16)STEER_CMD_CLAMP,
        8,
        (uint8)FULL_AUTO_SPEED_PCT
    };

    DummyState_t state = DUMMY_ESC;
    sint32 autoSpeedPct = 0;
    uint32 nextAutoSpeedMs = 0u;

    EscRunState_t escSt;
    CamServoState_t camSt;

    Board_InitDrivers();
    Timebase_Init();
    OnboardPot_Init();

    display_power_stabilize_delay();
    DisplayInit(0U, STD_ON);

    esc_enter(&escSt);
    StatusLed_Blue();

    for (;;)
    {
        uint32 now = Timebase_GetMs();
        boolean sw2;
        boolean sw3;
        uint8 pot;

        Buttons_Update();
        sw2 = Buttons_WasPressed(BUTTON_ID_SW2);
        sw3 = Buttons_WasPressed(BUTTON_ID_SW3);
        pot = OnboardPot_ReadLevelFiltered();

        if (sw3)
        {
            if (state == DUMMY_ESC)
            {
                Esc_StopNeutral();

                camservo_enter(&camSt, now, &finalDummyProfile);
                state = DUMMY_CAM;
                StatusLed_Green();

                autoSpeedPct = 0;
                nextAutoSpeedMs = now;
                EscSetBrake(0U);
                EscSetSpeed(ESC_TRUE_NEUTRAL_CMD);
            }
            else
            {
                steer_center_safe();
                LinearCameraStopStream();

                esc_enter(&escSt);
                state = DUMMY_ESC;
                StatusLed_Blue();

                autoSpeedPct = 0;
                nextAutoSpeedMs = 0u;
            }

            sw3 = FALSE;
        }

        if (state == DUMMY_ESC)
        {
            esc_update(&escSt, now, sw2, sw3, pot);

            DisplayText(0U, "FINAL DUMMY", 11U, 0U);
            DisplayText(1U, "SW2 START SW3 CAM", 18U, 0U);

            if (escSt.mode == CAR_IDLE)
            {
                DisplayText(2U, "MODE: IDLE ", 11U, 0U);
            }
            else if (escSt.mode == CAR_ARMING)
            {
                DisplayText(2U, "MODE: ARM  ", 11U, 0U);
            }
            else
            {
                DisplayText(2U, "MODE: RUN  ", 11U, 0U);
            }

            DisplayText(3U, "POT:", 4U, 0U);
            DisplayValue(3U, (int)pot, 3U, 5U);
            DisplayRefresh();
        }
        else
        {
            camservo_update(&camSt, now, sw2);

            if (time_reached(now, nextAutoSpeedMs))
            {
                nextAutoSpeedMs = now + FULL_AUTO_RAMP_PERIOD_MS;

                if (autoSpeedPct < (sint32)camSt.activeTune.baseSpeedPct)
                {
                    autoSpeedPct += (sint32)FULL_AUTO_RAMP_STEP_PCT;
                    if (autoSpeedPct > (sint32)camSt.activeTune.baseSpeedPct)
                    {
                        autoSpeedPct = (sint32)camSt.activeTune.baseSpeedPct;
                    }
                }

                if (autoSpeedPct < 0)   { autoSpeedPct = 0; }
                if (autoSpeedPct > 100) { autoSpeedPct = 100; }

                EscSetBrake(0U);
                EscSetSpeed(esc_apply_neutral_offset((int)(-autoSpeedPct)));
            }
        }
    }
}

#endif /* APP_TEST_FINAL_DUMMY */

/* =========================================================
   NXP CUP
========================================================= */
#if APP_TEST_NXP_CUP

typedef enum
{
    NXP_CUP_PROFILE_SUPERFAST = 0,
    NXP_CUP_PROFILE_5050      = 1,
    NXP_CUP_PROFILE_SLOW      = 2,
    NXP_CUP_PROFILE_COUNT     = 3
} NxpCupProfileId_t;

typedef enum
{
    NXP_CUP_STATE_MENU = 0,
    NXP_CUP_STATE_READY,
    NXP_CUP_STATE_ESC_REARM,
    NXP_CUP_STATE_RUN
} NxpCupState_t;

typedef enum
{
    NXP_CUP_ULTRA_CLEAR = 0,
    NXP_CUP_ULTRA_STOP_HOLD,
    NXP_CUP_ULTRA_CRAWL,
    NXP_CUP_ULTRA_CUTOFF
} NxpCupUltraMode_t;

typedef struct
{
    boolean enabled;
    boolean haveValidDistance;
    float   lastDistanceCm;
    uint32  nextTriggerMs;
    uint32  ultraEnableMs;
    NxpCupUltraMode_t mode;
    uint32  modeUntilMs;
} NxpCupUltraState_t;

static const CamTuneProfile_t gNxpCupProfiles[NXP_CUP_PROFILE_COUNT] =
{
    {
        NXP_CUP_SUPERFAST_KP,
        NXP_CUP_SUPERFAST_KD,
        NXP_CUP_SUPERFAST_KI,
        NXP_CUP_SUPERFAST_STEER_LPF_ALPHA,
        (sint16)NXP_CUP_SUPERFAST_STEER_CLAMP,
        (sint16)NXP_CUP_SUPERFAST_STEER_RATE_MAX,
        (uint8)NXP_CUP_SUPERFAST_SPEED_PCT
    },
    {
        NXP_CUP_5050_KP,
        NXP_CUP_5050_KD,
        NXP_CUP_5050_KI,
        NXP_CUP_5050_STEER_LPF_ALPHA,
        (sint16)NXP_CUP_5050_STEER_CLAMP,
        (sint16)NXP_CUP_5050_STEER_RATE_MAX,
        (uint8)NXP_CUP_5050_SPEED_PCT
    },
    {
        NXP_CUP_SLOW_KP,
        NXP_CUP_SLOW_KD,
        NXP_CUP_SLOW_KI,
        NXP_CUP_SLOW_STEER_LPF_ALPHA,
        (sint16)NXP_CUP_SLOW_STEER_CLAMP,
        (sint16)NXP_CUP_SLOW_STEER_RATE_MAX,
        (uint8)NXP_CUP_SLOW_SPEED_PCT
    }
};

static void nxp_cup_idle_motor(void)
{
    EscSetBrake(0U);
    EscSetSpeed(ESC_TRUE_NEUTRAL_CMD);
}

static void nxp_cup_obstacle_stop_motor(void)
{
    EscSetBrake(0U);
    EscSetSpeed(ESC_TRUE_NEUTRAL_CMD);
}

static void nxp_cup_launch_motor(int logicalForwardCmd)
{
    int physicalCmd = esc_apply_neutral_offset(logicalForwardCmd);

    EscSetBrake(0U);
    EscSetSpeed(physicalCmd);
    busy_delay(30000U);

    EscSetBrake(0U);
    EscSetSpeed(physicalCmd);
    busy_delay(30000U);

    EscSetBrake(0U);
    EscSetSpeed(physicalCmd);
}

static uint8 nxp_cup_profile_from_pot(uint8 pot)
{
    if (pot < 85U)
    {
        return (uint8)NXP_CUP_PROFILE_SUPERFAST;
    }
    else if (pot < 170U)
    {
        return (uint8)NXP_CUP_PROFILE_5050;
    }
    else
    {
        return (uint8)NXP_CUP_PROFILE_SLOW;
    }
}

static void nxp_cup_ultra_enter(NxpCupUltraState_t *st, uint32 nowMs)
{
    if (st == NULL)
    {
        return;
    }

#if NXP_CUP_ULTRASONIC_ENABLE
    st->enabled = TRUE;
    st->haveValidDistance = FALSE;
    st->lastDistanceCm = 0.0f;
    /* Start sampling immediately so the sensor already has fresh data by RUN. */
    st->nextTriggerMs = nowMs;
    st->ultraEnableMs = 0u;
    st->mode = NXP_CUP_ULTRA_CLEAR;
    st->modeUntilMs = 0u;
    Ultrasonic_Init();
#else
    st->enabled = FALSE;
    st->haveValidDistance = FALSE;
    st->lastDistanceCm = 0.0f;
    st->nextTriggerMs = 0u;
    st->ultraEnableMs = 0u;
    st->mode = NXP_CUP_ULTRA_CLEAR;
    st->modeUntilMs = 0u;
#endif
}

static void nxp_cup_ultra_arm_for_run(NxpCupUltraState_t *st, uint32 nowMs)
{
    if (st == NULL)
    {
        return;
    }

    /* Keep the most recent pre-launch measurement so RUN can react on its
       first cycle instead of waiting for another fresh frame. */
    st->ultraEnableMs = nowMs + NXP_ULTRA_ENABLE_AFTER_RUN_MS;
    /* Force an immediate measurement on launch instead of waiting another period. */
    st->nextTriggerMs = nowMs;
    st->mode = NXP_CUP_ULTRA_CLEAR;
    st->modeUntilMs = 0u;
}

static boolean nxp_cup_ultra_is_active(const NxpCupUltraState_t *st, uint32 nowMs)
{
    if ((st == NULL) || (st->enabled != TRUE))
    {
        return FALSE;
    }

    if (st->ultraEnableMs == 0u)
    {
        return FALSE;
    }

    if (time_reached(nowMs, st->ultraEnableMs) != TRUE)
    {
        return FALSE;
    }

    return TRUE;
}

static void nxp_cup_ultra_task(NxpCupUltraState_t *st, uint32 nowMs)
{
    float d;

    if ((st == NULL) || (st->enabled != TRUE))
    {
        return;
    }

    if (time_reached(nowMs, st->nextTriggerMs))
    {
        if (Ultrasonic_GetStatus() != ULTRA_STATUS_BUSY)
        {
            Ultrasonic_StartMeasurement();
            st->nextTriggerMs = nowMs + (uint32)NXP_CUP_ULTRA_TRIGGER_PERIOD_MS;
        }
    }

    Ultrasonic_Task();

    if (Ultrasonic_GetDistanceCm(&d) == TRUE)
    {
        if (d > 0.0f)
        {
            st->lastDistanceCm = d;
            st->haveValidDistance = TRUE;
        }
    }

    if (nxp_cup_ultra_is_active(st, nowMs) != TRUE)
    {
        st->mode = NXP_CUP_ULTRA_CLEAR;
        st->modeUntilMs = 0u;
        return;
    }

    /* Sequence:
       1) <= 50 cm -> stop motor for 2 s
       2) after hold -> fixed 10% crawl
       3) <= 8 cm -> latch cutoff for ESC + steering */
    if (st->mode == NXP_CUP_ULTRA_CUTOFF)
    {
        return;
    }

    if (st->haveValidDistance != TRUE)
    {
        return;
    }

    if (st->lastDistanceCm <= (float)NXP_CUP_ULTRA_CRAWL_STOP_CM)
    {
        st->mode = NXP_CUP_ULTRA_CUTOFF;
        st->modeUntilMs = 0u;
        return;
    }

    switch (st->mode)
    {
        case NXP_CUP_ULTRA_STOP_HOLD:
        {
            if ((st->modeUntilMs != 0u) &&
                (time_reached(nowMs, st->modeUntilMs) == TRUE))
            {
                if (st->lastDistanceCm <= (float)NXP_CUP_ULTRA_STOP_CM)
                {
                    st->mode = NXP_CUP_ULTRA_CRAWL;
                }
                else
                {
                    st->mode = NXP_CUP_ULTRA_CLEAR;
                }
                st->modeUntilMs = 0u;
            }
            break;
        }

        case NXP_CUP_ULTRA_CRAWL:
        {
            break;
        }

        case NXP_CUP_ULTRA_CLEAR:
        default:
        {
            if (st->lastDistanceCm <= (float)NXP_CUP_ULTRA_STOP_CM)
            {
                st->mode = NXP_CUP_ULTRA_STOP_HOLD;
                st->modeUntilMs = nowMs + (uint32)NXP_CUP_ULTRA_STOP_HOLD_MS;
            }
            break;
        }
    }
}

static boolean nxp_cup_ultra_should_hold_servo(const NxpCupUltraState_t *st,
                                               uint32 nowMs)
{
    if ((st == NULL) || (st->enabled != TRUE))
    {
        return FALSE;
    }

    if (nxp_cup_ultra_is_active(st, nowMs) != TRUE)
    {
        return FALSE;
    }

    if ((st->mode == NXP_CUP_ULTRA_STOP_HOLD) ||
        (st->mode == NXP_CUP_ULTRA_CRAWL) ||
        (st->mode == NXP_CUP_ULTRA_CUTOFF))
    {
        return TRUE;
    }

    return FALSE;
}

static uint8 nxp_cup_ultra_target_speed_pct(const NxpCupUltraState_t *st,
                                            uint32 nowMs,
                                            uint8 requestedSpeedPct)
{
    if ((st == NULL) || (st->enabled != TRUE))
    {
        return requestedSpeedPct;
    }

    if (nxp_cup_ultra_is_active(st, nowMs) != TRUE)
    {
        return requestedSpeedPct;
    }

    if (st->mode == NXP_CUP_ULTRA_STOP_HOLD)
    {
        return 0U;
    }

    /* After the hold, stay at a fixed crawl speed until the cutoff distance. */
    if ((st->mode == NXP_CUP_ULTRA_CRAWL) &&
        (st->haveValidDistance == TRUE))
    {
        if ((uint8)NXP_CUP_ULTRA_CRAWL_SPEED_PCT > requestedSpeedPct)
        {
            return requestedSpeedPct;
        }
        return (uint8)NXP_CUP_ULTRA_CRAWL_SPEED_PCT;
    }

    if (st->mode == NXP_CUP_ULTRA_CUTOFF)
    {
        return 0U;
    }

    return requestedSpeedPct;
}

static void mode_nxp_cup(void)
{
    CamServoState_t camSt;
    NxpCupUltraState_t ultraSt;
    NxpCupState_t state = NXP_CUP_STATE_MENU;
    NxpCupProfileId_t profileId = (NxpCupProfileId_t)NXP_CUP_DEFAULT_PROFILE;

    sint32 autoSpeedPct = 0;
    uint32 nextAutoSpeedMs = 0u;
    uint32 escRearmDoneMs = 0u;

    uint8 pot = 0U;
    boolean cameraStarted = FALSE;
    boolean sw2;
    boolean sw3;
    boolean systemBad;

    Board_InitDrivers();
    Timebase_Init();
    OnboardPot_Init();

    display_power_stabilize_delay();
    DisplayInit(0U, STD_ON);

    /* IMPORTANT:
       Do NOT do the full ESC arm here.
       The final ESC arming/beep sequence must happen only after
       the user chooses the profile and presses SW3. */

    nxp_cup_idle_motor();
    nxp_cup_ultra_enter(&ultraSt, Timebase_GetMs());
    StatusLed_Blue();

    for (;;)
    {
        uint32 now = Timebase_GetMs();

        Buttons_Update();
        sw2 = Buttons_WasPressed(BUTTON_ID_SW2);
        sw3 = Buttons_WasPressed(BUTTON_ID_SW3);
        pot = OnboardPot_ReadLevelFiltered();

        systemBad = FALSE;
        if ((state == NXP_CUP_STATE_RUN) &&
            (cameraStarted == TRUE) &&
            (ultraSt.mode != NXP_CUP_ULTRA_CUTOFF) &&
            ((camSt.haveValidVision != TRUE) ||
             ((uint32)(now - camSt.lastFrameMs) > CAM_STEER_HOLD_MS)))
        {
            systemBad = TRUE;
        }

        if (systemBad == TRUE)
        {
            StatusLed_Red();
            autoSpeedPct = 0;
            nxp_cup_idle_motor();
        }
        else if ((state == NXP_CUP_STATE_RUN) || (state == NXP_CUP_STATE_ESC_REARM))
        {
            StatusLed_Green();
        }
        else
        {
            StatusLed_Blue();
        }

        if (state == NXP_CUP_STATE_MENU)
        {
            profileId = (NxpCupProfileId_t)nxp_cup_profile_from_pot(pot);

            if (cameraStarted == TRUE)
            {
                LinearCameraStopStream();
                cameraStarted = FALSE;
            }

            nxp_cup_idle_motor();
            steer_center_safe();
            autoSpeedPct = 0;
            nextAutoSpeedMs = 0u;
            escRearmDoneMs = 0u;

            DisplayText(0U, "NXP CUP MENU", 12U, 0U);

            if (profileId == NXP_CUP_PROFILE_SUPERFAST)
            {
                DisplayText(1U, ">SUPERFAST", 10U, 0U);
                DisplayText(2U, " 5050", 5U, 0U);
                DisplayText(3U, " SLOW SW2 ENTER", 15U, 0U);
            }
            else if (profileId == NXP_CUP_PROFILE_5050)
            {
                DisplayText(1U, " SUPERFAST", 10U, 0U);
                DisplayText(2U, ">5050", 5U, 0U);
                DisplayText(3U, " SLOW SW2 ENTER", 15U, 0U);
            }
            else
            {
                DisplayText(1U, " SUPERFAST", 10U, 0U);
                DisplayText(2U, " 5050", 5U, 0U);
                DisplayText(3U, ">SLOW SW2 ENTER", 15U, 0U);
            }

            DisplayRefresh();

            if (sw2 == TRUE)
            {
                state = NXP_CUP_STATE_READY;
            }

            continue;
        }

        if (state == NXP_CUP_STATE_READY)
        {
            if (cameraStarted == TRUE)
            {
                LinearCameraStopStream();
                cameraStarted = FALSE;
            }

            nxp_cup_idle_motor();
            steer_center_safe();
            autoSpeedPct = 0;
            nextAutoSpeedMs = 0u;
            escRearmDoneMs = 0u;

            DisplayText(0U, "NXP CUP READY", 13U, 0U);

            if (profileId == NXP_CUP_PROFILE_SUPERFAST)
            {
                DisplayText(1U, "PROFILE:SUPER", 13U, 0U);
            }
            else if (profileId == NXP_CUP_PROFILE_5050)
            {
                DisplayText(1U, "PROFILE:5050 ", 13U, 0U);
            }
            else
            {
                DisplayText(1U, "PROFILE:SLOW ", 13U, 0U);
            }

            DisplayText(2U, "SW3 START", 9U, 0U);
            DisplayText(3U, "SW2 BACK", 8U, 0U);
            DisplayRefresh();

            if (sw2 == TRUE)
            {
                state = NXP_CUP_STATE_MENU;
                continue;
            }

            if (sw3 == TRUE)
            {
                camservo_enter(&camSt, now, &gNxpCupProfiles[profileId]);
                cameraStarted = TRUE;

                autoSpeedPct = 0;
                nextAutoSpeedMs = 0u;

                /* Ultrasonic exists, but still cannot affect launch */
                nxp_cup_ultra_enter(&ultraSt, now);

                /* Re-arm ESC HERE so the final beep happens now,
                   after profile selection and just before launch */
                EscInit(ESC_PWM_CH, ESC_DUTY_MIN, ESC_DUTY_MED, ESC_DUTY_MAX);

                escRearmDoneMs = now + ESC_ARM_TIME_MS + NXP_ESC_EXTRA_SETTLE_MS;

                state = NXP_CUP_STATE_ESC_REARM;
                continue;
            }

            continue;
        }

        if (state == NXP_CUP_STATE_ESC_REARM)
        {
            nxp_cup_ultra_task(&ultraSt, now);

            if (cameraStarted == TRUE)
            {
                camservo_update(&camSt, now, FALSE);
            }

            if (sw2 == TRUE)
            {
                nxp_cup_idle_motor();

                if (cameraStarted == TRUE)
                {
                    LinearCameraStopStream();
                    cameraStarted = FALSE;
                }

                steer_center_safe();
                autoSpeedPct = 0;
                nextAutoSpeedMs = 0u;
                escRearmDoneMs = 0u;
                state = NXP_CUP_STATE_MENU;
                continue;
            }

            if (sw3 == TRUE)
            {
                nxp_cup_idle_motor();

                if (cameraStarted == TRUE)
                {
                    LinearCameraStopStream();
                    cameraStarted = FALSE;
                }

                steer_center_safe();
                autoSpeedPct = 0;
                nextAutoSpeedMs = 0u;
                escRearmDoneMs = 0u;
                state = NXP_CUP_STATE_READY;
                continue;
            }

            /* Keep feeding real neutral while ESC finishes arming */
            EscSetBrake(0U);
            EscSetSpeed(ESC_TRUE_NEUTRAL_CMD);

            if (time_reached(now, escRearmDoneMs))
            {
                autoSpeedPct = (sint32)camSt.activeTune.baseSpeedPct;

                if (autoSpeedPct < 0)   { autoSpeedPct = 0; }
                if (autoSpeedPct > 100) { autoSpeedPct = 100; }

                /* Only after launch do we start the ultrasonic enable timer */
                nxp_cup_ultra_arm_for_run(&ultraSt, now);
                nxp_cup_ultra_task(&ultraSt, now);

                if ((ultraSt.mode == NXP_CUP_ULTRA_STOP_HOLD) ||
                    (ultraSt.mode == NXP_CUP_ULTRA_CUTOFF))
                {
                    autoSpeedPct = 0;
                    nxp_cup_obstacle_stop_motor();
                    steer_center_safe();
                }
                else
                {
                    sint32 launchSpeedPct =
                        (sint32)nxp_cup_ultra_target_speed_pct(&ultraSt,
                                                               now,
                                                               (uint8)autoSpeedPct);

                    if (launchSpeedPct < 0)   { launchSpeedPct = 0; }
                    if (launchSpeedPct > 100) { launchSpeedPct = 100; }

                    autoSpeedPct = launchSpeedPct;
                    nxp_cup_launch_motor((int)(-autoSpeedPct));
                }

                nextAutoSpeedMs = now + FULL_AUTO_RAMP_PERIOD_MS;
                state = NXP_CUP_STATE_RUN;
            }

            continue;
        }

        /* Ultrasonic only acts after RUN and after the long enable delay */
        nxp_cup_ultra_task(&ultraSt, now);

        if (cameraStarted == TRUE)
        {
            /* While ultrasonic is handling an obstacle, keep steering pointed
               straight ahead instead of continuing camera corrections. */
            if (nxp_cup_ultra_should_hold_servo(&ultraSt, now) == TRUE)
            {
                steer_center_safe();
            }
            else
            {
                camservo_update(&camSt, now, FALSE);
            }
        }

        if ((ultraSt.mode == NXP_CUP_ULTRA_STOP_HOLD) ||
            (ultraSt.mode == NXP_CUP_ULTRA_CUTOFF))
        {
            autoSpeedPct = 0;
            nxp_cup_obstacle_stop_motor();
        }

        if (sw2 == TRUE)
        {
            nxp_cup_idle_motor();

            if (cameraStarted == TRUE)
            {
                LinearCameraStopStream();
                cameraStarted = FALSE;
            }

            steer_center_safe();
            autoSpeedPct = 0;
            nextAutoSpeedMs = 0u;
            escRearmDoneMs = 0u;
            state = NXP_CUP_STATE_MENU;
            continue;
        }

        if (sw3 == TRUE)
        {
            nxp_cup_idle_motor();

            if (cameraStarted == TRUE)
            {
                LinearCameraStopStream();
                cameraStarted = FALSE;
            }

            steer_center_safe();
            autoSpeedPct = 0;
            nextAutoSpeedMs = 0u;
            escRearmDoneMs = 0u;
            state = NXP_CUP_STATE_READY;
            continue;
        }

        if (time_reached(now, nextAutoSpeedMs))
        {
            sint32 targetSpeedPct;
            int logicalCmd;

            nextAutoSpeedMs = now + FULL_AUTO_RAMP_PERIOD_MS;

            targetSpeedPct =
                (sint32)nxp_cup_ultra_target_speed_pct(&ultraSt,
                                                       now,
                                                       (uint8)camSt.activeTune.baseSpeedPct);

            if (targetSpeedPct < 0)   { targetSpeedPct = 0; }
            if (targetSpeedPct > 100) { targetSpeedPct = 100; }

            if (ultraSt.mode == NXP_CUP_ULTRA_CRAWL)
            {
                autoSpeedPct = targetSpeedPct;
            }
            else if (autoSpeedPct < targetSpeedPct)
            {
                autoSpeedPct += (sint32)FULL_AUTO_RAMP_STEP_PCT;
                if (autoSpeedPct > targetSpeedPct)
                {
                    autoSpeedPct = targetSpeedPct;
                }
            }
            else if (autoSpeedPct > targetSpeedPct)
            {
                autoSpeedPct -= (sint32)NXP_CUP_RAMP_DOWN_STEP_PCT;
                if (autoSpeedPct < targetSpeedPct)
                {
                    autoSpeedPct = targetSpeedPct;
                }
            }

            if (autoSpeedPct < 0)   { autoSpeedPct = 0; }
            if (autoSpeedPct > 100) { autoSpeedPct = 100; }

            logicalCmd = (int)(-autoSpeedPct);

            EscSetBrake(0U);
            EscSetSpeed(esc_apply_neutral_offset(logicalCmd));
        }
    }
}

#endif /* APP_TEST_NXP_CUP */

/* =========================================================
   Linear camera test
========================================================= */
static void mode_linear_camera_test(void)
{
    VisionDebug_State_t vdbg;
    Sw2Tracker_t sw2Tracker = { FALSE, FALSE, 0U };
    static LinearCameraFrame processedFrame;
    VisionOutput_t result;
    VisionLinear_DebugOut_t dbg;
    static uint16 filteredBuf[VISION_LINEAR_BUFFER_SIZE];
    static sint16 gradientBuf[VISION_LINEAR_BUFFER_SIZE];
    const uint32 loopPeriodMs = 5U;
    const uint32 displayPeriodMs = 20U;
    const uint32 displayTicks = (displayPeriodMs / loopPeriodMs);
    uint32 nextTickMs;
    uint32 tickCount = 0U;
    boolean haveValidVision = FALSE;
    boolean paused = FALSE;

    Board_InitCommonApp();
    VisionLinear_InitV2();
    VisionDebug_Init(&vdbg, 3200U);

    nextTickMs = Timebase_GetMs();

    (void)LinearCameraStartStream();

    for (;;)
    {
        uint32 nowMs = Timebase_GetMs();
        Sw2Action_t sw2Action;
        boolean modeNextPressed;
        boolean sw2ClickHandled = FALSE;

        if ((uint32)(nowMs - nextTickMs) < loopPeriodMs)
        {
            continue;
        }

        nextTickMs += loopPeriodMs;
        tickCount++;

        Buttons_Update();
        sw2Action = Sw2Tracker_Update(&sw2Tracker, nowMs);
        modeNextPressed = Buttons_WasPressed(BUTTON_ID_SW3);

        if ((sw2Action == SW2_ACTION_HOLD) && (paused != TRUE))
        {
            if (LinearCameraIsBusy() == TRUE)
            {
                LinearCameraStopStream();
            }
            paused = TRUE;
            vdbg.cfg.paused = TRUE;
        }
        else if ((sw2Action == SW2_ACTION_CLICK) && (paused == TRUE))
        {
            paused = FALSE;
            vdbg.cfg.paused = FALSE;
            vdbg.screen = vdbg.debugEntryScreen;
            (void)LinearCameraStartStream();
            sw2ClickHandled = TRUE;
        }

        if (paused == TRUE)
        {
            uint8 potLevel = OnboardPot_ReadLevelFiltered();
            vdbg.cfg.whiteSat = VisionDebug_WhiteMaxFromPot(potLevel);
        }

        VisionDebug_OnTick(&vdbg,
                           (boolean)((sw2Action == SW2_ACTION_CLICK) &&
                                     (sw2ClickHandled != TRUE) &&
                                     (paused != TRUE)),
                           modeNextPressed);

        RgbLed_ChangeColor((RgbLed_Color){ .r = true, .g = false, .b = false });

        if (paused != TRUE)
        {
            LinearCameraFrame latestFrame;

            if (LinearCameraCopyLatestFrame(&latestFrame) == TRUE)
            {
                dbg.mask = (uint32)VLIN_DBG_NONE;
                dbg.filteredOut = (uint16*)0;
                dbg.gradientOut = (sint16*)0;

                if (VisionDebug_WantsVisionDebugData(&vdbg) == TRUE)
                {
                    VisionDebug_PrepareVisionDbg(&vdbg, &dbg, filteredBuf, gradientBuf);
                }

                RgbLed_ChangeColor((RgbLed_Color){ .r = false, .g = false, .b = true });

                (void)memcpy(processedFrame.Values,
                             &latestFrame.Values[CAM_TRIM_LEFT_PX],
                             ((size_t)VISION_LINEAR_BUFFER_SIZE * sizeof(processedFrame.Values[0])));
                VisionLinear_ProcessFrameEx(processedFrame.Values, &result, &dbg);

                haveValidVision = TRUE;
            }
        }

        if ((displayTicks != 0U) &&
            ((tickCount % displayTicks) == 0U) &&
            (haveValidVision == TRUE))
        {
            const uint16 *pFiltered =
                (dbg.filteredOut != (uint16*)0) ? filteredBuf : (const uint16*)0;
            const sint16 *pGradient =
                (dbg.gradientOut != (sint16*)0) ? gradientBuf : (const sint16*)0;

            const VisionLinear_DebugOut_t *pDbg =
                (dbg.mask != (uint32)VLIN_DBG_NONE) ? &dbg : (const VisionLinear_DebugOut_t*)0;

            VisionDebug_Draw(&vdbg, processedFrame.Values, pFiltered, pGradient, &result, pDbg);
        }

        RgbLed_ChangeColor((RgbLed_Color){ .r = false, .g = false, .b = false });
    }
}

/* =========================================================
   Dispatcher
========================================================= */
void App_RunSelectedMode(void)
{
#if APP_TEST_NXP_CUP
    mode_nxp_cup();
#elif APP_TEST_FINAL_DUMMY
    mode_final_dummy();
#elif APP_TEST_LINEAR_CAMERA_TEST
    mode_linear_camera_test();
#elif APP_TEST_RECEIVER_TEST
    mode_receiver_test();
#elif APP_TEST_SERVO_TEST
    mode_servo_test();
#elif APP_TEST_ESC_TEST
    mode_esc_test();
#else
    while (1) { }
#endif
}
