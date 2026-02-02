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

/* UI (do not fight it now; just compile with project) */
#include "user_interface.h"

/* =========================================================
   Helpers
========================================================= */
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
    boolean enabled;
} EscRunState_t;

static void esc_enter(EscRunState_t *st)
{
    st->mode = CAR_IDLE;
    st->startGoMs = 0u;
    st->enabled = FALSE;

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
        st->enabled = FALSE;
        Esc_StopNeutral();
        return;
    }

    /* SW2 starts from IDLE (same pattern) */
    if ((st->mode == CAR_IDLE) && sw2)
    {
        st->mode = CAR_ARMING;
        st->enabled = TRUE;
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

    LinearCameraFrame frame;
    VisionLinear_ResultType result;

    VisionLinear_DebugOut_t dbg;
    uint8 smoothBuf[VISION_LINEAR_BUFFER_SIZE];

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

    LinearCameraInit(CAM_CLK_PWM_CH, CAM_SHUTTER_GPT_CH, CAM_ADC_GROUP, CAM_SHUTTER_PCR);
    VisionLinear_InitV2();

    VisionDebug_Init(&st->vdbg, (uint8)V2_WHITE_SAT_U8);

    SteeringLinear_Init(&st->ctrl);
    SteeringLinear_Reset(&st->ctrl);

    st->steerRaw = 0;
    st->steerFilt = 0;
    st->steerOut = 0;

    st->nextTickMs = nowMs;
    st->nextSteerMs = nowMs + STEER_UPDATE_MS;
    st->tickCount = 0u;
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

    LinearCameraGetFrameEx(&st->frame, (uint32)V2_TEST_EXPOSURE_TICKS);

    boolean doDisplay = ((DISP_TICKS != 0U) && ((st->tickCount % DISP_TICKS) == 0U));

    st->dbg.mask      = 0u;
    st->dbg.smoothOut = (uint8*)0;

    if ((doDisplay == TRUE) && (VisionDebug_WantsVisionDebugData(&st->vdbg) == TRUE))
    {
        VisionDebug_PrepareVisionDbg(&st->vdbg, &st->dbg, st->smoothBuf);
    }

    VisionLinear_ProcessFrameEx(st->frame.Values, &st->result, &st->dbg);

    if (time_reached(nowMs, st->nextSteerMs))
    {
        st->nextSteerMs += STEER_UPDATE_MS;

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

    if (doDisplay == TRUE)
    {
        const uint8 *pSmooth = (st->dbg.smoothOut != (uint8*)0) ? st->smoothBuf : (const uint8*)0;
        const VisionLinear_DebugOut_t *pDbg = (st->dbg.mask != 0u) ? &st->dbg : (const VisionLinear_DebugOut_t*)0;
        VisionDebug_Draw(&st->vdbg, st->frame.Values, pSmooth, &st->result, pDbg);
    }
}

/* =========================================================
   FINAL DUMMY
   - SW3 switches ESC <-> CAM+SERVO
   - POT mode: FULL RANGE manual (-100..+100)
   - CAM mode: explicit forward speed command capped to FULL_AUTO_SPEED_PCT with ramp
========================================================= */
static void mode_final_dummy(void)
{
    typedef enum { DUMMY_ESC = 0, DUMMY_CAM = 1 } DummyState_t;
    DummyState_t state = DUMMY_ESC;

    /* FULL AUTO (CAM mode) motor command */
    sint32 autoSpeedPct = 0;
    uint32 nextAutoSpeedMs = 0u;

    DriversInit();
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

/* =========================================================
   Dispatcher


  I deleted the other modes From the dispatcher for debugging, but they can be added anytime now
========================================================= */
void App_RunSelectedMode(void)
{
#if APP_TEST_FINAL_DUMMY
    mode_final_dummy();
#elif APP_TEST_ESC_ONLY_WORKING
    while (1) { }
#elif APP_TEST_CAMERA_SERVO_V2
    while (1) { }
#else
    while (1) { }
#endif
}
