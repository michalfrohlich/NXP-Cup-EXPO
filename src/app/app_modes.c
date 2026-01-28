/*
  app_modes.c
  ===========
  Application layer (the only place that touches hardware drivers).

  It provides:
   - Servo only test
   - ESC only test
   - Camera debug (v1)
   - Ultrasonic debug
   - Full line follow (v1 vision + PD/PID controller)  [VERY SLOW]
   - Vision v2 debug UI (friend’s latest approach)     [camera module testing]
   - NEW: Camera + Servo only (v1)  [NO MOTORS]
*/

#include "app_modes.h"
#include "car_config.h"
#include "main_types.h"

/* ===== Hardware drivers ===== */
#include "main_functions.h"
#include "timebase.h"
#include "buttons.h"
#include "onboard_pot.h"
#include "servo.h"
#include "esc.h"
#include "linear_camera.h"
#include "display.h"
#include "ultrasonic.h"

/* ===== Services (your current v1 pipeline) ===== */
#include "../services/vision_linear.h"
#include "../services/steering_control_linear.h"

/* ===== Friend-style debug pipeline (v2) ===== */
#include "rgb_led.h"
#include "vision_linear_v2.h"
#include "vision_debug.h"

/* =========================================================
   Helpers
========================================================= */

static boolean time_reached(uint32 nowMs, uint32 dueMs)
{
    return ((uint32)(nowMs - dueMs) < 0x80000000u) ? TRUE : FALSE;
}

static uint8 speed_from_pot(uint8 pot)
{
    uint16 span = (uint16)(SPEED_MAX - SPEED_MIN);
    return (uint8)(SPEED_MIN + ((uint16)pot * span) / 255u);
}

static uint8 threshold_from_pot(uint8 pot)
{
    return (uint8)(20u + (uint16)((uint16)pot * 50u) / 255u);
}

/* IMPORTANT:
   This function uses ESC, so DO NOT use it in "camera+servo only" mode. */
static void safe_stop_outputs(void)
{
    EscSetSpeed(0);
    EscSetBrake(1U);
    SteerStraight();
}

static sint32 steer_cmd_from_pot(uint8 pot)
{
    sint32 cmd;

    if (pot <= (uint8)POT_CENTER_RAW)
    {
        uint16 denom = (uint16)(POT_CENTER_RAW - POT_LEFT_RAW);
        if (denom == 0u) denom = 1u;

        cmd = -((sint32)((uint8)POT_CENTER_RAW - pot) * (sint32)STEER_CMD_CLAMP) / (sint32)denom;
    }
    else
    {
        uint16 denom = (uint16)(POT_RIGHT_RAW - POT_CENTER_RAW);
        if (denom == 0u) denom = 1u;

        cmd = ((sint32)(pot - (uint8)POT_CENTER_RAW) * (sint32)STEER_CMD_CLAMP) / (sint32)denom;
    }

    cmd = cmd * (sint32)STEER_SIGN;
    return cmd;
}

/* =========================================================
   MODE 1: SERVO ONLY TEST
========================================================= */
static void mode_servo_only(void)
{
    DriversInit();
    Timebase_Init();
    OnboardPot_Init();

    ServoInit(SERVO_PWM_CH, SERVO_DUTY_MAX, SERVO_DUTY_MIN, SERVO_DUTY_MED);
    SteerStraight();

    uint32 now = Timebase_GetMs();
    uint32 nextButtonsMs = now + BUTTONS_PERIOD_MS;

    CarMode_t mode = CAR_IDLE;
    uint32 startGoMs = 0u;

    while (1)
    {
        now = Timebase_GetMs();

        if (time_reached(now, nextButtonsMs))
        {
            nextButtonsMs += BUTTONS_PERIOD_MS;
            Buttons_Update();

            if (Buttons_WasPressed(BUTTON_ID_SW3))
            {
                mode = CAR_IDLE;
                SteerStraight();
            }

            if ((mode == CAR_IDLE) && Buttons_WasPressed(BUTTON_ID_SW2))
            {
                mode = CAR_ARMING;
                startGoMs = now + START_DELAY_MS;
                SteerStraight();
            }
        }

        if ((mode == CAR_ARMING) && time_reached(now, startGoMs))
        {
            mode = CAR_RUN;
        }

        if (mode != CAR_RUN)
        {
            SteerStraight();
            continue;
        }

        {
            uint8 pot = OnboardPot_ReadLevelFiltered();
            sint32 cmd = steer_cmd_from_pot(pot);
            Steer((int)cmd);
        }
    }
}

/* =========================================================
   MODE 2: ESC ONLY TEST (pot -> -100..+100)
========================================================= */
static void mode_motor_esc_only(void)
{
    DriversInit();
    Timebase_Init();
    OnboardPot_Init();

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

    uint32 now = Timebase_GetMs();
    uint32 nextButtonsMs = now + BUTTONS_PERIOD_MS;

    CarMode_t mode = CAR_IDLE;
    uint32 startGoMs = 0u;

    while (1)
    {
        now = Timebase_GetMs();

        if (time_reached(now, nextButtonsMs))
        {
            nextButtonsMs += BUTTONS_PERIOD_MS;
            Buttons_Update();

            if (Buttons_WasPressed(BUTTON_ID_SW3))
            {
                mode = CAR_IDLE;
                EscSetBrake(0U);
                EscSetSpeed(0);
            }

            if ((mode == CAR_IDLE) && Buttons_WasPressed(BUTTON_ID_SW2))
            {
                mode = CAR_ARMING;
                startGoMs = now + START_DELAY_MS;
                EscSetBrake(0U);
                EscSetSpeed(0);
            }
        }

        if ((mode == CAR_ARMING) && time_reached(now, startGoMs))
        {
            mode = CAR_RUN;
        }

        if (mode != CAR_RUN)
        {
            continue;
        }

        {
            uint8 pot = OnboardPot_ReadLevelFiltered();
            sint32 cmdPct;

            if (pot <= POT_CENTER_RAW)
            {
                uint16 denom = (uint16)(POT_CENTER_RAW - POT_LEFT_RAW);
                if (denom == 0u) denom = 1u;
                cmdPct = -((sint32)(POT_CENTER_RAW - pot) * 100) / (sint32)denom;
            }
            else
            {
                uint16 denom = (uint16)(POT_RIGHT_RAW - POT_CENTER_RAW);
                if (denom == 0u) denom = 1u;
                cmdPct = ((sint32)(pot - POT_CENTER_RAW) * 100) / (sint32)denom;
            }

            /* keep original behavior (warnings are fine) */
            if (cmdPct < MOTOR_DEADBAND_PCT && cmdPct > -MOTOR_DEADBAND_PCT)
            {
                cmdPct = 0;
            }

            EscSetBrake(0U);
            EscSetSpeed((int)cmdPct);
        }
    }
}

/* =========================================================
   MODE 3: CAMERA ONLY (v1 VisionLinear_Process debug text)
========================================================= */
static void mode_camera_only_v1(void)
{
    DriversInit();
    Timebase_Init();
    OnboardPot_Init();

    DisplayInit(0U, STD_ON);
    DisplayClear();
    DisplayText(0U, "CAM V1 TEST", 11U, 0U);
    DisplayText(1U, "SW3 freezes", 11U, 0U);
    DisplayRefresh();

    LinearCameraInit(CAM_CLK_PWM_CH, CAM_SHUTTER_GPT_CH, CAM_ADC_GROUP, CAM_SHUTTER_PCR);

    uint32 now = Timebase_GetMs();
    uint32 nextButtonsMs = now + BUTTONS_PERIOD_MS;
    uint32 nextCameraMs  = now + CAMERA_PERIOD_MS;
    uint32 nextDispMs    = now + DISPLAY_PERIOD_MS;

    CarMode_t mode = CAR_RUN;

    LinearCameraFrame frame;
    LinearVision_t vis;
    vis.valid = FALSE;
    vis.lane_center_px = (sint16)CAM_CENTER_PX;
    sint16 lastCenter = (sint16)CAM_CENTER_PX;

    while (1)
    {
        now = Timebase_GetMs();

        if (time_reached(now, nextButtonsMs))
        {
            nextButtonsMs += BUTTONS_PERIOD_MS;
            Buttons_Update();

            if (Buttons_WasPressed(BUTTON_ID_SW3))
            {
                mode = CAR_IDLE;
            }
        }

        if (mode == CAR_IDLE)
        {
            continue;
        }

        if (time_reached(now, nextCameraMs))
        {
            nextCameraMs += CAMERA_PERIOD_MS;

            uint8 thr = BLACK_THRESHOLD_DEFAULT;
#if USE_POT_FOR_THRESHOLD
            thr = threshold_from_pot(OnboardPot_ReadLevelFiltered());
#endif

            LinearCameraGetFrame(&frame);
            vis = VisionLinear_Process(&frame, thr, lastCenter);

            if (vis.valid)
            {
                lastCenter = vis.lane_center_px;
            }
        }

        if (time_reached(now, nextDispMs))
        {
            nextDispMs += DISPLAY_PERIOD_MS;

            DisplayClear();

            DisplayText(0U, "LC:", 3U, 0U);
            DisplayValue(0U, (int)vis.lane_center_px, 3U, 3U);

            DisplayText(1U, "ERR:", 4U, 0U);
            DisplayValue(1U, (int)vis.error_px, 4U, 4U);

            DisplayText(2U, "CF:", 3U, 0U);
            DisplayValue(2U, (int)vis.confidence, 3U, 3U);

            DisplayText(3U, "BR:", 3U, 0U);
            DisplayValue(3U, (int)vis.black_ratio_pct, 3U, 3U);

            DisplayRefresh();
        }
    }
}

/* =========================================================
   MODE 4: ULTRASONIC 500ms DEBUG
========================================================= */
static void mode_ultrasonic_500ms(void)
{
    DriversInit();
    Timebase_Init();

    DisplayInit(0U, STD_ON);
    DisplayClear();
    DisplayRefresh();

    Ultrasonic_Init();

    const char L0[] = "ULTRA 500ms     ";
    const char L1[] = "S:     I:      ";
    const char L2[] = "cm:    x:      ";
    const char L3[] = "t:     a:      ";

    uint32 lastTrigMs     = Timebase_GetMs();
    float  lastDistanceCm = 0.0f;

    for (;;)
    {
        uint32 nowMs = Timebase_GetMs();

        if ((uint32)(nowMs - lastTrigMs) >= 500u)
        {
            if (Ultrasonic_GetStatus() != ULTRA_STATUS_BUSY)
            {
                lastTrigMs = nowMs;
                Ultrasonic_StartMeasurement();
            }
        }

        Ultrasonic_Task();

        Ultrasonic_StatusType st = Ultrasonic_GetStatus();

        float d;
        if (Ultrasonic_GetDistanceCm(&d) == TRUE)
        {
            lastDistanceCm = d;
        }
        else if (st == ULTRA_STATUS_ERROR)
        {
            lastDistanceCm = ULTRA_MAX_DISTANCE_CM + 1.0f;
        }

        uint32 irqCnt = Ultrasonic_GetIrqCount();
        uint32 ticks  = Ultrasonic_GetLastHighTicks();
        uint16 idx    = Ultrasonic_GetTsIndex();
        uint32 ageMs  = (uint32)(nowMs - lastTrigMs);

        DisplayClear();

        DisplayText(0U, L0, 16U, 0U);

        DisplayText(1U, L1, 16U, 0U);
        if (st == ULTRA_STATUS_NEW_DATA)      { DisplayText(1U, "NEW  ", 5U, 2U); }
        else if (st == ULTRA_STATUS_BUSY)    { DisplayText(1U, "BUSY ", 5U, 2U); }
        else if (st == ULTRA_STATUS_ERROR)   { DisplayText(1U, "ERR  ", 5U, 2U); }
        else                                 { DisplayText(1U, "IDLE ", 5U, 2U); }
        DisplayValue(1U, (int)irqCnt, 6U, 10U);

        DisplayText(2U, L2, 16U, 0U);
        {
            int distInt = (int)(lastDistanceCm + 0.5f);
            DisplayValue(2U, distInt, 4U, 3U);
        }
        DisplayValue(2U, (int)idx, 6U, 10U);

        DisplayText(3U, L3, 16U, 0U);
        DisplayValue(3U, (int)ticks, 5U, 2U);
        DisplayValue(3U, (int)ageMs, 6U, 10U);

        DisplayRefresh();
    }
}

/* =========================================================
   MODE 5 NEW: CAMERA + SERVO ONLY (NO MOTOR)
   ---------------------------------------------------------
   SAME "brain" as full mode:
      VisionLinear_Process() -> SteeringLinear_Update() -> Steer()

   But:
   - NO EscInit()
   - NO EscSetSpeed()
   - NO safe_stop_outputs() (because that brakes ESC)

   Buttons:
   - SW2 start after delay
   - SW3 stop immediately -> center steering
========================================================= */
static void mode_camera_servo_only_v1(void)
{
    DriversInit();
    Timebase_Init();
    OnboardPot_Init();

    /* Servo init */
    ServoInit(SERVO_PWM_CH, SERVO_DUTY_MAX, SERVO_DUTY_MIN, SERVO_DUTY_MED);
    SteerStraight();

    /* Camera init */
    LinearCameraInit(CAM_CLK_PWM_CH, CAM_SHUTTER_GPT_CH, CAM_ADC_GROUP, CAM_SHUTTER_PCR);

    /* Display */
    DisplayInit(0U, STD_ON);
    DisplayClear();
    DisplayText(0U, "CAM+SERVO", 9U, 0U);
    DisplayText(1U, "NO MOTOR", 8U, 0U);
    DisplayText(2U, "SW2 start", 9U, 0U);
    DisplayText(3U, "SW3 stop", 8U, 0U);
    DisplayRefresh();

    uint32 now = Timebase_GetMs();
    uint32 nextButtonsMs = now + BUTTONS_PERIOD_MS;
    uint32 nextCameraMs  = now + CAMERA_PERIOD_MS;
    uint32 nextControlMs = now + CONTROL_PERIOD_MS;
    uint32 nextDispMs    = now + DISPLAY_PERIOD_MS;

    CarMode_t mode = CAR_IDLE;
    uint32 startGoMs = 0u;

    SteeringLinearState_t ctrl;
    SteeringLinear_Init(&ctrl);

    LinearCameraFrame frame;
    LinearVision_t vis;
    vis.valid = FALSE;
    vis.lane_center_px = (sint16)CAM_CENTER_PX;
    sint16 lastCenter = (sint16)CAM_CENTER_PX;

    sint16 lastSteerCmd = 0;

    while (1)
    {
        now = Timebase_GetMs();

        /* Buttons */
        if (time_reached(now, nextButtonsMs))
        {
            nextButtonsMs += BUTTONS_PERIOD_MS;
            Buttons_Update();

            if (Buttons_WasPressed(BUTTON_ID_SW3))
            {
                mode = CAR_IDLE;
                SteerStraight();
            }

            if ((mode == CAR_IDLE) && Buttons_WasPressed(BUTTON_ID_SW2))
            {
                mode = CAR_ARMING;
                startGoMs = now + START_DELAY_MS;

                SteeringLinear_Reset(&ctrl);
                lastCenter = (sint16)CAM_CENTER_PX;
                lastSteerCmd = 0;

                SteerStraight();
            }
        }

        if ((mode == CAR_ARMING) && time_reached(now, startGoMs))
        {
            mode = CAR_RUN;
            SteeringLinear_Reset(&ctrl);
        }

        if (mode == CAR_IDLE)
        {
            SteerStraight();
            continue;
        }

        /* Camera */
        if (time_reached(now, nextCameraMs))
        {
            nextCameraMs += CAMERA_PERIOD_MS;

            uint8 thr = BLACK_THRESHOLD_DEFAULT;
#if USE_POT_FOR_THRESHOLD
            thr = threshold_from_pot(OnboardPot_ReadLevelFiltered());
#endif
            LinearCameraGetFrame(&frame);
            vis = VisionLinear_Process(&frame, thr, lastCenter);

            if (vis.valid)
            {
                lastCenter = vis.lane_center_px;
            }
        }

        /* Control -> steer only */
        if (time_reached(now, nextControlMs))
        {
            nextControlMs += CONTROL_PERIOD_MS;
            float dt = ((float)CONTROL_PERIOD_MS) * 0.001f;

            if (mode != CAR_RUN)
            {
                SteerStraight();
                continue;
            }

            /* "base_speed" used only for speed policy inside controller.
               We are not driving motors, so any safe constant is fine. */
            const uint8 fakeSpeed = 20U;

            SteeringOutput_t out = SteeringLinear_Update(&ctrl, &vis, dt, fakeSpeed);

            Steer((int)out.steer_cmd);
            lastSteerCmd = out.steer_cmd;
        }

        /* Display */
        if (time_reached(now, nextDispMs))
        {
            nextDispMs += DISPLAY_PERIOD_MS;

            DisplayClear();

            if (mode == CAR_IDLE) DisplayText(0U, "STATE: IDLE", 11U, 0U);
            else if (mode == CAR_ARMING) DisplayText(0U, "STATE: ARM", 10U, 0U);
            else DisplayText(0U, "STATE: RUN", 10U, 0U);

            DisplayText(1U, "LC:", 3U, 0U);
            DisplayValue(1U, (int)vis.lane_center_px, 3U, 3U);

            DisplayText(1U, "CF:", 3U, 8U);
            DisplayValue(1U, (int)vis.confidence, 3U, 11U);

            DisplayText(2U, "ER:", 3U, 0U);
            DisplayValue(2U, (int)vis.error_px, 4U, 3U);

            DisplayText(3U, "ST:", 3U, 0U);
            DisplayValue(3U, (int)lastSteerCmd, 4U, 3U);

            DisplayRefresh();
        }
    }
}

/* =========================================================
   MODE 5: FULL LINE FOLLOW (v1 pipeline) - VERY SLOW
========================================================= */
static void mode_full_line_follow_v1(void)
{
    DriversInit();
    Timebase_Init();
    OnboardPot_Init();

    ServoInit(SERVO_PWM_CH, SERVO_DUTY_MAX, SERVO_DUTY_MIN, SERVO_DUTY_MED);
    SteerStraight();

    EscInit(ESC_PWM_CH, ESC_DUTY_MIN, ESC_DUTY_MED, ESC_DUTY_MAX);
    EscSetBrake(1U);
    EscSetSpeed(0);

    LinearCameraInit(CAM_CLK_PWM_CH, CAM_SHUTTER_GPT_CH, CAM_ADC_GROUP, CAM_SHUTTER_PCR);

    DisplayInit(0U, STD_ON);
    DisplayClear();
    DisplayText(0U, "LINE FOLLOW V1", 14U, 0U);
    DisplayText(1U, "SW2 start", 9U, 0U);
    DisplayText(2U, "SW3 stop", 8U, 0U);
    DisplayRefresh();

    uint32 now = Timebase_GetMs();
    uint32 nextButtonsMs = now + BUTTONS_PERIOD_MS;
    uint32 nextCameraMs  = now + CAMERA_PERIOD_MS;
    uint32 nextControlMs = now + CONTROL_PERIOD_MS;
    uint32 nextDispMs    = now + DISPLAY_PERIOD_MS;

    CarMode_t mode = CAR_IDLE;
    uint32 startGoMs = 0u;
    uint32 lastLineSeenMs = now;

    uint32 intersectionEndMs = 0u;
    sint16 intersectionSteerCmd = 0;

    SteeringLinearState_t ctrl;
    SteeringLinear_Init(&ctrl);

    LinearCameraFrame frame;
    LinearVision_t vis;
    vis.valid = FALSE;
    vis.lane_center_px = (sint16)CAM_CENTER_PX;
    sint16 lastCenter = (sint16)CAM_CENTER_PX;

    sint16 lastSteerCmd = 0;

    safe_stop_outputs();

    while (1)
    {
        now = Timebase_GetMs();

        if (time_reached(now, nextButtonsMs))
        {
            nextButtonsMs += BUTTONS_PERIOD_MS;
            Buttons_Update();

            if (Buttons_WasPressed(BUTTON_ID_SW3))
            {
                mode = CAR_IDLE;
                safe_stop_outputs();
            }

            if ((mode == CAR_IDLE) && Buttons_WasPressed(BUTTON_ID_SW2))
            {
                mode = CAR_ARMING;
                startGoMs = now + START_DELAY_MS;

                SteeringLinear_Reset(&ctrl);
                lastCenter = (sint16)CAM_CENTER_PX;
                lastSteerCmd = 0;
                lastLineSeenMs = now;

                safe_stop_outputs();
            }
        }

        if (time_reached(now, nextCameraMs))
        {
            nextCameraMs += CAMERA_PERIOD_MS;

            if (mode != CAR_IDLE)
            {
                uint8 thr = BLACK_THRESHOLD_DEFAULT;
#if USE_POT_FOR_THRESHOLD
                thr = threshold_from_pot(OnboardPot_ReadLevelFiltered());
#endif
                LinearCameraGetFrame(&frame);
                vis = VisionLinear_Process(&frame, thr, lastCenter);

                if (vis.valid)
                {
                    lastCenter = vis.lane_center_px;
                    lastLineSeenMs = now;
                }
            }
        }

        if (time_reached(now, nextControlMs))
        {
            nextControlMs += CONTROL_PERIOD_MS;
            float dt = ((float)CONTROL_PERIOD_MS) * 0.001f;

            if (mode == CAR_IDLE)
            {
                safe_stop_outputs();
                continue;
            }

            if (mode == CAR_ARMING)
            {
                safe_stop_outputs();

                if (time_reached(now, startGoMs))
                {
                    mode = CAR_RUN;
                    SteeringLinear_Reset(&ctrl);
                }
                continue;
            }

            if ((uint32)(now - lastLineSeenMs) > LINE_LOST_COAST_MS)
            {
                mode = CAR_IDLE;
                safe_stop_outputs();
                continue;
            }

            uint8 baseSpeed;
#if FULLMODE_FORCE_SLOW_SPEED
            baseSpeed = (uint8)FULLMODE_SLOW_SPEED_PCT;
#else
            baseSpeed = speed_from_pot(OnboardPot_ReadLevelFiltered());
#endif

            if ((mode == CAR_RUN) && (vis.intersection_likely == TRUE))
            {
                if (vis.error_px > -10 && vis.error_px < 10) intersectionSteerCmd = 0;
                else intersectionSteerCmd = (vis.error_px > 0) ? +35 : -35;

                intersectionEndMs = now + INTERSECTION_COMMIT_MS;
                mode = CAR_INTERSECTION_COMMIT;
            }

            if (mode == CAR_INTERSECTION_COMMIT)
            {
                if (time_reached(now, intersectionEndMs))
                {
                    mode = CAR_RUN;
                }
                else
                {
                    EscSetBrake(0U);
                    EscSetSpeed((int)INTERSECTION_SPEED);
                    Steer((int)intersectionSteerCmd);
                    lastSteerCmd = intersectionSteerCmd;
                    continue;
                }
            }

            SteeringOutput_t out = SteeringLinear_Update(&ctrl, &vis, dt, baseSpeed);

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

            Steer((int)out.steer_cmd);
            lastSteerCmd = out.steer_cmd;
        }

        if (time_reached(now, nextDispMs))
        {
            nextDispMs += DISPLAY_PERIOD_MS;

            DisplayClear();

            if (mode == CAR_IDLE) DisplayText(0U, "STATE: IDLE", 11U, 0U);
            else if (mode == CAR_ARMING) DisplayText(0U, "STATE: ARM", 10U, 0U);
            else if (mode == CAR_INTERSECTION_COMMIT) DisplayText(0U, "STATE: XCOM", 11U, 0U);
            else DisplayText(0U, "STATE: RUN", 10U, 0U);

            DisplayText(1U, "LC:", 3U, 0U);
            DisplayValue(1U, (int)vis.lane_center_px, 3U, 3U);

            DisplayText(1U, "CF:", 3U, 8U);
            DisplayValue(1U, (int)vis.confidence, 3U, 11U);

            DisplayText(2U, "ST:", 3U, 0U);
            DisplayValue(2U, (int)lastSteerCmd, 4U, 3U);

            DisplayText(3U, "LOST:", 5U, 0U);
            DisplayValue(3U, (int)(now - lastLineSeenMs), 6U, 5U);

            DisplayRefresh();
        }
    }
}

/* =========================================================
   MODE 6: VISION V2 DEBUG (friend’s debug UI)
   (unchanged from your last working version)
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
    const uint32 DISP_MS    = (uint32)V2_DISPLAY_PERIOD_MS;
    const uint32 DISP_TICKS = (DISP_MS / LOOP_MS);

    for (;;)
    {
        uint32 nowMs = Timebase_GetMs();

        if ((uint32)(nowMs - nextTickMs) < LOOP_MS)
        {
            continue;
        }
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
   MAIN DISPATCHER (compile-time mode selection)
========================================================= */
void App_RunSelectedMode(void)
{
#if APP_TEST_SERVO_ONLY
    mode_servo_only();

#elif APP_TEST_MOTOR_ESC_ONLY
    mode_motor_esc_only();

#elif APP_TEST_CAMERA_ONLY_V1
    mode_camera_only_v1();

#elif APP_TEST_ULTRASONIC_500MS
    mode_ultrasonic_500ms();

#elif APP_TEST_CAMERA_SERVO_ONLY_V1
    mode_camera_servo_only_v1();

#elif APP_TEST_FULL_LINE_FOLLOW_V1
    mode_full_line_follow_v1();

#elif APP_TEST_VISION_V2_DEBUG
    mode_vision_v2_debug();

#else
    DriversInit();
    Timebase_Init();
    while (1) { }
#endif
}
