#include "app_modes.h"
#include "main_types.h"

/* FIX: app_modes.c is in app/, car_config.h is also in app/ */
#include "car_config.h"

/* ===== Hardware ===== */
#include "main_functions.h"
#include "timebase.h"
#include "buttons.h"
#include "onboard_pot.h"
#include "servo.h"
#include "linear_camera.h"
#include "display.h"

/* FIX: vision header lives in services/ */
#include "../services/vision_linear_v2.h"

/* V2 debug UI */
#include "rgb_led.h"
#include "vision_debug.h"

/* PID controller lives in services/ */
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

/* extra smoothing on final servo command */
static sint16 iir_s16(sint16 y_prev, sint16 x, float alpha)
{
    float y = (float)y_prev + alpha * ((float)x - (float)y_prev);
    if (y >  32767.0f) y =  32767.0f;
    if (y < -32768.0f) y = -32768.0f;
    return (sint16)(y);
}

/* =========================================================
   MODE: CAMERA ONLY (VISION V2 DEBUG)  [KNOWN WORKING]
========================================================= */
static void mode_vision_v2_debug(void)
{
    DriversInit();
    Timebase_Init();
    OnboardPot_Init();
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
   MODE: CAMERA + SERVO (VISION V2 + PID)
   ---------------------------------------------------------
   Buttons:
     SW2 => DEBUG VIEW (camera visualization)
     SW3 => TELEMETRY VIEW (numbers)

   UNIT KEY:
     - result.Error is normalized [-1..+1]
     - LCD shows E(m) = Error*1000  (so -1000..+1000 fits screen)
     - steer_cmd is servo command units [-100..+100]
========================================================= */
static void mode_camera_servo_v2(void)
{
    DriversInit();
    Timebase_Init();
    OnboardPot_Init();
    DisplayInit(0U, STD_ON);

    /* Servo */
    ServoInit(SERVO_PWM_CH, SERVO_DUTY_MAX, SERVO_DUTY_MIN, SERVO_DUTY_MED);
    SteerStraight();

    /* Camera + vision v2 */
    LinearCameraInit(CAM_CLK_PWM_CH, CAM_SHUTTER_GPT_CH, CAM_ADC_GROUP, CAM_SHUTTER_PCR);
    VisionLinear_InitV2();

    VisionDebug_State_t vdbg;
    VisionDebug_Init(&vdbg, (uint8)V2_WHITE_SAT_U8);

    /* PID */
    SteeringLinearState_t ctrl;
    SteeringLinear_Init(&ctrl);
    SteeringLinear_Reset(&ctrl);

    LinearCameraFrame frame;
    VisionLinear_ResultType result;

    VisionLinear_DebugOut_t dbg;
    static uint8 smoothBuf[VISION_LINEAR_BUFFER_SIZE];

    /* extra servo shaping */
    sint16 steerRaw = 0;     /* raw PID output (-100..+100) */
    sint16 steerFilt = 0;    /* extra filtered */
    sint16 steerOut = 0;     /* rate-limited final command */

    const sint16 STEER_DEADBAND = 2;
    const sint16 STEER_RATE_MAX = 6;

    uint8 screen = 0; /* 0=DEBUG, 1=TELEMETRY */

    uint32 now = Timebase_GetMs();
    uint32 nextButtonsMs = now + BUTTONS_PERIOD_MS;
    uint32 nextCamMs     = now + CAMERA_PERIOD_MS;
    uint32 nextSteerMs   = now + STEER_UPDATE_MS;
    uint32 nextDispMs    = now + DISPLAY_PERIOD_MS;

    for (;;)
    {
        now = Timebase_GetMs();

        if (time_reached(now, nextButtonsMs))
        {
            nextButtonsMs += BUTTONS_PERIOD_MS;
            Buttons_Update();

            if (Buttons_WasPressed(BUTTON_ID_SW2)) { screen = 0; }
            if (Buttons_WasPressed(BUTTON_ID_SW3)) { screen = 1; }
        }

        if (time_reached(now, nextCamMs))
        {
            nextCamMs += CAMERA_PERIOD_MS;

            LinearCameraGetFrameEx(&frame, (uint32)V2_TEST_EXPOSURE_TICKS);

            dbg.mask      = 0u;
            dbg.smoothOut = (uint8*)0;

            if (screen == 0)
            {
                VisionDebug_PrepareVisionDbg(&vdbg, &dbg, smoothBuf);
            }

            VisionLinear_ProcessFrameEx(frame.Values, &result, &dbg);
        }

        if (time_reached(now, nextSteerMs))
        {
            nextSteerMs += STEER_UPDATE_MS;

            const float dt = ((float)STEER_UPDATE_MS) * 0.001f;
            const uint8 fakeSpeed = 20U;

            SteeringOutput_t out = SteeringLinear_UpdateV2(&ctrl, &result, dt, fakeSpeed);
            steerRaw = (sint16)out.steer_cmd;

            if (abs_s16(steerRaw) <= STEER_DEADBAND) { steerRaw = 0; }

            steerFilt = iir_s16(steerFilt, steerRaw, 0.35f);

            {
                sint16 delta = (sint16)(steerFilt - steerOut);
                delta = clamp_s16(delta, (sint16)(-STEER_RATE_MAX), (sint16)(+STEER_RATE_MAX));
                steerOut = (sint16)(steerOut + delta);
            }

            steerOut = clamp_s16(steerOut, (sint16)(-STEER_CMD_CLAMP), (sint16)(+STEER_CMD_CLAMP));
            Steer((int)steerOut);
        }

        if (time_reached(now, nextDispMs))
        {
            nextDispMs += DISPLAY_PERIOD_MS;

            if (screen == 0)
            {
                const uint8 *pSmooth = (dbg.smoothOut != (uint8*)0) ? smoothBuf : (const uint8*)0;
                const VisionLinear_DebugOut_t *pDbg = (dbg.mask != 0u) ? &dbg : (const VisionLinear_DebugOut_t*)0;

                VisionDebug_Draw(&vdbg, frame.Values, pSmooth, &result, pDbg);
            }
            else
            {
                DisplayClear();
                DisplayText(0U, "CAM+SERVO V2", 12U, 0U);

                /* normalized error [-1..+1] -> milli units [-1000..+1000] */
                int err_milli = (int)(result.Error * 1000.0f);

                DisplayText(1U, "E(m):", 5U, 0U);
                DisplayValue(1U, err_milli, 6U, 5U);

                DisplayText(1U, "CF:", 3U, 12U);
                DisplayValue(1U, (int)result.Confidence, 3U, 15U);

                DisplayText(2U, "ST:", 3U, 0U);
                DisplayValue(2U, (int)steerOut, 4U, 3U);

                DisplayText(2U, "RW:", 3U, 8U);
                DisplayValue(2U, (int)steerRaw, 4U, 11U);

                DisplayText(3U, "S:", 2U, 0U);
                DisplayValue(3U, (int)result.Status, 2U, 2U);

                DisplayText(3U, "L:", 2U, 6U);
                DisplayValue(3U, (int)result.LeftLineIdx, 3U, 8U);

                DisplayText(3U, "R:", 2U, 12U);
                DisplayValue(3U, (int)result.RightLineIdx, 3U, 14U);

                DisplayRefresh();
            }
        }
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

#else
    DriversInit();
    Timebase_Init();
    while (1) { }
#endif
}
