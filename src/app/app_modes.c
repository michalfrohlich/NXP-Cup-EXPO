#include "app_modes.h"
#include "main_types.h"
#include "car_config.h"

/* ===== Hardware ===== */
#include "main_functions.h"
#include "timebase.h"
#include "buttons.h"
#include "servo.h"
#include "esc.h"
#include "linear_camera.h"
#include "display.h"

/* ===== Vision V2 + Debug ===== */
#include "../services/vision_linear_v2.h"
#include "rgb_led.h"
#include "vision_debug.h"

/* ===== PID (Vision V2 Error) ===== */
#include "../services/steering_control_linear.h"

/* =========================================================
   Helpers
========================================================= */
static boolean time_reached(uint32 nowMs, uint32 dueMs)
{
    return ((uint32)(nowMs - dueMs) < 0x80000000u) ? TRUE : FALSE;
}

static sint16 clamp_s16(sint16 x, sint16 lo, sint16 hi)
{
    if (x < lo) return lo;
    if (x > hi) return hi;
    return x;
}
static sint16 abs_s16(sint16 x) { return (x < 0) ? (sint16)(-x) : x; }

static sint16 iir_s16(sint16 y_prev, sint16 x, float alpha)
{
    float y = (float)y_prev + alpha * ((float)x - (float)y_prev);
    if (y >  32767.0f) y =  32767.0f;
    if (y < -32768.0f) y = -32768.0f;
    return (sint16)(y);
}

/* safe stop for full-car mode */
static void fullcar_stop(void)
{
    EscSetSpeed(0);
    EscSetBrake(1U);
    SteerStraight();
}

/* =========================================================
   MODE 1: CAMERA ONLY (Vision V2 Debug) - unchanged
========================================================= */
static void mode_vision_v2_debug(void)
{
    DriversInit();
    Timebase_Init();
    DisplayInit(0U, STD_ON);

    LinearCameraInit(CAM_CLK_PWM_CH, CAM_SHUTTER_GPT_CH, CAM_ADC_GROUP, CAM_SHUTTER_PCR);
    VisionLinear_InitV2();

    VisionDebug_State_t vdbg;
    VisionDebug_Init(&vdbg, (uint8)V2_WHITE_SAT_U8);

    LinearCameraFrame frame;
    VisionLinear_ResultType result;

    VisionLinear_DebugOut_t dbg;
    static uint8 smoothBuf[VISION_LINEAR_BUFFER_SIZE];

    uint32 nextTickMs = Timebase_GetMs();
    uint32 tickCount  = 0U;

    const uint32 LOOP_MS    = (uint32)V2_LOOP_PERIOD_MS;
    const uint32 DISP_MS    = (uint32)DISPLAY_PERIOD_MS;
    const uint32 DISP_TICKS = (DISP_MS / LOOP_MS);

    for (;;)
    {
        uint32 nowMs = Timebase_GetMs();
        if ((uint32)(nowMs - nextTickMs) < LOOP_MS) { continue; }
        nextTickMs += LOOP_MS;
        tickCount++;

        Buttons_Update();

        boolean screenTogglePressed = Buttons_WasPressed(BUTTON_ID_SW2);
        boolean modeNextPressed     = Buttons_WasPressed(BUTTON_ID_SW3);

        VisionDebug_OnTick(&vdbg, screenTogglePressed, modeNextPressed);

        RgbLed_ChangeColor((RgbLed_Color){ .r=true, .g=false, .b=false });
        LinearCameraGetFrameEx(&frame, (uint32)V2_TEST_EXPOSURE_TICKS);

        boolean doDisplay = ((DISP_TICKS != 0U) && ((tickCount % DISP_TICKS) == 0U));

        dbg.mask      = 0u;
        dbg.smoothOut = (uint8*)0;

        if ((doDisplay == TRUE) && (VisionDebug_WantsVisionDebugData(&vdbg) == TRUE))
        {
            VisionDebug_PrepareVisionDbg(&vdbg, &dbg, smoothBuf);
        }

        RgbLed_ChangeColor((RgbLed_Color){ .r=false, .g=false, .b=true });
        VisionLinear_ProcessFrameEx(frame.Values, &result, &dbg);

        if (doDisplay == TRUE)
        {
            const uint8 *pSmooth = (dbg.smoothOut != (uint8*)0) ? smoothBuf : (const uint8*)0;
            const VisionLinear_DebugOut_t *pDbg = (dbg.mask != 0u) ? &dbg : (const VisionLinear_DebugOut_t*)0;
            VisionDebug_Draw(&vdbg, frame.Values, pSmooth, &result, pDbg);
        }

        RgbLed_ChangeColor((RgbLed_Color){ .r=false, .g=false, .b=false });
    }
}

/* =========================================================
   MODE 2: CAMERA + SERVO TEST (NO motor)
   ---------------------------------------------------------
   REQUIREMENT:
   - Display MUST be VisionDebug fully (same as camera-only)
   - Buttons MUST be VisionDebug logic (SW2 screen toggle, SW3 mode next)
   - Ignore servo values on display (so no telemetry screen)
   - Servo still runs PID in background.
========================================================= */
static void mode_camera_servo_v2(void)
{
    DriversInit();
    Timebase_Init();
    DisplayInit(0U, STD_ON);

    /* Servo */
    ServoInit(SERVO_PWM_CH, SERVO_DUTY_MAX, SERVO_DUTY_MIN, SERVO_DUTY_MED);
    SteerStraight();

    /* Camera + vision */
    LinearCameraInit(CAM_CLK_PWM_CH, CAM_SHUTTER_GPT_CH, CAM_ADC_GROUP, CAM_SHUTTER_PCR);
    VisionLinear_InitV2();

    VisionDebug_State_t vdbg;
    VisionDebug_Init(&vdbg, (uint8)V2_WHITE_SAT_U8);

    SteeringLinearState_t ctrl;
    SteeringLinear_Init(&ctrl);
    SteeringLinear_Reset(&ctrl);

    LinearCameraFrame frame;
    VisionLinear_ResultType result;

    VisionLinear_DebugOut_t dbg;
    static uint8 smoothBuf[VISION_LINEAR_BUFFER_SIZE];

    /* extra shaping */
    sint16 steerRaw = 0;
    sint16 steerFilt = 0;
    sint16 steerOut = 0;
    const sint16 STEER_DEADBAND = 2;
    const sint16 STEER_RATE_MAX = 8; /* slightly more decisive */

    uint32 nextTickMs  = Timebase_GetMs();
    uint32 nextSteerMs = nextTickMs + STEER_UPDATE_MS;
    uint32 tickCount   = 0U;

    const uint32 LOOP_MS    = (uint32)V2_LOOP_PERIOD_MS;
    const uint32 DISP_MS    = (uint32)DISPLAY_PERIOD_MS;
    const uint32 DISP_TICKS = (DISP_MS / LOOP_MS);

    for (;;)
    {
        uint32 nowMs = Timebase_GetMs();
        if ((uint32)(nowMs - nextTickMs) < LOOP_MS) { continue; }
        nextTickMs += LOOP_MS;
        tickCount++;

        Buttons_Update();

        /* EXACT same button meaning as camera-only debug */
        boolean screenTogglePressed = Buttons_WasPressed(BUTTON_ID_SW2);
        boolean modeNextPressed     = Buttons_WasPressed(BUTTON_ID_SW3);
        VisionDebug_OnTick(&vdbg, screenTogglePressed, modeNextPressed);

        RgbLed_ChangeColor((RgbLed_Color){ .r=true, .g=false, .b=false });
        LinearCameraGetFrameEx(&frame, (uint32)V2_TEST_EXPOSURE_TICKS);

        boolean doDisplay = ((DISP_TICKS != 0U) && ((tickCount % DISP_TICKS) == 0U));

        dbg.mask      = 0u;
        dbg.smoothOut = (uint8*)0;

        if ((doDisplay == TRUE) && (VisionDebug_WantsVisionDebugData(&vdbg) == TRUE))
        {
            VisionDebug_PrepareVisionDbg(&vdbg, &dbg, smoothBuf);
        }

        RgbLed_ChangeColor((RgbLed_Color){ .r=false, .g=false, .b=true });
        VisionLinear_ProcessFrameEx(frame.Values, &result, &dbg);

        /* Servo update (independent timer) */
        if (time_reached(nowMs, nextSteerMs))
        {
            nextSteerMs += STEER_UPDATE_MS;

            const float dt = ((float)STEER_UPDATE_MS) * 0.001f;
            const uint8 fakeSpeed = 20U;

            SteeringOutput_t out = SteeringLinear_UpdateV2(&ctrl, &result, dt, fakeSpeed);
            steerRaw = (sint16)out.steer_cmd;

            if (abs_s16(steerRaw) <= STEER_DEADBAND) { steerRaw = 0; }

            /* extra smooth but still decisive */
            steerFilt = iir_s16(steerFilt, steerRaw, 0.45f);

            {
                sint16 delta = (sint16)(steerFilt - steerOut);
                delta = clamp_s16(delta, (sint16)(-STEER_RATE_MAX), (sint16)(+STEER_RATE_MAX));
                steerOut = (sint16)(steerOut + delta);
            }

            steerOut = clamp_s16(steerOut, (sint16)(-STEER_CMD_CLAMP), (sint16)(+STEER_CMD_CLAMP));
            Steer((int)steerOut);
        }

        if (doDisplay == TRUE)
        {
            const uint8 *pSmooth = (dbg.smoothOut != (uint8*)0) ? smoothBuf : (const uint8*)0;
            const VisionLinear_DebugOut_t *pDbg = (dbg.mask != 0u) ? &dbg : (const VisionLinear_DebugOut_t*)0;
            VisionDebug_Draw(&vdbg, frame.Values, pSmooth, &result, pDbg);
        }

        RgbLed_ChangeColor((RgbLed_Color){ .r=false, .g=false, .b=false });
    }
}

/* =========================================================
   MODE 3: FULL CAR (ESC + SERVO + CAMERA)
   ---------------------------------------------------------
   REQUIREMENT:
   - SW2 toggles IDLE <-> RUN
   - SW3 toggles between BOTH camera modes (VisionDebug modeNext)
   - Display uses VisionDebug fully

   Behavior:
   - In IDLE: motor braked, servo centered
   - In RUN: PID steering + throttle policy
========================================================= */
static void mode_full_car_v2(void)
{
    DriversInit();
    Timebase_Init();
    DisplayInit(0U, STD_ON);

    /* Servo */
    ServoInit(SERVO_PWM_CH, SERVO_DUTY_MAX, SERVO_DUTY_MIN, SERVO_DUTY_MED);
    SteerStraight();

    /* ESC */
    EscInit(ESC_PWM_CH, ESC_DUTY_MIN, ESC_DUTY_MED, ESC_DUTY_MAX);
    EscSetBrake(1U);
    EscSetSpeed(0);

    /* arm ESC (neutral) */
    {
        uint32 t0 = Timebase_GetMs();
        while ((uint32)(Timebase_GetMs() - t0) < ESC_ARM_TIME_MS)
        {
            EscSetBrake(1U);
            EscSetSpeed(0);
        }
    }

    /* Camera + vision */
    LinearCameraInit(CAM_CLK_PWM_CH, CAM_SHUTTER_GPT_CH, CAM_ADC_GROUP, CAM_SHUTTER_PCR);
    VisionLinear_InitV2();

    VisionDebug_State_t vdbg;
    VisionDebug_Init(&vdbg, (uint8)V2_WHITE_SAT_U8);

    SteeringLinearState_t ctrl;
    SteeringLinear_Init(&ctrl);
    SteeringLinear_Reset(&ctrl);

    LinearCameraFrame frame;
    VisionLinear_ResultType result;

    VisionLinear_DebugOut_t dbg;
    static uint8 smoothBuf[VISION_LINEAR_BUFFER_SIZE];

    /* extra shaping */
    sint16 steerRaw = 0;
    sint16 steerFilt = 0;
    sint16 steerOut = 0;
    const sint16 STEER_DEADBAND = 2;
    const sint16 STEER_RATE_MAX = 10;

    /* state */
    boolean running = FALSE;

    uint32 nextTickMs  = Timebase_GetMs();
    uint32 nextSteerMs = nextTickMs + STEER_UPDATE_MS;
    uint32 tickCount   = 0U;

    const uint32 LOOP_MS    = (uint32)V2_LOOP_PERIOD_MS;
    const uint32 DISP_MS    = (uint32)DISPLAY_PERIOD_MS;
    const uint32 DISP_TICKS = (DISP_MS / LOOP_MS);

    fullcar_stop();

    for (;;)
    {
        uint32 nowMs = Timebase_GetMs();
        if ((uint32)(nowMs - nextTickMs) < LOOP_MS) { continue; }
        nextTickMs += LOOP_MS;
        tickCount++;

        Buttons_Update();

        /* SW2 = RUN toggle (your requirement) */
        if (Buttons_WasPressed(BUTTON_ID_SW2))
        {
            running = (running == TRUE) ? FALSE : TRUE;
            if (running == FALSE)
            {
                fullcar_stop();
            }
            else
            {
                SteeringLinear_Reset(&ctrl);
                steerRaw = steerFilt = steerOut = 0;
            }
        }

        /* SW3 = camera mode toggle (your requirement) */
        {
            boolean modeNextPressed = Buttons_WasPressed(BUTTON_ID_SW3);
            /* keep debug screen enabled always; we only change modes */
            VisionDebug_OnTick(&vdbg, FALSE, modeNextPressed);
        }

        RgbLed_ChangeColor((RgbLed_Color){ .r=true, .g=false, .b=false });
        LinearCameraGetFrameEx(&frame, (uint32)V2_TEST_EXPOSURE_TICKS);

        boolean doDisplay = ((DISP_TICKS != 0U) && ((tickCount % DISP_TICKS) == 0U));

        dbg.mask      = 0u;
        dbg.smoothOut = (uint8*)0;

        if ((doDisplay == TRUE) && (VisionDebug_WantsVisionDebugData(&vdbg) == TRUE))
        {
            VisionDebug_PrepareVisionDbg(&vdbg, &dbg, smoothBuf);
        }

        RgbLed_ChangeColor((RgbLed_Color){ .r=false, .g=false, .b=true });
        VisionLinear_ProcessFrameEx(frame.Values, &result, &dbg);

        /* Control loop */
        if (time_reached(nowMs, nextSteerMs))
        {
            nextSteerMs += STEER_UPDATE_MS;

            if (running == FALSE)
            {
                fullcar_stop();
            }
            else
            {
                const float dt = ((float)STEER_UPDATE_MS) * 0.001f;

                /* base speed policy */
                uint8 baseSpeed =
#if FULLMODE_FORCE_SLOW_SPEED
                    (uint8)FULLMODE_SLOW_SPEED_PCT;
#else
                    (uint8)SPEED_MIN;
#endif

                SteeringOutput_t out = SteeringLinear_UpdateV2(&ctrl, &result, dt, baseSpeed);

                /* motor */
                if (out.brake)
                {
                    EscSetSpeed(0);
                    EscSetBrake(1U);
                }
                else
                {
                    EscSetBrake(0U);
                    EscSetSpeed((int)out.throttle_pct);
                }

                /* steering + shaping */
                steerRaw = (sint16)out.steer_cmd;
                if (abs_s16(steerRaw) <= STEER_DEADBAND) { steerRaw = 0; }

                steerFilt = iir_s16(steerFilt, steerRaw, 0.45f);

                {
                    sint16 delta = (sint16)(steerFilt - steerOut);
                    delta = clamp_s16(delta, (sint16)(-STEER_RATE_MAX), (sint16)(+STEER_RATE_MAX));
                    steerOut = (sint16)(steerOut + delta);
                }

                steerOut = clamp_s16(steerOut, (sint16)(-STEER_CMD_CLAMP), (sint16)(+STEER_CMD_CLAMP));
                Steer((int)steerOut);
            }
        }

        /* Display always VisionDebug */
        if (doDisplay == TRUE)
        {
            const uint8 *pSmooth = (dbg.smoothOut != (uint8*)0) ? smoothBuf : (const uint8*)0;
            const VisionLinear_DebugOut_t *pDbg = (dbg.mask != 0u) ? &dbg : (const VisionLinear_DebugOut_t*)0;
            VisionDebug_Draw(&vdbg, frame.Values, pSmooth, &result, pDbg);
        }

        RgbLed_ChangeColor((RgbLed_Color){ .r=false, .g=false, .b=false });
    }
}

/* =========================================================
   MAIN DISPATCHER
========================================================= */
void App_RunSelectedMode(void)
{
#if APP_TEST_VISION_V2_DEBUG
    mode_vision_v2_debug();

#elif APP_TEST_CAMERA_SERVO_V2
    mode_camera_servo_v2();

#elif APP_TEST_FULL_CAR_V2
    mode_full_car_v2();

#else
    DriversInit();
    Timebase_Init();
    while (1) { }
#endif
}
