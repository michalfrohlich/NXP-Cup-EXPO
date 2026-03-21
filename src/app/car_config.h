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
#define APP_TEST_ESC_TEST                 0
#define APP_TEST_FINAL_DUMMY              0
#define APP_TEST_NXP_CUP                  1

#define APP_MODE_COUNT ( \
    (APP_TEST_LINEAR_CAMERA_TEST) + \
    (APP_TEST_RECEIVER_TEST) + \
    (APP_TEST_SERVO_TEST) + \
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
#define ULTRASONIC_DEFAULT_ON             1

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
#define SERVO_DUTY_MIN                    1650U
#define SERVO_DUTY_MED                    2700U
#define SERVO_DUTY_MAX                    3550U

#define STEER_SIGN                        (+1)
#define STEER_CMD_CLAMP                   70

/* Simple compile-time servo enable.
   1 = normal steering output
   0 = keep servo centered / no steering command applied */
#define SERVO_OUTPUT_ENABLE               1

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
#define STEER_LPF_ALPHA                   0.70f

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

Tune one profile at a time.
Change only 1 or 2 things at once.
First make the car survive the track at lower speed.
Only then push speed up.

Recommended tuning order:
1) STEER_CLAMP
2) STEER_RATE_MAX
3) KP
4) KD
5) STEER_LPF_ALPHA
6) SPEED_PCT

---------------------------------------------------------
WHAT EACH PARAMETER DOES
---------------------------------------------------------

STEER_CLAMP
- Max steering authority.
- Raise it if the car cannot make curves/intersections/loops.
- Lower it if steering becomes too extreme or snaps too hard.

STEER_RATE_MAX
- Max steering change per control update.
- Raise it if steering reacts too slowly entering curves.
- Lower it if steering becomes twitchy or jerky.

KP
- Main correction strength.
- Raise it if the car is lazy and does not correct enough.
- Lower it if the car wiggles, overreacts, or hunts left-right.

KD
- Damping.
- Raise it if the car oscillates, bounces, or overshoots after turning.
- Lower it if the car feels too damped or slow to settle.

STEER_LPF_ALPHA
- In this code: higher alpha = less smoothing = faster response.
- Lower it if steering is noisy, twitchy, or jittery.
- Raise it if steering feels too delayed or too soft.

SPEED_PCT
- Base motor speed command.
- Higher speed makes every instability worse.
- Lower speed makes tuning easier and turns easier to survive.

---------------------------------------------------------
DIAGNOSIS RULES
---------------------------------------------------------

If the car does NOT turn enough / misses curves:
- Raise STEER_CLAMP first.
- Then raise STEER_RATE_MAX.
- Then raise KP.

If the car turns, but too slowly:
- Raise STEER_RATE_MAX.

If the car turns enough, but then oscillates or bounces:
- Raise KD.

If the car wiggles on straights:
- Lower KP.
- Or lower STEER_RATE_MAX.
- Or lower STEER_LPF_ALPHA.

If the car snaps violently left-right:
- Lower STEER_RATE_MAX.
- Lower KP.
- Possibly lower STEER_LPF_ALPHA.

If the car feels smooth but too weak:
- Raise STEER_CLAMP before doing huge KP increases.

---------------------------------------------------------
SPEED EFFECT
---------------------------------------------------------

At HIGHER speed:
- Usually need more KD.
- Often need slightly less KP.
- Often need more STEER_CLAMP.
- Too much STEER_RATE_MAX becomes unstable fast.

At LOWER speed:
- Can usually tolerate more KP.
- Need less KD.
- Smoother settings can still work.

---------------------------------------------------------
PRACTICAL METHOD
---------------------------------------------------------

Step 1:
- Make the car able to do the curve at all.

Step 2:
- Remove wiggle / oscillation.

Step 3:
- Increase speed.

Step 4:
- If higher speed breaks it:
  do NOT instantly add lots of KP.
  Usually:
  - add a bit of KD
  - maybe reduce KP slightly
  - maybe raise CLAMP if it still needs authority

---------------------------------------------------------
SAFE TUNING STEPS
---------------------------------------------------------

CLAMP:
- Move in steps of about 4 to 8.

RATE_MAX:
- Move in steps of about 1.

KP:
- Move in steps of about 0.10 to 0.20.

KD:
- Move in steps of about 0.05 to 0.10.

LPF_ALPHA:
- Move in steps of about 0.03 to 0.08.

SPEED_PCT:
- Move in steps of about 2 to 4.

---------------------------------------------------------
IMPORTANT
---------------------------------------------------------

If it currently fails even the FIRST curve:
- Do NOT start by changing KD.
- First raise:
  1) STEER_CLAMP
  2) STEER_RATE_MAX
  3) KP

Only once it turns enough, then use KD to clean up bouncing.
*/



/* ---------- SUPERFAST ---------- */
#define NXP_CUP_SUPERFAST_KP              4.5f
#define NXP_CUP_SUPERFAST_KD              2.0f
#define NXP_CUP_SUPERFAST_KI              0.03f
#define NXP_CUP_SUPERFAST_ITERM_CLAMP     0.15f
#define NXP_CUP_SUPERFAST_STEER_CLAMP     100
#define NXP_CUP_SUPERFAST_STEER_LPF_ALPHA 0.7f
#define NXP_CUP_SUPERFAST_STEER_RATE_MAX  80
#define NXP_CUP_SUPERFAST_SPEED_PCT       40

/* ---------- 5050 ---------- */
#define NXP_CUP_5050_KP                   3.0f
#define NXP_CUP_5050_KD                   2.4f
#define NXP_CUP_5050_KI                   0.02f
#define NXP_CUP_5050_ITERM_CLAMP          0.10f
#define NXP_CUP_5050_STEER_CLAMP          100
#define NXP_CUP_5050_STEER_LPF_ALPHA      0.68f
#define NXP_CUP_5050_STEER_RATE_MAX       40
#define NXP_CUP_5050_SPEED_PCT            25

/* ---------- SLOW ---------- */
#define NXP_CUP_SLOW_KP                   1.0f
#define NXP_CUP_SLOW_KD                   0.5f
#define NXP_CUP_SLOW_KI                   0.01f
#define NXP_CUP_SLOW_ITERM_CLAMP          0.08f
#define NXP_CUP_SLOW_STEER_CLAMP          100
#define NXP_CUP_SLOW_STEER_LPF_ALPHA      0.6f
#define NXP_CUP_SLOW_STEER_RATE_MAX       20
#define NXP_CUP_SLOW_SPEED_PCT            14




/* ---------- NXP CUP ultrasonic behavior (per profile) ---------- */
#define NXP_CUP_ULTRASONIC_ENABLE         1
#define NXP_CUP_ULTRA_TRIGGER_PERIOD_MS   30u

/* Ignore ultrasonic briefly after run starts so it does not block launch */
#define NXP_CUP_ULTRA_START_IGNORE_MS     1000u

/* Additional hard enable delay after launch */
#define NXP_ULTRA_ENABLE_AFTER_RUN_MS     1000u

/* ---------- SUPERFAST ultrasonic ---------- */
#define NXP_CUP_SUPERFAST_ULTRA_SLOW_CM        60.0f
#define NXP_CUP_SUPERFAST_ULTRA_SLOW_SPEED_PCT 2U
#define NXP_CUP_SUPERFAST_ULTRA_STOP_CM        18.0f
#define NXP_CUP_SUPERFAST_ULTRA_STOP_RELEASE_CM 20.0f

/* ---------- 5050 ultrasonic ---------- */
#define NXP_CUP_5050_ULTRA_SLOW_CM             40.0f
#define NXP_CUP_5050_ULTRA_SLOW_SPEED_PCT      2U
#define NXP_CUP_5050_ULTRA_STOP_CM             10.0f
#define NXP_CUP_5050_ULTRA_STOP_RELEASE_CM     12.0f

/* ---------- SLOW ultrasonic ---------- */
#define NXP_CUP_SLOW_ULTRA_SLOW_CM             20.0f
#define NXP_CUP_SLOW_ULTRA_SLOW_SPEED_PCT      2U
#define NXP_CUP_SLOW_ULTRA_STOP_CM             10.0f
#define NXP_CUP_SLOW_ULTRA_STOP_RELEASE_CM     12.0f

/* Shared stop timing */
#define NXP_CUP_ULTRA_STOP_HOLD_MS        0u
#define NXP_CUP_ULTRA_STOP_LOCK_MS        2500u

/* Faster ramp-down so slow/stop reacts quickly */
#define NXP_CUP_RAMP_DOWN_STEP_PCT        100


