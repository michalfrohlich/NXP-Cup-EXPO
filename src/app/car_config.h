/* ============================== car_config.h ============================== */
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

/* --- Basic module tests --- */
#define APP_TEST_SERVO_ONLY               0
#define APP_TEST_MOTOR_ESC_ONLY           0
#define APP_TEST_CAMERA_ONLY_V1           0      /* VisionLinear_Process (v1) debug text */
#define APP_TEST_ULTRASONIC_500MS         0

/* --- Combined module tests (SAFE, no motor) --- */
#define APP_TEST_CAMERA_SERVO_ONLY_V1     0      /* v1 vision -> v1 controller -> servo only */
#define APP_TEST_CAMERA_SERVO_ONLY_V2     1      /* v2 vision -> adapter -> v1 controller -> servo only */

/* --- Full car mode (BLDC + ESC + Linear camera + Controller v1) --- */
#define APP_TEST_FULL_LINE_FOLLOW_V1      0      /* VERY SLOW by default */

/* --- Camera/Vision advanced debug (friend’s latest style) --- */
#define APP_TEST_VISION_V2_DEBUG          0      /* Vision v2 + debug overlay UI */

/* ---------------------------------------------------------
   Build safety: EXACTLY one mode must be enabled.
--------------------------------------------------------- */
#define APP_MODE_COUNT ( \
    (APP_TEST_SERVO_ONLY) + \
    (APP_TEST_MOTOR_ESC_ONLY) + \
    (APP_TEST_CAMERA_ONLY_V1) + \
    (APP_TEST_ULTRASONIC_500MS) + \
    (APP_TEST_CAMERA_SERVO_ONLY_V1) + \
    (APP_TEST_CAMERA_SERVO_ONLY_V2) + \
    (APP_TEST_FULL_LINE_FOLLOW_V1) + \
    (APP_TEST_VISION_V2_DEBUG) \
)

#if (APP_MODE_COUNT != 1)
  #error "CONFIG ERROR: Enable EXACTLY ONE APP_TEST_* flag in car_config.h"
#endif


/* =========================================================
   2) BUTTON BEHAVIOR
========================================================= */
#define START_DELAY_MS                 1000u


/* =========================================================
   3) SCHEDULER PERIODS (milliseconds)
========================================================= */
#define BUTTONS_PERIOD_MS              5u
#define CAMERA_PERIOD_MS               5u
#define CONTROL_PERIOD_MS              2u
#define DISPLAY_PERIOD_MS              100u


/* =========================================================
   4) SERVO CONFIG (DO NOT TOUCH)
========================================================= */
#define SERVO_PWM_CH                   1U

#define SERVO_DUTY_MIN                 1000U
#define SERVO_DUTY_MED                 1500U
#define SERVO_DUTY_MAX                 2000U

#define STEER_SIGN                     (+1)
#define STEER_CMD_CLAMP                100

#define POT_LEFT_RAW                   0
#define POT_CENTER_RAW                 128
#define POT_RIGHT_RAW                  255


/* =========================================================
   5) ESC CONFIG
========================================================= */
#define ESC_PWM_CH                     0U

#define ESC_DUTY_MIN                   1638U
#define ESC_DUTY_MED                   2457U
#define ESC_DUTY_MAX                   3276U

#define MOTOR_DEADBAND_PCT             6U
#define ESC_ARM_TIME_MS                3000u


/* =========================================================
   6) LINEAR CAMERA CONFIG
========================================================= */
#define CAM_CLK_PWM_CH                 4U
#define CAM_SHUTTER_GPT_CH             1U
#define CAM_ADC_GROUP                  0U
#define CAM_SHUTTER_PCR                97U

#define CAM_N_PIXELS                   128u
#define CAM_CENTER_PX                  63


/* =========================================================
   7) CAMERA DETECTION SETTINGS (v1 VisionLinear_Process)
========================================================= */
#define BLACK_THRESHOLD_DEFAULT        40u
#define USE_POT_FOR_THRESHOLD          0
#define EXPECTED_TRACK_WIDTH_PX        88


/* =========================================================
   8) LINE LOST BEHAVIOR (full mode)
========================================================= */
#define LINE_LOST_COAST_MS             2000u
#define SPEED_LOST_LINE                20


/* =========================================================
   9) INTERSECTION BEHAVIOR (full mode)
========================================================= */
#define INTERSECTION_BLACK_RATIO_PCT   35u
#define INTERSECTION_COMMIT_MS         600u
#define INTERSECTION_SPEED             22


/* =========================================================
   10) CONTROLLER (PD/PID) SETTINGS
========================================================= */
#define KP                             2.2f
#define KD                             8.0f
#define KI                             0.0f
#define ITERM_CLAMP                    0.6f
#define STEER_LPF_ALPHA                0.35f


/* =========================================================
   11) SPEED SETTINGS (full mode)
========================================================= */
#define SPEED_MIN                      18
#define SPEED_MAX                      60
#define SPEED_SLOW_PER_STEER           25


/* =========================================================
   12) FULL MODE: FORCE VERY SLOW SPEED (SAFE TESTING)
========================================================= */
#define FULLMODE_FORCE_SLOW_SPEED      1
#define FULLMODE_SLOW_SPEED_PCT        12


/* =========================================================
   13) VISION V2 DEBUG MODE SETTINGS
========================================================= */
#define V2_LOOP_PERIOD_MS              5u
#define V2_DISPLAY_PERIOD_MS           20u
#define V2_TEST_EXPOSURE_TICKS         100u
#define V2_WHITE_SAT_U8                220u
