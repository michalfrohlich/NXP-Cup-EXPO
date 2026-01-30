#include "app_modes.h"
#include "main_types.h"
#include "car_config.h"

/* Hardware */
#include "main_functions.h"
#include "timebase.h"
#include "buttons.h"
#include "onboard_pot.h"
#include "servo.h"
#include "esc.h"
#include "linear_camera.h"
#include "display.h"
#include "ultrasonic.h"

/* Vision V2 + debug */
#include "../services/vision_linear_v2.h"
#include "rgb_led.h"
#include "vision_debug.h"

/* PID */
#include "../services/steering_control_linear.h"

/* UI */
#include "user_interface.h"

/* =========================================================
   Braking state placeholder for UI
   ---------------------------------------------------------
   We don't know the enum labels in braking.h.
   But enum value 0 is always valid.
========================================================= */
#define UI_BRAKE_STATE_IDLE   ((Braking_StateType)0)

/* =========================================================
   Helpers
========================================================= */
static boolean time_reached(uint32 nowMs, uint32 dueMs)
{
    /* Handles uint32 wrap-around safely */
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
    /* Simple low-pass filter: y = y_prev + alpha*(x - y_prev) */
    float y = (float)y_prev + alpha * ((float)x - (float)y_prev);
    if (y >  32767.0f) y =  32767.0f;
    if (y < -32768.0f) y = -32768.0f;
    return (sint16)(y);
}

/* =========================================================
   Ultrasonic runtime enable (independent of mode)
   Toggle: press SW2 + SW3 in the same update cycle
========================================================= */
static boolean g_ultraEnabled = (ULTRASONIC_DEFAULT_ON ? TRUE : FALSE);

/* IMPORTANT:
   Buttons_WasPressed() is typically "edge detected" and can be consumed.
   So we toggle using the sampled sw2/sw3 values from the loop (read once). */
static void toggle_ultrasonic_if_both_pressed(boolean sw2, boolean sw3)
{
    if (sw2 && sw3)
    {
        g_ultraEnabled = (g_ultraEnabled == TRUE) ? FALSE : TRUE;
    }
}

/* =========================================================
   Full-car safe stop
========================================================= */
static void fullcar_stop(void)
{
    EscSetSpeed(0);
    EscSetBrake(1U);
    SteerStraight();
}

/* =========================================================
   RGB status LED for FULL CAR mode
   ---------------------------------------------------------
   Blue  = IDLE (stopped)
   Green = RUN  (car active)
   Red   = FAIL (line lost / forced brake / etc.)
========================================================= */
static void StatusLed_SetIdle(void)
{
    RgbLed_ChangeColor((RgbLed_Color){ .r=false, .g=false, .b=true });
}

static void StatusLed_SetRun(void)
{
    RgbLed_ChangeColor((RgbLed_Color){ .r=false, .g=true, .b=false });
}

static void StatusLed_SetFail(void)
{
    RgbLed_ChangeColor((RgbLed_Color){ .r=true, .g=false, .b=false });
}

/* =========================================================
   MODE: ESC ONLY (known working)
   - Arming: brake OFF + speed 0
   - SW2: run
   - SW3: FULL STOP (brake ON)
   - POT: speed
========================================================= */
static void mode_esc_only_working(void)
{
    DriversInit();
    Timebase_Init();
    OnboardPot_Init();

    DisplayInit(0U, STD_ON);
    DisplayClear();
    DisplayText(0U, "ESC ONLY", 8U, 0U);
    DisplayText(1U, "SW2 run", 7U, 0U);
    DisplayText(2U, "SW3 STOP", 8U, 0U);
    DisplayText(3U, "POT speed", 9U, 0U);
    DisplayRefresh();

    UI_Init();

    EscInit(ESC_PWM_CH, ESC_DUTY_MIN, ESC_DUTY_MED, ESC_DUTY_MAX);

    /* WORKING ARM: brake OFF */
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

    /* Default stopped after arming */
    EscSetSpeed(0);
    EscSetBrake(1U);

    CarMode_t mode = CAR_IDLE;
    uint32 startGoMs = 0u;

    for (;;)
    {
        uint32 now = Timebase_GetMs();

        Buttons_Update();

        /* Read inputs ONCE per loop (WasPressed is edge-based) */
        boolean sw2 = Buttons_WasPressed(BUTTON_ID_SW2);
        boolean sw3 = Buttons_WasPressed(BUTTON_ID_SW3);
        uint8  pot  = OnboardPot_ReadLevelFiltered();

        UI_Input(sw2, sw3, pot);

        toggle_ultrasonic_if_both_pressed(sw2, sw3);

        /* SW3 = FULL STOP */
        if (sw3)
        {
            mode = CAR_IDLE;
            EscSetSpeed(0);
            EscSetBrake(1U);
        }

        /* SW2 = start */
        if ((mode == CAR_IDLE) && sw2)
        {
            mode = CAR_ARMING;
            startGoMs = now + START_DELAY_MS;
            EscSetSpeed(0);
            EscSetBrake(0U);
        }

        if ((mode == CAR_ARMING) && time_reached(now, startGoMs))
        {
            mode = CAR_RUN;
        }

        if (mode != CAR_RUN)
        {
            EscSetSpeed(0);
            EscSetBrake(1U);
        }
        else
        {
            sint32 cmdPct;

            /* Map potentiometer position into -100..+100 throttle (%) */
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

            /* Deadband: ignore tiny pot movement around center */
            if (cmdPct < (sint32)MOTOR_DEADBAND_PCT && cmdPct > -((sint32)MOTOR_DEADBAND_PCT))
            {
                cmdPct = 0;
            }

            EscSetBrake(0U);
            EscSetSpeed((int)cmdPct);
        }

        if (g_ultraEnabled == TRUE)
        {
            Ultrasonic_Task();
        }

        UI_Task(Ultrasonic_GetStatus(),
                Ultrasonic_GetIrqCount(),
                Ultrasonic_GetLastHighTicks(),
                UI_BRAKE_STATE_IDLE,
                0.0f);
    }
}

/* =========================================================
   MODE: Vision V2 debug (camera only)
========================================================= */
static void mode_vision_v2_debug(void)
{
    DriversInit();
    Timebase_Init();
    DisplayInit(0U, STD_ON);

    UI_Init();

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

        /* Read inputs ONCE per loop (WasPressed is edge-based) */
        boolean sw2 = Buttons_WasPressed(BUTTON_ID_SW2);
        boolean sw3 = Buttons_WasPressed(BUTTON_ID_SW3);
        uint8  pot  = OnboardPot_ReadLevelFiltered(); /* UI only in this mode */

        UI_Input(sw2, sw3, pot);

        toggle_ultrasonic_if_both_pressed(sw2, sw3);

        boolean screenTogglePressed = sw2;
        boolean modeNextPressed     = sw3;
        VisionDebug_OnTick(&vdbg, screenTogglePressed, modeNextPressed);

        /* keep the camera debug LED behavior in camera-only mode */
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

        if (g_ultraEnabled == TRUE)
        {
            Ultrasonic_Task();
        }

        UI_Task(Ultrasonic_GetStatus(),
                Ultrasonic_GetIrqCount(),
                Ultrasonic_GetLastHighTicks(),
                UI_BRAKE_STATE_IDLE,
                0.0f);

        RgbLed_ChangeColor((RgbLed_Color){ .r=false, .g=false, .b=false });
    }
}

/* =========================================================
   MODE: Camera + Servo (no motor) - VisionDebug display
========================================================= */
static void mode_camera_servo_v2(void)
{
    DriversInit();
    Timebase_Init();
    DisplayInit(0U, STD_ON);

    UI_Init();

    ServoInit(SERVO_PWM_CH, SERVO_DUTY_MAX, SERVO_DUTY_MIN, SERVO_DUTY_MED);
    SteerStraight();

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

    sint16 steerRaw = 0;
    sint16 steerFilt = 0;
    sint16 steerOut = 0;

    const sint16 STEER_DEADBAND = 2;
    const sint16 STEER_RATE_MAX = 8;

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

        /* Read inputs ONCE per loop (WasPressed is edge-based) */
        boolean sw2 = Buttons_WasPressed(BUTTON_ID_SW2);
        boolean sw3 = Buttons_WasPressed(BUTTON_ID_SW3);
        uint8  pot  = OnboardPot_ReadLevelFiltered(); /* UI only in this mode */

        UI_Input(sw2, sw3, pot);

        toggle_ultrasonic_if_both_pressed(sw2, sw3);

        boolean screenTogglePressed = sw2;
        boolean modeNextPressed     = sw3;
        VisionDebug_OnTick(&vdbg, screenTogglePressed, modeNextPressed);

        /* camera-only debug LED behavior is fine here */
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

        if (time_reached(nowMs, nextSteerMs))
        {
            nextSteerMs += STEER_UPDATE_MS;

            const float dt = ((float)STEER_UPDATE_MS) * 0.001f;
            const uint8 fakeSpeed = 20U;

            SteeringOutput_t out = SteeringLinear_UpdateV2(&ctrl, &result, dt, fakeSpeed);
            steerRaw = (sint16)out.steer_cmd;

            if (abs_s16(steerRaw) <= STEER_DEADBAND) { steerRaw = 0; }

            steerFilt = iir_s16(steerFilt, steerRaw, 0.45f);

            {
                /* Rate limit steering changes so it doesn't snap instantly */
                sint16 delta = (sint16)(steerFilt - steerOut);
                delta = clamp_s16(delta, (sint16)(-STEER_RATE_MAX), (sint16)(+STEER_RATE_MAX));
                steerOut = (sint16)(steerOut + delta);
            }

            steerOut = clamp_s16(steerOut, (sint16)(-STEER_CMD_CLAMP), (sint16)(+STEER_CMD_CLAMP));
            Steer((int)steerOut);
        }

        if (doDisplay == TRUE)
        {
            /* pSmooth2 is the smoothed version of the camera line (if enabled in debug view) */
            const uint8 *pSmooth2 = (dbg.smoothOut != (uint8*)0) ? smoothBuf : (const uint8*)0;
            const VisionLinear_DebugOut_t *pDbg = (dbg.mask != 0u) ? &dbg : (const VisionLinear_DebugOut_t*)0;
            VisionDebug_Draw(&vdbg, frame.Values, pSmooth2, &result, pDbg);
        }

        if (g_ultraEnabled == TRUE)
        {
            Ultrasonic_Task();
        }

        UI_Task(Ultrasonic_GetStatus(),
                Ultrasonic_GetIrqCount(),
                Ultrasonic_GetLastHighTicks(),
                UI_BRAKE_STATE_IDLE,
                0.0f);

        RgbLed_ChangeColor((RgbLed_Color){ .r=false, .g=false, .b=false });
    }
}

/* =========================================================
   MODE: Full car
   - SW2 toggles IDLE/RUN
   - LED shows IDLE/RUN/FAIL
   - VisionDebug stays active on screen
========================================================= */
static void mode_full_car_v2(void)
{
    DriversInit();
    Timebase_Init();
    DisplayInit(0U, STD_ON);

    UI_Init();

    ServoInit(SERVO_PWM_CH, SERVO_DUTY_MAX, SERVO_DUTY_MIN, SERVO_DUTY_MED);
    SteerStraight();

    EscInit(ESC_PWM_CH, ESC_DUTY_MIN, ESC_DUTY_MED, ESC_DUTY_MAX);

    /* Arm: brake OFF */
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

    fullcar_stop();
    StatusLed_SetIdle();

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

    sint16 steerRaw = 0;
    sint16 steerFilt = 0;
    sint16 steerOut = 0;

    const sint16 STEER_DEADBAND = 2;
    const sint16 STEER_RATE_MAX = 10;

    boolean running = FALSE;
    boolean fail = FALSE;

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

        /* Read inputs ONCE per loop (WasPressed is edge-based) */
        boolean sw2 = Buttons_WasPressed(BUTTON_ID_SW2);
        boolean sw3 = Buttons_WasPressed(BUTTON_ID_SW3);
        uint8  pot  = OnboardPot_ReadLevelFiltered(); /* UI only in this mode */

        UI_Input(sw2, sw3, pot);

        toggle_ultrasonic_if_both_pressed(sw2, sw3);

        /* SW2 start/stop */
        if (sw2)
        {
            running = (running == TRUE) ? FALSE : TRUE;
            fail = FALSE; /* reset fail when toggling */

            if (running == FALSE)
            {
                fullcar_stop();
                StatusLed_SetIdle();
            }
            else
            {
                SteeringLinear_Reset(&ctrl);
                steerRaw = steerFilt = steerOut = 0;
                EscSetBrake(0U);
                StatusLed_SetRun();
            }
        }

        /* SW3 camera debug mode switch */
        {
            VisionDebug_OnTick(&vdbg, FALSE, sw3);
        }

        /* Camera capture (NO LED overrides here) */
        LinearCameraGetFrameEx(&frame, (uint32)V2_TEST_EXPOSURE_TICKS);

        boolean doDisplay = ((DISP_TICKS != 0U) && ((tickCount % DISP_TICKS) == 0U));

        dbg.mask      = 0u;
        dbg.smoothOut = (uint8*)0;

        if ((doDisplay == TRUE) && (VisionDebug_WantsVisionDebugData(&vdbg) == TRUE))
        {
            VisionDebug_PrepareVisionDbg(&vdbg, &dbg, smoothBuf);
        }

        VisionLinear_ProcessFrameEx(frame.Values, &result, &dbg);

        if (time_reached(nowMs, nextSteerMs))
        {
            nextSteerMs += STEER_UPDATE_MS;

            if (running == FALSE)
            {
                fullcar_stop();
                /* keep idle LED unless fail */
                if (fail == FALSE) StatusLed_SetIdle();
            }
            else
            {
                const float dt = ((float)STEER_UPDATE_MS) * 0.001f;

                uint8 baseSpeed =
#if FULLMODE_FORCE_SLOW_SPEED
                    (uint8)FULLMODE_SLOW_SPEED_PCT;
#else
                    (uint8)SPEED_MIN;
#endif

                SteeringOutput_t out = SteeringLinear_UpdateV2(&ctrl, &result, dt, baseSpeed);

                /* FAIL detection: controller requests brake OR vision lost */
                if (out.brake == TRUE)
                {
                    fail = TRUE;
                    fullcar_stop();
                    StatusLed_SetFail();
                    continue;
                }

                /* You can also treat "lost line" as fail if you want */
                if (result.Status == VISION_LINEAR_LOST)
                {
                    fail = TRUE;
                    fullcar_stop();
                    StatusLed_SetFail();
                    continue;
                }

                /* If we are running and no fail -> green */
                fail = FALSE;
                StatusLed_SetRun();

                /* motor */
                EscSetBrake(0U);
                EscSetSpeed((int)out.throttle_pct);

                /* steer shaping */
                steerRaw = (sint16)out.steer_cmd;
                if (abs_s16(steerRaw) <= STEER_DEADBAND) { steerRaw = 0; }

                steerFilt = iir_s16(steerFilt, steerRaw, 0.45f);

                {
                    /* Rate limit steering changes so it doesn't snap instantly */
                    sint16 delta = (sint16)(steerFilt - steerOut);
                    delta = clamp_s16(delta, (sint16)(-STEER_RATE_MAX), (sint16)(+STEER_RATE_MAX));
                    steerOut = (sint16)(steerOut + delta);
                }

                steerOut = clamp_s16(steerOut,
                                     (sint16)(-STEER_CMD_CLAMP),
                                     (sint16)(+STEER_CMD_CLAMP));

                Steer((int)steerOut);
            }
        }

        if (doDisplay == TRUE)
        {
            const uint8 *pSmooth = (dbg.smoothOut != (uint8*)0) ? smoothBuf : (const uint8*)0;
            const VisionLinear_DebugOut_t *pDbg = (dbg.mask != 0u) ? &dbg : (const VisionLinear_DebugOut_t*)0;
            VisionDebug_Draw(&vdbg, frame.Values, pSmooth, &result, pDbg);
        }

        if (g_ultraEnabled == TRUE)
        {
            Ultrasonic_Task();
        }

        UI_Task(Ultrasonic_GetStatus(),
                Ultrasonic_GetIrqCount(),
                Ultrasonic_GetLastHighTicks(),
                UI_BRAKE_STATE_IDLE,
                0.0f);
    }
}

/* =========================================================
   Dispatcher
========================================================= */
void App_RunSelectedMode(void)
{
#if APP_TEST_ESC_ONLY_WORKING
    mode_esc_only_working();
#elif APP_TEST_VISION_V2_DEBUG
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
