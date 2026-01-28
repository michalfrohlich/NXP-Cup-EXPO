#pragma once
/*
  car_config.h
  ============
  Single source of truth for build-time mode selection and tuning constants.

  RULE:
  - Enable EXACTLY ONE APP_TEST_* flag (set it to 1).
  - All others MUST be 0.
  - If not, the build FAILS with a #error.
*/

/* =========================================================
   1) BUILD MODE FLAGS (Enable EXACTLY ONE)
========================================================= */

/* 1) Servo only (steering) */
#define APP_TEST_SERVO_ONLY               0

/* 2) ESC only (drive motor) */
#define APP_TEST_ESC_ONLY                 0

/* 3) Camera only (CORRECT camera debug) */
#define APP_TEST_VISION_V2_DEBUG          0

/* 4) Camera + Servo (NO drive motor) */
#define APP_TEST_CAMERA_SERVO_V2          1

/* 5) Full car (ESC + Servo + Camera + controller) */
#define APP_TEST_FULL_CAR_V1              0


#define APP_MODE_COUNT ( \
    (APP_TEST_SERVO_ONLY) + \
    (APP_TEST_ESC_ONLY) + \
    (APP_TEST_VISION_V2_DEBUG) + \
    (APP_TEST_CAMERA_SERVO_V2) + \
    (APP_TEST_FULL_CAR_V1) \
)

#if (APP_MODE_COUNT != 1)
  #error "CONFIG ERROR: Enable EXACTLY ONE APP_TEST_* flag in car_config.h"
#endif


/* =========================================================
   2) BUTTON / STARTUP BEHAVIOR (for later full-car work)
========================================================= */
#define START_DELAY_MS                    1000u


/* =========================================================
   3) SCHEDULER PERIODS (milliseconds)
========================================================= */
#define BUTTONS_PERIOD_MS                 5u
#define CAMERA_PERIOD_MS                  5u
#define DISPLAY_PERIOD_MS                 20u   /* debug feels alive */

/* Servo control update in camera+servo mode (anti-jitter) */
#define STEER_UPDATE_MS                   10u


/* =========================================================
   4) SERVO CONFIG
========================================================= */
#define SERVO_PWM_CH                      1U

#define SERVO_DUTY_MIN                    1000U
#define SERVO_DUTY_MED                    1500U
#define SERVO_DUTY_MAX                    2000U

#define STEER_SIGN                        (+1)
#define STEER_CMD_CLAMP                   140 //was 100


/* =========================================================
   5) ESC CONFIG
========================================================= */
#define ESC_PWM_CH                        0U

#define ESC_DUTY_MIN                      1638U
#define ESC_DUTY_MED                      2457U
#define ESC_DUTY_MAX                      3276U

#define MOTOR_DEADBAND_PCT                6U
#define ESC_ARM_TIME_MS                   3000u


/* =========================================================
   6) LINEAR CAMERA CONFIG
========================================================= */
#define CAM_CLK_PWM_CH                    4U
#define CAM_SHUTTER_GPT_CH                1U
#define CAM_ADC_GROUP                     0U
#define CAM_SHUTTER_PCR                   97U

#define CAM_N_PIXELS                      128u
#define CAM_CENTER_PX                     63


/* =========================================================
   7) CAMERA DETECTION SETTINGS
   (keep them even if V2 doesn't use all of them)
========================================================= */
#define BLACK_THRESHOLD_DEFAULT           40u
#define USE_POT_FOR_THRESHOLD             0
#define EXPECTED_TRACK_WIDTH_PX           88


/* =========================================================
   8) FULL CAR: LINE LOST BEHAVIOR (required by services)
========================================================= */
#define LINE_LOST_COAST_MS                2000u
#define SPEED_LOST_LINE                   20


/* =========================================================
   9) FULL CAR: INTERSECTION BEHAVIOR (keep for later)
========================================================= */
#define INTERSECTION_BLACK_RATIO_PCT      35u
#define INTERSECTION_COMMIT_MS            600u
#define INTERSECTION_SPEED                22


/* =========================================================
   10) PID SETTINGS
   IMPORTANT: Vision V2 Error is normalized [-1..+1]
========================================================= */
/* ================= PID ================= */

/* MAIN steering strength (this gives bigger angles) */
#define KP                4.5f     // was 2.2 → too weak

/* D term causes jitter if too high */
#define KD                1.2f     // was 8.0 → WAY too high

/* Integral OFF for now */
#define KI                0.0f
#define ITERM_CLAMP       0.3f

/* Less smoothing = more decisive */
#define STEER_LPF_ALPHA   0.6f    // was 0.55 → too sluggish


/* =========================================================
   11) FULL CAR: SPEED POLICY SETTINGS (required by services)
========================================================= */
#define SPEED_MIN                         18
#define SPEED_MAX                         60
#define SPEED_SLOW_PER_STEER              25


/* =========================================================
   12) FULL MODE: FORCE VERY SLOW SPEED (safe testing)
========================================================= */
#define FULLMODE_FORCE_SLOW_SPEED         1
#define FULLMODE_SLOW_SPEED_PCT           12


/* =========================================================
   13) VISION V2 DEBUG SETTINGS
========================================================= */
#define V2_LOOP_PERIOD_MS                 5u
#define V2_TEST_EXPOSURE_TICKS            100u
#define V2_WHITE_SAT_U8                   220u
