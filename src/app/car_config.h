#pragma once
/*
  car_config.h
  ============
  Single source of truth for build-time mode selection and tuning constants.
*/

/* =========================================================
   BUILD MODE FLAGS (Enable EXACTLY ONE)
========================================================= */
#define APP_TEST_SERVO_ONLY               0
#define APP_TEST_ESC_ONLY_WORKING         0
#define APP_TEST_VISION_V2_DEBUG          0
#define APP_TEST_CAMERA_SERVO_V2          0
#define APP_TEST_FULL_CAR_V2              0

/* NEW: combined dummy final (ESC-only + Camera+Servo) *///Which for some reason happens to work Unlike the Full Car v2 (for now)
#define APP_TEST_FINAL_DUMMY              1

#define APP_MODE_COUNT ( \
    (APP_TEST_SERVO_ONLY) + \
    (APP_TEST_ESC_ONLY_WORKING) + \
    (APP_TEST_VISION_V2_DEBUG) + \
    (APP_TEST_CAMERA_SERVO_V2) + \
    (APP_TEST_FULL_CAR_V2) + \
    (APP_TEST_FINAL_DUMMY) \
)

#if (APP_MODE_COUNT != 1)
  #error "CONFIG ERROR: Enable EXACTLY ONE APP_TEST_* flag in car_config.h"
#endif

/* =========================================================
   Ultrasonic default + runtime toggle
   ---------------------------------------------------------
   You can toggle ultrasonic in ANY mode:
   Press SW2 + SW3 at the same time.
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

/* =========================================================
   Servo
========================================================= */
#define SERVO_PWM_CH                      1U
#define SERVO_DUTY_MIN                    1000U
#define SERVO_DUTY_MED                    1500U
#define SERVO_DUTY_MAX                    2000U

#define STEER_SIGN                        (+1)
#define STEER_CMD_CLAMP                   140

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
#define ESC_DUTY_MIN   1638U   /* was: 409U */
#define ESC_DUTY_MED   2457U   /* was: 614U */
#define ESC_DUTY_MAX   3276U   /* was: 819U */

#define MOTOR_DEADBAND_PCT                6U
#define ESC_ARM_TIME_MS                   3000u /* Try lowering later; keep stable for now */

/* =========================================================
   Linear camera
========================================================= */
#define CAM_CLK_PWM_CH                    4U
#define CAM_SHUTTER_GPT_CH                1U
#define CAM_ADC_GROUP                     0U
#define CAM_SHUTTER_PCR                   97U

#define CAM_N_PIXELS                      128u
#define CAM_CENTER_PX                     63

#define BLACK_THRESHOLD_DEFAULT           40u
#define USE_POT_FOR_THRESHOLD             0
#define EXPECTED_TRACK_WIDTH_PX           88

/* =========================================================
   Full car safety
========================================================= */
#define LINE_LOST_COAST_MS                2000u
#define SPEED_LOST_LINE                   20   //I guess If the line is lost it should be zero test this because Idle mode seems to be this speed but idk for now

/* =========================================================
   PID (Vision V2 error is normalized -1..+1)
========================================================= */
/* What changing each does (quick tuning notes):

  Described below :
*/


// - KP: higher = stronger centering, too high = oscillation / weave
#define KP                                4.0f
// - KD: higher = more damping, too high = twitchy/jitter (amplifies noise)
#define KD                                1.5f
//   - KI: fixes steady drift/bias, too high = slow weave + wind-up
#define KI                                0.03f
//   - ITERM_CLAMP: caps integral; higher = more bias correction but more wind-up risk
#define ITERM_CLAMP                       0.3f
//   - STEER_LPF_ALPHA: higher = smoother steering, but adds lag (can miss corners)
#define STEER_LPF_ALPHA                   0.60f



/* =========================================================
   Speed policy
========================================================= */
#define SPEED_MIN                         18
#define SPEED_MAX                         60
#define SPEED_SLOW_PER_STEER              25

#define FULLMODE_FORCE_SLOW_SPEED         1
#define FULLMODE_SLOW_SPEED_PCT           12

/* IMPORTANT:
   - POT (manual) mode should stay FULL RANGE (-100..+100)
   - FULL AUTO (SW3 camera mode in FINAL_DUMMY) should be capped here
*/
#define FULL_AUTO_SPEED_PCT               25   /* was effectively: 100 (uncapped) */
#define FULL_AUTO_RAMP_STEP_PCT           2    /* was: (none) */
#define FULL_AUTO_RAMP_PERIOD_MS          20u  /* was: (none) */

/* =========================================================
   Vision V2 debug settings
========================================================= */
#define V2_LOOP_PERIOD_MS                 5u
#define V2_TEST_EXPOSURE_TICKS            100u
#define V2_WHITE_SAT_U8                   220u
