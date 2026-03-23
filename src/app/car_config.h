#pragma once
/*
  car_config.h
  ============
  Single source of truth for build-time mode selection and tuning constants.
*/

/* =========================================================
   BUILD MODE FLAGS (Enable EXACTLY ONE REAL MODE)
========================================================= */
#define APP_TEST_LINEAR_CAMERA_TEST       0
#define APP_TEST_RECEIVER_TEST            0
#define APP_TEST_SERVO_TEST               0
#define APP_TEST_ULTRASONIC_TEST          0
#define APP_TEST_ESC_TEST                 0
#define APP_TEST_FINAL_DUMMY              0
#define APP_TEST_NXP_CUP                  1

#define APP_MODE_COUNT ( \
    (APP_TEST_LINEAR_CAMERA_TEST) + \
    (APP_TEST_RECEIVER_TEST) + \
    (APP_TEST_SERVO_TEST) + \
    (APP_TEST_ULTRASONIC_TEST) + \
    (APP_TEST_ESC_TEST) + \
    (APP_TEST_FINAL_DUMMY) + \
    (APP_TEST_NXP_CUP) \
)

#if (APP_MODE_COUNT != 1)
  #error "CONFIG ERROR: Enable EXACTLY ONE APP_TEST_* flag in car_config.h"
#endif

/* =========================================================
   Ultrasonic default + runtime toggle
========================================================= */
#define ULTRASONIC_DEFAULT_ON             0

/* =========================================================
   Timing
========================================================= */
#define START_DELAY_MS                    1000u
#define BUTTONS_PERIOD_MS                 5u
#define CAMERA_PERIOD_MS                  5u
#define DISPLAY_PERIOD_MS                 20u
#define STEER_UPDATE_MS                   10u
#define SERVO_REFRESH_MS                  10u
#define CAM_FRAME_STALE_MS                120u
#define CAM_STEER_HOLD_MS                 350u
#define CAM_DEBUG_PAUSE_HOLD_MS           1000U
#define NXP_CUP_PROFILE_SWITCH_DEBOUNCE_MS 250u

/* =========================================================
   Servo
========================================================= */
#define SERVO_PWM_CH                      1U
/* These are AUTOSAR PWM duty units, not microseconds. */
#define SERVO_DUTY_MIN                    1650U
#define SERVO_DUTY_MED                    2700U
#define SERVO_DUTY_MAX                    3550U

#define STEER_SIGN                        (+1)
#define STEER_CMD_CLAMP                   50

/* Simple compile-time servo enable.
   1 = normal steering output
   0 = keep servo centered / no steering command applied */
#define SERVO_OUTPUT_ENABLE               1
/* 0 = bypass live camera steering smoothing in FINAL_DUMMY / NXP_CUP.
   Servo test still keeps its own RAW / SMOOTH selector. */
#define LIVE_STEER_SMOOTHING_ENABLE       0

/* Servo test mode only.
   These do NOT affect FINAL_DUMMY / NXP_CUP steering.
    */
#define SERVO_TEST_CMD_CLAMP              70
#define SERVO_TEST_RATE_MAX               8
#define SERVO_TEST_DEADBAND               3
#define SERVO_TEST_LPF_ALPHA              0.45f
#define SERVO_TEST_UPDATE_MS              5u

/* Pot constants (required by ESC-only working mode) */
#define POT_LEFT_RAW                      0
#define POT_CENTER_RAW                    128
#define POT_RIGHT_RAW                     255

/* =========================================================
   ESC
========================================================= */
#define ESC_PWM_CH                        0U

/* ESC calibration values (timer compare) that arm correctly on your setup.
   DO NOT change these to "25% speed".
   Old attempt (did NOT arm): 409/614/819
*/
#define ESC_DUTY_MIN                      1638U
#define ESC_DUTY_MED                      2457U
#define ESC_DUTY_MAX                      3276U

#define MOTOR_DEADBAND_PCT                6U
#define ESC_ARM_TIME_MS                   3000u

/* =========================================================
   Linear camera
========================================================= */
#define CAM_CLK_PWM_CH                    4U
#define CAM_SHUTTER_GPT_CH                1U
#define CAM_ADC_GROUP                     0U
#define CAM_SHUTTER_PCR                   97U

#define CAM_SHUTTER_HIGH_TIME_TICKS       100U
#define CAM_FRAME_INTERVAL_TICKS          50000U

#define CAM_N_PIXELS                      128u
#define CAM_TRIM_LEFT_PX                  2u
#define CAM_TRIM_RIGHT_PX                 2u
#define CAM_EFFECTIVE_PIXELS             (CAM_N_PIXELS - CAM_TRIM_LEFT_PX - CAM_TRIM_RIGHT_PX)
#define CAM_CENTER_PX                    ((CAM_EFFECTIVE_PIXELS - 1u) / 2u)

#define V2_LOOP_PERIOD_MS                 5u
#define V2_TEST_EXPOSURE_TICKS            100u
#define V2_WHITE_SAT_U8                   220u

#define VLIN_MAX_EDGE_CANDIDATES          12U

#define VISION_LINEAR_MIN_CONTRAST        650U
#define VISION_LINEAR_MIN_WEAK_EDGE       32U
#define VISION_LINEAR_MIN_STRONG_EDGE     40U
#define VISION_LINEAR_EDGE_HIGH_PCT       40U
#define VISION_LINEAR_EDGE_LOW_PCT        55U
#define VISION_LINEAR_NOMINAL_LANE_WIDTH  90U
#define VISION_LINEAR_LANE_WIDTH_TOL_PCT  20U
#define VISION_LINEAR_SPLIT_MARGIN_PX     10U

#define VISION_FINISH_INNER_WIDTH_MM      530U
#define VISION_FINISH_BAR_WIDTH_MM         94U
#define VISION_FINISH_CENTER_GAP_MM        74U
#define VISION_FINISH_WIDTH_MIN_PCT       50U
#define VISION_FINISH_WIDTH_MAX_PCT       150U

/* =========================================================
   Full car safety
========================================================= */
#define LINE_LOST_COAST_MS                2000u
#define SPEED_LOST_LINE                   0

/* =========================================================
   PID (Vision V2 error is normalized -1..+1)
========================================================= */
#define KP                                2.4f
#define KD                                0.5f
#define KI                                0.00f
#define ITERM_CLAMP                       0.20f
#define STEER_CENTER_ERR_DEADBAND         0.07f
#define STEER_LPF_ALPHA                   0.70f
#define STEER_ERROR_LPF_ALPHA             0.30f
#define STEER_D_INPUT_ALPHA               0.55f
#define STEER_DTERM_LPF_ALPHA             0.35f
#define STEER_DTERM_CLAMP                 4.0f
#define STEER_CMD_DEADBAND                6
#define STEER_CMD_SHAPE_BLEND_PCT         75
#define STEER_RATE_BOOST_DIV              4
#define STEER_RATE_BOOST_MAX              10

/* =========================================================
   Speed policy
========================================================= */
#define SPEED_MIN                         18
#define SPEED_MAX                         60
#define SPEED_SLOW_PER_STEER              25

#define FULLMODE_FORCE_SLOW_SPEED         1
#define FULLMODE_SLOW_SPEED_PCT           12

#define FULL_AUTO_SPEED_PCT               25
#define FULL_AUTO_RAMP_STEP_PCT           2
#define FULL_AUTO_RAMP_PERIOD_MS          20u

/* =========================================================
   NXP CUP MODE
=========================================================
*/

#define NXP_CUP_DEFAULT_PROFILE           1U   /* 0=SUPERFAST, 1=5050, 2=SLOW */

#define NXP_CUP_MODE_PIN_ENABLE           0
#define NXP_CUP_MODE_PIN_CH               0U
#define NXP_CUP_MODE_PIN_ACTIVE_LEVEL     0U



/* =========================================================
   GENERAL TUNING INSTRUCTIONS
=========================================================

This file has two groups of steering knobs:

1) Per-profile knobs:
   NXP_CUP_*_KP
   NXP_CUP_*_KD
   NXP_CUP_*_KI
   NXP_CUP_*_ITERM_CLAMP
   NXP_CUP_*_STEER_CLAMP
   NXP_CUP_*_STEER_LPF_ALPHA
   NXP_CUP_*_STEER_RATE_MAX
   NXP_CUP_*_SPEED_PCT

2) Shared steering shaping / noise knobs:
   STEER_CENTER_ERR_DEADBAND
   STEER_ERROR_LPF_ALPHA
   STEER_D_INPUT_ALPHA
   STEER_DTERM_LPF_ALPHA
   STEER_DTERM_CLAMP
   STEER_CMD_DEADBAND
   STEER_CMD_SHAPE_BLEND_PCT
   STEER_RATE_BOOST_DIV
   STEER_RATE_BOOST_MAX

Recommended order when a profile is too aggressive:
1) Lower NXP_CUP_*_STEER_RATE_MAX
2) Lower NXP_CUP_*_KP
3) Lower NXP_CUP_*_STEER_LPF_ALPHA
4) Lower NXP_CUP_*_KD only if D noise is part of the problem
5) Raise STEER_CENTER_ERR_DEADBAND or STEER_CMD_DEADBAND if straights still twitch

Recommended order when a profile is too weak:
1) Raise NXP_CUP_*_STEER_RATE_MAX
2) Raise NXP_CUP_*_KP
3) Raise NXP_CUP_*_STEER_LPF_ALPHA
4) Raise NXP_CUP_*_STEER_CLAMP only if it still cannot make the turn

What each knob does:

NXP_CUP_*_STEER_RATE_MAX
- Max steering change per update.
- Lower it if the car commits too hard, zigzags, or snaps into turns.
- Raise it if the car reacts too late entering curves.

NXP_CUP_*_KP
- Main steering strength.
- Lower it if the car overcorrects or weaves.
- Raise it if the car stays off-center and does not correct enough.

NXP_CUP_*_KD
- Damping and edge anticipation.
- Lower it if steering reacts too hard to noisy line movement.
- Raise it if the car oscillates after a turn and needs more settling.

NXP_CUP_*_STEER_LPF_ALPHA
- Output smoothing.
- Higher alpha = less smoothing and faster response.
- Lower it if the servo movement is too sharp or twitchy.
- Raise it if the car feels delayed and lazy.

NXP_CUP_*_STEER_CLAMP
- Maximum steering authority.
- Lower it if the car uses too much steering overall.
- Raise it only if it cannot physically turn enough.

NXP_CUP_*_KI and NXP_CUP_*_ITERM_CLAMP
- Long-term bias correction.
- Lower them if the car "keeps committing" after the line has already moved back.
- Raise them only if the car has a steady left/right bias on long straights.

STEER_CENTER_ERR_DEADBAND
- Ignores tiny camera error near center before PID.
- Raise it if straights still cause small left-right wiggles.
- Lower it if the car waits too long before correcting.

STEER_ERROR_LPF_ALPHA
- Smooths the camera error before PID.
- Lower it for noisier camera motion and calmer steering.
- Raise it for faster response if the image is already clean.

STEER_D_INPUT_ALPHA and STEER_DTERM_LPF_ALPHA
- Smooth the derivative path.
- Lower them if D feels noisy or the car reacts to tiny line jitter.
- Raise them only if steering feels too damped and slow.

STEER_DTERM_CLAMP
- Hard cap on derivative contribution.
- Lower it if brief line movement creates big steering spikes.
- Raise it only if the car overshoots because D is too limited.

STEER_CMD_DEADBAND
- Small steering commands are forced to zero after PID.
- Raise it if the servo chatters near center.
- Lower it if the car needs more tiny corrections on straights.

STEER_CMD_SHAPE_BLEND_PCT
- Higher values soften small and medium steering commands while keeping large ones available.
- Raise it if the car commits too much to mild curves or small zigzags.
- Lower it if the car feels too soft around center.

STEER_RATE_BOOST_DIV and STEER_RATE_BOOST_MAX
- Extra rate allowance for larger steering requests.
- Lower STEER_RATE_BOOST_MAX if big moves still snap too hard.
- Raise it only if large corners are too slow even with reasonable RATE_MAX.

Ultrasonic testing notes:
- Set NXP_CUP_ULTRASONIC_ENABLE to 0 to disable ultrasonic in all NXP Cup profiles.
- NXP_ULTRA_ENABLE_AFTER_RUN_MS delays when ultrasonic starts affecting the run.
- NXP_CUP_ULTRA_STOP_CM is the first trigger distance.
- NXP_CUP_ULTRA_STOP_HOLD_MS is how long speed stays at 0 after the trigger.
- NXP_CUP_ULTRA_CRAWL_LOGICAL_CMD is the fixed ESC logical command after the hold.
- NXP_CUP_ULTRA_CRAWL_STOP_CM is the hard cutoff distance for servo + ESC.
*/



/* ---------- SUPERFAST ---------- */
#define NXP_CUP_SUPERFAST_KP              3.40f
#define NXP_CUP_SUPERFAST_KD              0.88f
#define NXP_CUP_SUPERFAST_KI              0.02f
#define NXP_CUP_SUPERFAST_ITERM_CLAMP     0.06f
#define NXP_CUP_SUPERFAST_STEER_CLAMP     60
#define NXP_CUP_SUPERFAST_STEER_LPF_ALPHA 0.44f
#define NXP_CUP_SUPERFAST_STEER_RATE_MAX  24
#define NXP_CUP_SUPERFAST_SPEED_PCT       40

/* ---------- 5050 ---------- */
/* Feb 2 old-controller equivalent for 5050:
   - old global PID: KP 4.5, KD 2.0, KI 0.03, ITERM 0.3
   - old app path: output LPF 0.45, fixed rate 8, no shape/boost, tiny deadband
   - the extra legacy-equivalence filters/deadzones are pinned internally in
     app_modes.c so the tuning surface here stays small.
   - practical tuning notes for the hidden smoothness values are in
     src/app/steering_tuning_notes.md */
#define NXP_CUP_5050_KP                   4.5f
#define NXP_CUP_5050_KD                   2.0f
#define NXP_CUP_5050_KI                   0.03f
#define NXP_CUP_5050_ITERM_CLAMP          0.30f
#define NXP_CUP_5050_STEER_CLAMP          100
#define NXP_CUP_5050_STEER_LPF_ALPHA      0.45f
#define NXP_CUP_5050_STEER_RATE_MAX       8
#define NXP_CUP_5050_SPEED_PCT            25

/* ---------- SLOW ---------- */
#define NXP_CUP_SLOW_KP                   0.72f
#define NXP_CUP_SLOW_KD                   0.22f
#define NXP_CUP_SLOW_KI                   0.00f
#define NXP_CUP_SLOW_ITERM_CLAMP          0.03f
#define NXP_CUP_SLOW_STEER_CLAMP          60
#define NXP_CUP_SLOW_STEER_LPF_ALPHA      0.38f
/* Lower than the other profiles on purpose so the servo sweeps across its
   range more slowly while you tune the slow mode. */
#define NXP_CUP_SLOW_STEER_RATE_MAX       6
#define NXP_CUP_SLOW_SPEED_PCT            16




/* ---------- NXP CUP ultrasonic behavior ----------
   Set to 0 while tuning in a small room with nearby walls. */
#define NXP_CUP_ULTRASONIC_ENABLE         1
/* Lower period = faster re-trigger / faster wall detection.
   The driver still blocks overlap because it will not start a new ping while
   the previous one is BUSY. */
#define NXP_CUP_ULTRA_TRIGGER_PERIOD_MS   1u

/* Ultrasonic starts affecting the run 2 s after launch in all NXP Cup profiles. */
#define NXP_ULTRA_ENABLE_AFTER_RUN_MS     2000u

/* At or below this distance, speed is forced to 0 for NXP_CUP_ULTRA_STOP_HOLD_MS. */
#define NXP_CUP_ULTRA_STOP_CM             35.0f
/* At or below this distance, stop ESC and keep steering centered. */
#define NXP_CUP_ULTRA_CRAWL_STOP_CM       10.0f
/* After the hold, the car resumes at this exact logical ESC command.
   -8 here becomes physical -14 after ESC_TRUE_NEUTRAL_CMD = -6 is applied. */
#define NXP_CUP_ULTRA_CRAWL_LOGICAL_CMD   (-8)
/* Hold time at zero speed after the trigger before crawl resumes. */
#define NXP_CUP_ULTRA_STOP_HOLD_MS        350u

/* Keep ramp-down effectively immediate when the ultrasonic state machine asks for 0. */
#define NXP_CUP_RAMP_DOWN_STEP_PCT        100

