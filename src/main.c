/*==================================================================================================
*  EXPO_03_Nxp_Cup_project - main.c
*  --------------------------------------------------------------
*  NXP Cup Line Following (Linear Camera) - S32K144EVB
*
*  CAR BEHAVIOR (what it MUST do):
*   - SW2 = START: after 3 seconds it begins driving
*   - SW3 = STOP: stops immediately and returns to IDLE
*   - Linear camera provides 128 pixels -> detect black edge lines -> lane center
*   - Steering: servo (Ackermann)
*   - Drive: ESC PWM (single output for both rear motors in phase-1)
*   - Line lost:
*       If camera sees no black (all white), keep moving for 2 seconds.
*       If still lost after 2 seconds -> stop and go back to IDLE.
*
*  EMBEDDED TIMING / POLLING:
*   - NO random delay loops for control.
*   - Use Timebase_GetMs() to run tasks on schedule:
*       Buttons_Update  every BUTTONS_PERIOD_MS
*       Camera capture  every CAMERA_PERIOD_MS
*       Control update  every CONTROL_PERIOD_MS  (fixed dt for PID)
*       Display update  every DISPLAY_PERIOD_MS
==================================================================================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*==================================================================================================
 *                                        INCLUDE FILES
==================================================================================================*/
#include "main_functions.h"
#include "timebase.h"
#include "buttons.h"
#include "onboard_pot.h"
#include "display.h"

#include "servo.h"
#include "esc.h"
#include "hbridge.h"        /* optional fallback */
#include "linear_camera.h"

#include "Std_Types.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/*==================================================================================================
 *                                       LOCAL MACROS
==================================================================================================*/

/* ===== Choose what runs ===== */
#define MAIN_ENABLE                 1
#define RUN_LINE_FOLLOW_APP         1   /* <-- THIS is the real app */

/* ===== Drive mode =====
 * Phase-1: ESC (recommended). If you ever want H-bridge, set USE_ESC=0.
 */
#define USE_ESC                     1

/* ===== Hardware mapping ===== */

/* Servo (you confirmed this configuration works) */
#define SERVO_PWM_CH                1U

/* SERVO RANGE: CHANGE HERE ONLY
 * ServoInit(channel, MaxDuty, MinDuty, MedDuty)
 * - If wheels don't turn enough: increase distance between MED and MIN/MAX
 * - If wheels bind: reduce extremes
 * - If steering direction is wrong: flip STEER_SIGN (+1 / -1) OR swap MIN/MAX
 */
#define SERVO_DUTY_MAX              3300U
#define SERVO_DUTY_MIN              1700U
#define SERVO_DUTY_MED              2500U
#define STEER_SIGN                  (+1)

/* ESC (values already in your project comments; keep if ESC arms correctly) */
#define ESC_PWM_CH                  0U
#define ESC_DUTY_MIN                1638U
#define ESC_DUTY_MED                2457U
#define ESC_DUTY_MAX                3276U

/* Optional H-bridge mapping (not used in Phase-1) */
#define PWM_LEFT_CH                 2U
#define PWM_RIGHT_CH                3U
#define L_FWD_CH                    32U
#define L_REV_CH                    33U
#define R_FWD_CH                    6U
#define R_REV_CH                    64U

/* Linear camera init (you confirmed this works) */
#define CAM_CLK_PWM_CH              4U
#define CAM_SHUTTER_GPT_CH          1U
#define CAM_ADC_GROUP               0U
#define CAM_SHUTTER_PCR             97U

/* Buttons */
#define BTN_START_ID                BUTTON_ID_SW2
#define BTN_STOP_ID                 BUTTON_ID_SW3

/* ===== Timing ===== */
#define BUTTONS_PERIOD_MS           5u
#define CAMERA_PERIOD_MS            5u    /* 200 Hz camera capture */
#define CONTROL_PERIOD_MS           2u    /* 500 Hz control loop */
#define DISPLAY_PERIOD_MS           100u  /* 10 Hz display refresh */

/* Start delay after SW2 */
#define START_DELAY_MS              3000u

/* Line lost behavior: keep going for 2 seconds */
#define LINE_LOST_COAST_MS          2000u

/* ===== Camera processing ===== */
#define CAM_N_PIXELS                128u
#define CAM_CENTER_PX               63

/* Threshold control */
#define USE_POT_FOR_THRESHOLD       1
#define BLACK_THRESHOLD_DEFAULT     40u

/* Edge width estimate (depends on camera mounting, tune later) */
#define EXPECTED_TRACK_WIDTH_PX     88

/* Intersection heuristics */
#define INTERSECTION_BLACK_RATIO_PCT 35u
#define INTERSECTION_COMMIT_MS       600u
#define INTERSECTION_SPEED           22

/* ===== PID steering tuning =====
 * Error is normalized to roughly [-1..+1]
 */
#define KP                          2.2f
#define KD                          8.0f
#define KI                          0.0f
#define ITERM_CLAMP                 0.6f

/* Output clamps */
#define STEER_CMD_CLAMP             80

/* Low-pass filter on steering output to reduce jitter */
#define STEER_LPF_ALPHA             0.35f

/* Speed policy */
#define SPEED_BASE                  35
#define SPEED_MIN                   18
#define SPEED_MAX                   60
#define SPEED_SLOW_PER_STEER        25
#define SPEED_LOST_LINE             20

/*==================================================================================================
 *                                       HELPERS
==================================================================================================*/
static inline sint32 abs_s32(sint32 x) { return (x < 0) ? -x : x; }

static inline sint32 clamp_s32(sint32 v, sint32 lo, sint32 hi)
{
    return (v < lo) ? lo : ((v > hi) ? hi : v);
}

static inline float clamp_f(float v, float lo, float hi)
{
    return (v < lo) ? lo : ((v > hi) ? hi : v);
}

/* Wrap-safe "time reached" */
static inline boolean time_reached(uint32 nowMs, uint32 dueMs)
{
    return ((uint32)(nowMs - dueMs) < 0x80000000u) ? TRUE : FALSE;
}

/* Map pot 0..255 to threshold 20..70 */
static uint8 threshold_from_pot(uint8 pot)
{
    return (uint8)(20u + (uint16)((uint16)pot * 50u) / 255u);
}

/*==================================================================================================
 *                          CAMERA -> LANE READING
==================================================================================================*/
typedef struct
{
    boolean valid;

    sint16 left_edge_px;     /* -1 if not found */
    sint16 right_edge_px;    /* -1 if not found */
    sint16 lane_center_px;   /* 0..127 */

    uint8 region_count;
    uint8 black_ratio_pct;   /* 0..100 */

    uint8 confidence;        /* 0..100 */
    boolean intersection_likely;
} LaneReading_t;

static uint8 find_black_regions(const uint8 *vals, uint8 thr, sint16 *centers, uint8 maxCenters, uint16 *blackCountOut)
{
    uint8 inBlack = 0u;
    uint16 start = 0u;
    uint8 count = 0u;
    uint16 blackCount = 0u;

    for (uint16 i = 0u; i < CAM_N_PIXELS; i++)
    {
        boolean isBlack = (vals[i] < thr) ? TRUE : FALSE;
        if (isBlack) { blackCount++; }

        if (isBlack && (inBlack == 0u))
        {
            inBlack = 1u;
            start = i;
        }
        else if ((!isBlack) && (inBlack != 0u))
        {
            uint16 end = (i > 0u) ? (i - 1u) : 0u;
            if (count < maxCenters)
            {
                centers[count] = (sint16)((start + end) / 2u);
                count++;
            }
            inBlack = 0u;
        }
    }

    if (inBlack != 0u)
    {
        uint16 end = (CAM_N_PIXELS - 1u);
        if (count < maxCenters)
        {
            centers[count] = (sint16)((start + end) / 2u);
            count++;
        }
    }

    if (blackCountOut != NULL_PTR)
    {
        *blackCountOut = blackCount;
    }
    return count;
}

static LaneReading_t process_camera(const LinearCameraFrame *frame, uint8 thr, sint16 lastLaneCenter)
{
    LaneReading_t out;

    /* FULL initialization (prevents “maybe uninitialized” warnings) */
    out.valid = FALSE;
    out.left_edge_px = -1;
    out.right_edge_px = -1;
    out.lane_center_px = lastLaneCenter;
    out.region_count = 0u;
    out.black_ratio_pct = 0u;
    out.confidence = 0u;
    out.intersection_likely = FALSE;

    sint16 centers[8];
    uint16 blackCount = 0u;

    uint8 n = find_black_regions(frame->Values, thr, centers, 8u, &blackCount);
    out.region_count = n;
    out.black_ratio_pct = (uint8)((blackCount * 100u) / CAM_N_PIXELS);

    if (n == 0u)
    {
        out.valid = FALSE;
        out.confidence = 0u;
        return out;
    }

    if (n == 1u)
    {
        /* one edge -> estimate other using EXPECTED_TRACK_WIDTH_PX */
        sint16 e = centers[0];

        if (e < (sint16)CAM_CENTER_PX)
        {
            out.left_edge_px = e;
            out.right_edge_px = (sint16)(e + EXPECTED_TRACK_WIDTH_PX);
            if (out.right_edge_px > 127) out.right_edge_px = 127;
        }
        else
        {
            out.right_edge_px = e;
            out.left_edge_px = (sint16)(e - EXPECTED_TRACK_WIDTH_PX);
            if (out.left_edge_px < 0) out.left_edge_px = 0;
        }

        out.lane_center_px = (sint16)((out.left_edge_px + out.right_edge_px) / 2);
        out.valid = TRUE;
        out.confidence = 40u;
    }
    else
    {
        /* >=2 regions -> take left-most and right-most */
        out.left_edge_px = centers[0];
        out.right_edge_px = centers[n - 1u];
        out.lane_center_px = (sint16)((out.left_edge_px + out.right_edge_px) / 2);

        /* confidence based on plausible width and region count */
        sint32 width = (sint32)out.right_edge_px - (sint32)out.left_edge_px;
        sint32 widthErr = abs_s32(width - (sint32)EXPECTED_TRACK_WIDTH_PX);

        sint32 conf = 100;
        conf -= (widthErr * 2);
        conf -= ((sint32)(n - 2u) * 10);
        conf = clamp_s32(conf, 0, 100);

        out.confidence = (uint8)conf;
        out.valid = (out.confidence > 10u) ? TRUE : FALSE;
    }

    /* intersection heuristic */
    if ((out.black_ratio_pct >= INTERSECTION_BLACK_RATIO_PCT) || (n >= 3u))
    {
        out.intersection_likely = TRUE;
    }

    return out;
}

/*==================================================================================================
 *                                  PID CONTROLLER STATE
==================================================================================================*/
typedef struct
{
    float prev_err;
    float i_term;
    float steer_filtered;
} PidState_t;

static void pid_reset(PidState_t *s)
{
    s->prev_err = 0.0f;
    s->i_term = 0.0f;
    s->steer_filtered = 0.0f;
}

/*==================================================================================================
 *                                  DRIVE WRAPPERS
==================================================================================================*/
static void drive_stop(void)
{
#if USE_ESC
    EscSetSpeed(0);
    EscSetBrake(1U);
#else
    HbridgeSetSpeed(0);
    HbridgeSetBrake(1U);
#endif
}

static void drive_forward(sint32 speedPercent_0_100)
{
    sint32 spd = clamp_s32(speedPercent_0_100, 0, 100);

#if USE_ESC
    EscSetBrake(0U);
    EscSetSpeed((int)spd);
#else
    HbridgeSetBrake(0U);
    HbridgeSetSpeed((int)spd);
#endif
}

/*==================================================================================================
 *                                  STATE MACHINE
==================================================================================================*/
typedef enum
{
    STATE_IDLE = 0,
    STATE_ARMING,
    STATE_RUNNING,
    STATE_INTERSECTION_COMMIT
} CarState_t;

/*==================================================================================================
 *                                  MAIN
==================================================================================================*/
#if MAIN_ENABLE
int main(void)
{
#if RUN_LINE_FOLLOW_APP

    /* ===== INIT ===== */
    DriversInit();
    Timebase_Init();
    OnboardPot_Init();

    ServoInit(SERVO_PWM_CH, SERVO_DUTY_MAX, SERVO_DUTY_MIN, SERVO_DUTY_MED);
    SteerStraight();

#if USE_ESC
    EscInit(ESC_PWM_CH, ESC_DUTY_MIN, ESC_DUTY_MED, ESC_DUTY_MAX);
    EscSetBrake(1U);
    EscSetSpeed(0);
#else
    HbridgeInit(PWM_LEFT_CH, PWM_RIGHT_CH, L_FWD_CH, L_REV_CH, R_FWD_CH, R_REV_CH);
    HbridgeSetBrake(1U);
    HbridgeSetSpeed(0);
#endif

    LinearCameraInit(CAM_CLK_PWM_CH, CAM_SHUTTER_GPT_CH, CAM_ADC_GROUP, CAM_SHUTTER_PCR);

    DisplayInit(0U, STD_ON);
    DisplayClear();
    DisplayText(0U, "LINE FOLLOW APP", 14U, 0U);
    DisplayText(1U, "SW2 START (3s) ", 14U, 0U);
    DisplayText(2U, "SW3 STOP       ", 10U, 0U);
    DisplayRefresh();

    /* ===== Scheduler timestamps (polling) ===== */
    uint32 now = Timebase_GetMs();
    uint32 nextButtonsMs = now + BUTTONS_PERIOD_MS;
    uint32 nextCameraMs  = now + CAMERA_PERIOD_MS;
    uint32 nextControlMs = now + CONTROL_PERIOD_MS;
    uint32 nextDispMs    = now + DISPLAY_PERIOD_MS;

    /* ===== App state ===== */
    CarState_t state = STATE_IDLE;
    uint32 startGoMs = 0u;

    /* Line-lost timer:
     * We use lastLineSeenMs ONLY (no unused vars).
     * If no line seen for > LINE_LOST_COAST_MS -> stop -> IDLE.
     */
    uint32 lastLineSeenMs = now;

    /* Intersection commit */
    uint32 intersectionEndMs = 0u;
    sint32 intersectionSteerCmd = 0;

    /* Controller state */
    PidState_t pid;
    pid_reset(&pid);

    /* Latest sensor data (initialized to safe defaults) */
    LinearCameraFrame frame;
    LaneReading_t lane;
    lane.valid = FALSE;
    lane.left_edge_px = -1;
    lane.right_edge_px = -1;
    lane.lane_center_px = (sint16)CAM_CENTER_PX;
    lane.region_count = 0u;
    lane.black_ratio_pct = 0u;
    lane.confidence = 0u;
    lane.intersection_likely = FALSE;

    /* Last known good lane center + last steering command */
    sint16 lastLaneCenter = (sint16)CAM_CENTER_PX;
    sint32 lastSteerCmd = 0;

    /* Safe outputs */
    drive_stop();
    SteerStraight();

    while (1)
    {
        now = Timebase_GetMs();

        /* ---------------- Buttons task (polling) ---------------- */
        if (time_reached(now, nextButtonsMs))
        {
            nextButtonsMs += BUTTONS_PERIOD_MS;
            Buttons_Update();

            /* STOP always wins */
            if (Buttons_WasPressed(BTN_STOP_ID))
            {
                state = STATE_IDLE;
                drive_stop();
                SteerStraight();
            }

            /* START only from IDLE */
            if ((state == STATE_IDLE) && Buttons_WasPressed(BTN_START_ID))
            {
                state = STATE_ARMING;
                startGoMs = now + START_DELAY_MS;

                pid_reset(&pid);
                lastSteerCmd = 0;
                lastLaneCenter = (sint16)CAM_CENTER_PX;

                lastLineSeenMs = now;

                drive_stop();
                SteerStraight();
            }
        }

        /* ---------------- Camera task (polling) ---------------- */
        if (time_reached(now, nextCameraMs))
        {
            nextCameraMs += CAMERA_PERIOD_MS;

            if (state != STATE_IDLE)
            {
                uint8 thr = BLACK_THRESHOLD_DEFAULT;
#if USE_POT_FOR_THRESHOLD
                uint8 pot = OnboardPot_ReadLevelFiltered();
                thr = threshold_from_pot(pot);
#endif
                LinearCameraGetFrame(&frame);

                lane = process_camera(&frame, thr, lastLaneCenter);

                if (lane.valid)
                {
                    lastLaneCenter = lane.lane_center_px;
                    lastLineSeenMs = now; /* <-- USED in line-lost logic */
                }
            }
        }

        /* ---------------- Control task (polling fixed-step) ---------------- */
        if (time_reached(now, nextControlMs))
        {
            nextControlMs += CONTROL_PERIOD_MS;
            const float dt = ((float)CONTROL_PERIOD_MS) * 0.001f;

            if (state == STATE_IDLE)
            {
                drive_stop();
                SteerStraight();
                continue;
            }

            if (state == STATE_ARMING)
            {
                drive_stop();
                SteerStraight();

                if (time_reached(now, startGoMs))
                {
                    state = STATE_RUNNING;
                    pid_reset(&pid);
                }
                continue;
            }

            /* ===== RUNNING / INTERSECTION / LINE LOST ===== */

            /* Line lost check (based on elapsed time since last valid line) */
            if ((uint32)(now - lastLineSeenMs) > LINE_LOST_COAST_MS)
            {
                /* Stop and go IDLE */
                state = STATE_IDLE;
                drive_stop();
                SteerStraight();
                continue;
            }

            /* If the current frame is invalid, coast with last commands (still within 2s) */
            if (!lane.valid)
            {
                drive_forward(SPEED_LOST_LINE);
                Steer((int)lastSteerCmd);
                continue;
            }

            /* Intersection commit state machine */
            if ((state == STATE_RUNNING) && lane.intersection_likely)
            {
                sint32 errPx = (sint32)lane.lane_center_px - (sint32)CAM_CENTER_PX;

                if (errPx > -10 && errPx < 10)
                    intersectionSteerCmd = 0;
                else
                    intersectionSteerCmd = (errPx > 0) ? +35 : -35;

                intersectionEndMs = now + INTERSECTION_COMMIT_MS;
                state = STATE_INTERSECTION_COMMIT;
            }

            if (state == STATE_INTERSECTION_COMMIT)
            {
                if (time_reached(now, intersectionEndMs))
                {
                    state = STATE_RUNNING;
                }
                else
                {
                    drive_forward(INTERSECTION_SPEED);
                    Steer((int)intersectionSteerCmd);
                    lastSteerCmd = intersectionSteerCmd;
                    continue;
                }
            }

            /* ===== Normal PID steering ===== */
            {
                float errPx = ((float)lane.lane_center_px) - ((float)CAM_CENTER_PX);
                float error = errPx / 63.0f; /* normalize approx */

                float dErr = (error - pid.prev_err) / (dt + 1e-9f);
                pid.prev_err = error;

                pid.i_term += error * dt;
                pid.i_term = clamp_f(pid.i_term, -ITERM_CLAMP, +ITERM_CLAMP);

                float steer = (KP * error) + (KD * dErr) + (KI * pid.i_term);
                steer = clamp_f(steer, -1.0f, +1.0f);

                float steerCmdF = steer * (float)STEER_CMD_CLAMP * (float)STEER_SIGN;

                /* Low-pass filter */
                pid.steer_filtered = (STEER_LPF_ALPHA * steerCmdF) +
                                     ((1.0f - STEER_LPF_ALPHA) * pid.steer_filtered);

                sint32 steerCmd = (sint32)(pid.steer_filtered);
                steerCmd = clamp_s32(steerCmd, -STEER_CMD_CLAMP, +STEER_CMD_CLAMP);

                Steer((int)steerCmd);
                lastSteerCmd = steerCmd;

                /* Speed policy */
                sint32 absSteer = abs_s32(steerCmd);
                sint32 speed = (sint32)SPEED_BASE - ((sint32)SPEED_SLOW_PER_STEER * absSteer) / 100;
                speed = clamp_s32(speed, SPEED_MIN, SPEED_MAX);

                drive_forward(speed);
            }
        }

        /* ---------------- Display task (polling) ---------------- */
        if (time_reached(now, nextDispMs))
        {
            nextDispMs += DISPLAY_PERIOD_MS;

            DisplayClear();

            if (state == STATE_IDLE) DisplayText(0U, "STATE: IDLE     ", 16U, 0U);
            else if (state == STATE_ARMING) DisplayText(0U, "STATE: ARMING   ", 16U, 0U);
            else if (state == STATE_INTERSECTION_COMMIT) DisplayText(0U, "STATE: XCOMMIT  ", 16U, 0U);
            else DisplayText(0U, "STATE: RUN      ", 16U, 0U);

            DisplayText(1U, "LC:    CF:     ", 16U, 0U);
            DisplayValue(1U, (int)lastLaneCenter, 3U, 3U);
            DisplayValue(1U, (int)lane.confidence, 3U, 12U);

            DisplayText(2U, "LOST(ms):      ", 16U, 0U);
            DisplayValue(2U, (int)(now - lastLineSeenMs), 6U, 9U);

            DisplayText(3U, "ST:            ", 16U, 0U);
            DisplayValue(3U, (int)lastSteerCmd, 4U, 3U);

            DisplayRefresh();
        }
    }

#else
    DriversInit();
    while (1) { }
#endif
}
#endif /* MAIN_ENABLE */

#ifdef __cplusplus
}
#endif
