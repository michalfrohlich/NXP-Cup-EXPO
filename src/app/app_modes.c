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
static void busy_delay(volatile uint32 ticks)
{
    while (ticks != 0U)
    {
        ticks--;
    }
}

static void mode_receiver_test(void)
{
    int receiverChannels[8];
    uint8 displayValueOffset;

    Board_InitDrivers();
    DisplayInit(0U, STD_ON);

    /* Historical receiver setup from the first commit:
       ReceiverInit(0U, 0U, 11700U, 17700U, 23700U, 26000U).
       The receiver has not been physically connected in this project yet,
       so these values are preserved as a starting point only and must be
       revalidated when the hardware is wired and the ICU/GPT path is tested. */
    ReceiverInit(0U, 0U, 11700U, 17700U, 23700U, 26000U);

    DisplayText(0U, "Ch0:    Ch1:", 12U, 0U);
    DisplayText(1U, "Ch2:    Ch3:", 12U, 0U);
    DisplayText(2U, "Ch4:    Ch5:", 12U, 0U);
    DisplayText(3U, "Ch6:    Ch7:", 12U, 0U);

    for (;;)
    {
        for (uint8 i = 0U; i < 8U; i++)
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
    Board_InitDrivers();
    ServoInit(SERVO_PWM_CH, SERVO_DUTY_MAX, SERVO_DUTY_MIN, SERVO_DUTY_MED);

    for (;;)
    {
        SteerRight();
        busy_delay(5000000U);
        SteerStraight();
        busy_delay(5000000U);
        SteerLeft();
        busy_delay(5000000U);

        for (int steerStrength = -100; steerStrength <= 100; steerStrength++)
        {
            busy_delay(100000U);
            Steer(steerStrength);
        }
    }
}

static void mode_esc_test(void)
{
    Board_InitDrivers();
    EscInit(ESC_PWM_CH, ESC_DUTY_MIN, ESC_DUTY_MED, ESC_DUTY_MAX);

    for (;;)
    {
        for (int speed = 0; speed <= 100; speed++)
        {
            busy_delay(500000U);
            EscSetSpeed(speed);
        }
        EscSetBrake(1U);
        EscSetSpeed(0);
        busy_delay(5000000U);
        EscSetBrake(0U);

        for (int speed = 0; speed >= -100; speed--)
        {
            busy_delay(500000U);
            EscSetSpeed(speed);
        }
        EscSetBrake(1U);
        EscSetSpeed(0);
        busy_delay(5000000U);
        EscSetBrake(0U);
    }
}

#if APP_TEST_FINAL_DUMMY
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
    RgbLed_ChangeColor((RgbLed_Color){ .r=false, .g=false, .b=true });
}

static void StatusLed_Green(void)
{
    RgbLed_ChangeColor((RgbLed_Color){ .r=false, .g=true, .b=false });
}

/* Neutral stop for ESC: do NOT use brake unless you are 100% sure the ESC driver/ESC supports it.
   This avoids the “full reverse/full blast on braking” hazard noted in esc.c.

   I Don't really know what the thing above means but I will test later on*/
static void Esc_StopNeutral(void)
{
    EscSetBrake(0U);
    EscSetSpeed(0);
}

/* =========================================================
   WORKING ESC-ONLY logic (used inside FINAL_DUMMY)
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

    /* Init + arm exactly like working mode */
    EscInit(ESC_PWM_CH, ESC_DUTY_MIN, ESC_DUTY_MED, ESC_DUTY_MAX);

    EscSetBrake(0U);
    EscSetSpeed(0);

    {
        uint32 t0 = Timebase_GetMs();
        while ((uint32)(Timebase_GetMs() - t0) < ESC_ARM_TIME_MS)
        {
            EscSetBrake(0U);
            EscSetSpeed(0);
        }
    }

    /* Default STOP */
    Esc_StopNeutral();
}

static void esc_update(EscRunState_t *st, uint32 nowMs, boolean sw2, boolean sw3, uint8 pot)
{
    /* SW3 = hard stop (same as working mode) */
    if (sw3)
    {
        st->mode = CAR_IDLE;
        Esc_StopNeutral();
        return;
    }

    /* SW2 starts from IDLE (same pattern) */
    if ((st->mode == CAR_IDLE) && sw2)
    {
        st->mode = CAR_ARMING;
        st->startGoMs = nowMs + START_DELAY_MS;

        EscSetSpeed(0);
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
        /* POT -> -100..+100 FULL RANGE */
        sint32 cmdPct;

        if (pot <= (uint8)POT_CENTER_RAW)
        {
            uint16 denom = (uint16)(POT_CENTER_RAW - POT_LEFT_RAW);
            if (denom == 0u) denom = 1u;
            cmdPct = -((sint32)((uint8)POT_CENTER_RAW - pot) * 100) / (sint32)denom;
        }
        else
        {
            uint16 denom = (uint16)(POT_RIGHT_RAW - POT_CENTER_RAW);
            if (denom == 0u) denom = 1u;
            cmdPct = ((sint32)(pot - (uint8)POT_CENTER_RAW) * 100) / (sint32)denom;
        }

        if (cmdPct < (sint32)MOTOR_DEADBAND_PCT && cmdPct > -((sint32)MOTOR_DEADBAND_PCT))
        {
            cmdPct = 0;
        }

        EscSetBrake(0U);
        EscSetSpeed((int)cmdPct);
    }
}

/* =========================================================
   WORKING CAMERA+SERVO V2 logic (used inside FINAL_DUMMY)
========================================================= */
typedef struct
{
    VisionDebug_State_t vdbg;
    SteeringLinearState_t ctrl;

    LinearCameraFrame processedFrame;
    VisionLinear_ResultType result;

    VisionLinear_DebugOut_t dbg;
    uint16 smoothBuf[VISION_LINEAR_BUFFER_SIZE];
    boolean haveValidVision;

    sint16 steerRaw;
    sint16 steerFilt;
    sint16 steerOut;

    uint32 nextTickMs;
    uint32 nextSteerMs;
    uint32 tickCount;
} CamServoState_t;

static sint16 abs_s16(sint16 x) { return (x < 0) ? (sint16)(-x) : x; }
static sint16 clamp_s16(sint16 x, sint16 lo, sint16 hi)
{
    if (x < lo) return lo;
    if (x > hi) return hi;
    return x;
}
static sint16 iir_s16(sint16 y_prev, sint16 x, float alpha)
{
    float y = (float)y_prev + alpha * ((float)x - (float)y_prev);
    if (y >  32767.0f) y =  32767.0f;
    if (y < -32768.0f) y = -32768.0f;
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
    (void)memset(st->smoothBuf, 0, sizeof(st->smoothBuf));

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
    const uint32 LOOP_MS    = (uint32)V2_LOOP_PERIOD_MS;
    const uint32 DISP_MS    = (uint32)DISPLAY_PERIOD_MS;
    const uint32 DISP_TICKS = (DISP_MS / LOOP_MS);

    if ((uint32)(nowMs - st->nextTickMs) < LOOP_MS)
    {
        return;
    }
    st->nextTickMs += LOOP_MS;
    st->tickCount++;

    /* SW2 allowed, SW3 NOT passed here (reserved for switching modes) */
    VisionDebug_OnTick(&st->vdbg, sw2, FALSE);

    boolean doDisplay = ((DISP_TICKS != 0U) && ((st->tickCount % DISP_TICKS) == 0U));

    {
        const LinearCameraFrame *latestFrame = (const LinearCameraFrame*)0;

        if (LinearCameraGetLatestFrame(&latestFrame) == TRUE)
        {
            st->dbg.mask = (uint32)VLIN_DBG_NONE;
            st->dbg.smoothOut = (uint16*)0;

            if ((doDisplay == TRUE) && (VisionDebug_WantsVisionDebugData(&st->vdbg) == TRUE))
            {
                VisionDebug_PrepareVisionDbg(&st->vdbg, &st->dbg, st->smoothBuf);
            }

            (void)memcpy(st->processedFrame.Values,
                         latestFrame->Values,
                         sizeof(st->processedFrame.Values));
            VisionLinear_ProcessFrameEx(st->processedFrame.Values, &st->result, &st->dbg);
            st->haveValidVision = TRUE;
        }
    }

    if (time_reached(nowMs, st->nextSteerMs))
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

        const float dt = ((float)STEER_UPDATE_MS) * 0.001f;
        const uint8 fakeSpeed = 20U;

        SteeringOutput_t out = SteeringLinear_UpdateV2(&st->ctrl, &st->result, dt, fakeSpeed);
        st->steerRaw = (sint16)out.steer_cmd;

        if (abs_s16(st->steerRaw) <= 2) { st->steerRaw = 0; }

        st->steerFilt = iir_s16(st->steerFilt, st->steerRaw, 0.45f);

        {
            const sint16 STEER_RATE_MAX = 8;
            sint16 delta = (sint16)(st->steerFilt - st->steerOut);
            delta = clamp_s16(delta, (sint16)(-STEER_RATE_MAX), (sint16)(+STEER_RATE_MAX));
            st->steerOut = (sint16)(st->steerOut + delta);
        }

        st->steerOut = clamp_s16(st->steerOut,
                                 (sint16)(-STEER_CMD_CLAMP),
                                 (sint16)(+STEER_CMD_CLAMP));

        Steer((int)st->steerOut);
    }

    if ((doDisplay == TRUE) && (st->haveValidVision == TRUE))
    {
        const uint16 *pSmooth = (st->dbg.smoothOut != (uint16*)0) ? st->smoothBuf : (const uint16*)0;
        const VisionLinear_DebugOut_t *pDbg =
            (st->dbg.mask != (uint32)VLIN_DBG_NONE) ? &st->dbg : (const VisionLinear_DebugOut_t*)0;

        VisionDebug_Draw(&st->vdbg, st->processedFrame.Values, pSmooth, &st->result, pDbg);
    }
}

/* =========================================================
   FINAL DUMMY
   - SW3 switches ESC <-> CAM+SERVO
   - POT mode: FULL RANGE manual (-100..+100)
   - CAM mode: explicit forward speed command capped to FULL_AUTO_SPEED_PCT with ramp
========================================================= */
#if APP_TEST_FINAL_DUMMY
static void mode_final_dummy(void)
{
    typedef enum { DUMMY_ESC = 0, DUMMY_CAM = 1 } DummyState_t;
    DummyState_t state = DUMMY_ESC;

    /* FULL AUTO (CAM mode) motor command */
    sint32 autoSpeedPct = 0;
    uint32 nextAutoSpeedMs = 0u;

    Board_InitDrivers();
    Timebase_Init();
    OnboardPot_Init();

    display_power_stabilize_delay();
    DisplayInit(0U, STD_ON);

    /* init states */
    EscRunState_t escSt;
    CamServoState_t camSt;

    /* start in ESC mode */
    esc_enter(&escSt);
    StatusLed_Blue();

    for (;;)
    {
        uint32 now = Timebase_GetMs();

        Buttons_Update();
        boolean sw2 = Buttons_WasPressed(BUTTON_ID_SW2);
        boolean sw3 = Buttons_WasPressed(BUTTON_ID_SW3);
        uint8 pot   = OnboardPot_ReadLevelFiltered();

        /* SW3 switches which “working module” is active */
        if (sw3)
        {
            if (state == DUMMY_ESC)
            {
                /* stop ESC safely before switching */
                Esc_StopNeutral();

                camservo_enter(&camSt, now);
                state = DUMMY_CAM;
                StatusLed_Green();

                /* Start auto speed ramp from 0 */
                autoSpeedPct = 0;
                nextAutoSpeedMs = now;
                EscSetBrake(0U);
                EscSetSpeed(0);
            }
            else
            {
                /* stop steering */
                SteerStraight();
                LinearCameraStopStream();

                esc_enter(&escSt);
                state = DUMMY_ESC;
                StatusLed_Blue();

                autoSpeedPct = 0;
                nextAutoSpeedMs = 0u;
            }

            /* do not also treat SW3 as ESC stop in the same tick */
            sw3 = FALSE;
        }

        if (state == DUMMY_ESC)
        {
            esc_update(&escSt, now, sw2, sw3, pot);
        }
        else
        {
            camservo_update(&camSt, now, sw2);

            /* Motor command in CAM mode (full auto): ramp up to capped forward speed */
            if (time_reached(now, nextAutoSpeedMs))
            {
                nextAutoSpeedMs = now + FULL_AUTO_RAMP_PERIOD_MS;

                if (autoSpeedPct < (sint32)FULL_AUTO_SPEED_PCT)
                {
                    autoSpeedPct += (sint32)FULL_AUTO_RAMP_STEP_PCT;
                    if (autoSpeedPct > (sint32)FULL_AUTO_SPEED_PCT)
                    {
                        autoSpeedPct = (sint32)FULL_AUTO_SPEED_PCT;
                    }
                }

                /* forward-only for now */
                if (autoSpeedPct < 0) autoSpeedPct = 0;
                if (autoSpeedPct > 100) autoSpeedPct = 100;

                EscSetBrake(0U);
        // flipped: SW3 mode forward direction (If the - sign removed = direction change )
                EscSetSpeed((int)(-autoSpeedPct));

            }
        }
    }
}
#endif
#endif

static void mode_linear_camera_test(void)
{
    VisionDebug_State_t vdbg;
    static LinearCameraFrame processedFrame;
    VisionLinear_ResultType result;
    VisionLinear_DebugOut_t dbg;
    static uint16 smoothBuf[VISION_LINEAR_BUFFER_SIZE];
    const uint32 loopPeriodMs = 5U;
    const uint32 displayPeriodMs = 20U;
    const uint32 displayTicks = (displayPeriodMs / loopPeriodMs);
    uint32 nextTickMs;
    uint32 tickCount = 0U;
    boolean haveValidVision = FALSE;

    Board_InitCommonApp();
    VisionLinear_InitV2();
    VisionDebug_Init(&vdbg, 3200U);

    nextTickMs = Timebase_GetMs();

    (void)LinearCameraStartStream();

    for (;;)
    {
        uint32 nowMs = Timebase_GetMs();

        if ((uint32)(nowMs - nextTickMs) < loopPeriodMs)
        {
            continue;
        }

        nextTickMs += loopPeriodMs;
        tickCount++;

        Buttons_Update();

        {
            boolean screenTogglePressed = Buttons_WasPressed(BUTTON_ID_SW2);
            boolean modeNextPressed = Buttons_WasPressed(BUTTON_ID_SW3);

            VisionDebug_OnTick(&vdbg, screenTogglePressed, modeNextPressed);
        }

        RgbLed_ChangeColor((RgbLed_Color){ .r=true, .g=false, .b=false });

        {
            const LinearCameraFrame *latestFrame = (const LinearCameraFrame*)0;

            if (LinearCameraGetLatestFrame(&latestFrame) == TRUE)
            {
                dbg.mask = (uint32)VLIN_DBG_NONE;
                dbg.smoothOut = (uint16*)0;

                if (VisionDebug_WantsVisionDebugData(&vdbg) == TRUE)
                {
                    VisionDebug_PrepareVisionDbg(&vdbg, &dbg, smoothBuf);
                }

                RgbLed_ChangeColor((RgbLed_Color){ .r=false, .g=false, .b=true });

                (void)memcpy(processedFrame.Values,
                             latestFrame->Values,
                             sizeof(processedFrame.Values));
                VisionLinear_ProcessFrameEx(processedFrame.Values, &result, &dbg);

                haveValidVision = TRUE;
            }
        }

        if ((displayTicks != 0U) &&
            ((tickCount % displayTicks) == 0U) &&
            (haveValidVision == TRUE))
        {
            const uint16 *pSmooth =
                (dbg.smoothOut != (uint16*)0) ? smoothBuf : (const uint16*)0;

            const VisionLinear_DebugOut_t *pDbg =
                (dbg.mask != (uint32)VLIN_DBG_NONE) ? &dbg : (const VisionLinear_DebugOut_t*)0;

            VisionDebug_Draw(&vdbg, processedFrame.Values, pSmooth, &result, pDbg);
        }

        RgbLed_ChangeColor((RgbLed_Color){ .r=false, .g=false, .b=false });
    }
}

/* =========================================================
   Dispatcher
========================================================= */
void App_RunSelectedMode(void)
{
#if APP_TEST_FINAL_DUMMY
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
