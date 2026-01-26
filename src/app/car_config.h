#pragma once
/*
  car_config.h
  ============
  This file is the "control panel" for your whole project.

  ✅ If you want to change behavior, you change values here.
  ✅ If you want to run a different test, you switch the flags here.

  IMPORTANT RULE:
  - Enable ONLY ONE TEST MODE at a time (set it to 1).
  - All other test modes must be 0.
*/

/* =========================================================
   1) TEST MODE FLAGS (Enable ONE at a time)
   ---------------------------------------------------------
   These are similar to your teammate's style.

   EXAMPLES:
   - Servo calibration (no display, no esc, no camera):
       APP_TEST_SERVO_ONLY = 1
   - Ultrasonic OLED test:
       APP_TEST_ULTRASONIC_500MS = 1
   - Full line following (camera + servo + esc):
       APP_TEST_FULL_LINE_FOLLOW = 1
========================================================= */
#define APP_TEST_SERVO_ONLY            0
#define APP_TEST_MOTOR_ESC_ONLY        1
#define APP_TEST_CAMERA_ONLY           0
#define APP_TEST_ULTRASONIC_500MS      0
#define APP_TEST_FULL_LINE_FOLLOW      0


/* =========================================================
   2) BUTTON BEHAVIOR (same for all modes)
   ---------------------------------------------------------
   SW2 = START (after delay)
   SW3 = STOP immediately -> safe outputs
========================================================= */
#define START_DELAY_MS                 1000u


/* =========================================================
   3) SCHEDULER PERIODS (milliseconds)
   ---------------------------------------------------------
   "Polling tasks" like you already used.
========================================================= */
#define BUTTONS_PERIOD_MS              5u
#define CAMERA_PERIOD_MS               5u
#define CONTROL_PERIOD_MS              2u
#define DISPLAY_PERIOD_MS              100u


/* =========================================================
  /* =========================================================
   4) SERVO CONFIG (calibrated & testable) DO NOT TOUCH
   ========================================================= */

/* PWM channel used for the steering servo */
#define SERVO_PWM_CH                   1U

/* Servo PWM limits (in PWM ticks)
   RULE:  SERVO_DUTY_MIN < SERVO_DUTY_MED < SERVO_DUTY_MAX
*/
#define SERVO_DUTY_MIN                 1000U    /* full LEFT */
#define SERVO_DUTY_MED                 1500U   /* CENTER (straight wheels) */
#define SERVO_DUTY_MAX                 2000U   /* full RIGHT */

/* If steering direction is backwards, flip this: +1 -> -1 */
#define STEER_SIGN                     (+1)

/* Steering command clamp
   Steer() works in range -100 .. +100 ONLY
*/
#define STEER_CMD_CLAMP                100

/* ---------------------------------------------------------
   POTENTIOMETER CALIBRATION FOR SERVO TEST
   These define how the pot maps to steering:

   - POT_CENTER_RAW  -> wheels straight
   - POT_LEFT_RAW    -> full LEFT
   - POT_RIGHT_RAW   -> full RIGHT
--------------------------------------------------------- */
#define POT_LEFT_RAW                   0
#define POT_CENTER_RAW                 128
#define POT_RIGHT_RAW                  255

/* =========================================================
   5) ESC CONFIG (BIDIRECTIONAL TEST)
   ========================================================= */
#define ESC_PWM_CH            0U

#define ESC_DUTY_MIN          1638U   /* ~1.0ms */
#define ESC_DUTY_MED          2457U   /* ~1.5ms (NEUTRAL) */
#define ESC_DUTY_MAX          3276U   /* ~2.0ms */

#define POT_LEFT_RAW          0
#define POT_CENTER_RAW        128
#define POT_RIGHT_RAW         255

#define MOTOR_DEADBAND_PCT    6U
#define ESC_ARM_TIME_MS       3000u


/* =========================================================
   6) LINEAR CAMERA CONFIG (your known working values)
========================================================= */
#define CAM_CLK_PWM_CH                 4U
#define CAM_SHUTTER_GPT_CH             1U
#define CAM_ADC_GROUP                  0U
#define CAM_SHUTTER_PCR                97U

#define CAM_N_PIXELS                   128u
#define CAM_CENTER_PX                  63


/* =========================================================
   7) CAMERA DETECTION SETTINGS
========================================================= */

/* Basic threshold:
   pixel < threshold => black
   pixel >= threshold => white */
#define BLACK_THRESHOLD_DEFAULT        40u

/* OPTIONAL: use pot to change threshold live (helps if lighting changes).
   0 = use constant threshold
   1 = threshold from pot (20..70) */
#define USE_POT_FOR_THRESHOLD          0

/* Used when only one edge is seen, we "guess" the other edge. */
#define EXPECTED_TRACK_WIDTH_PX        88


/* =========================================================
   8) LINE LOST BEHAVIOR
   ---------------------------------------------------------
   If we lose the line:
   - keep going for LINE_LOST_COAST_MS
   - if still lost after that -> STOP to IDLE
========================================================= */
#define LINE_LOST_COAST_MS             2000u
#define SPEED_LOST_LINE                20


/* =========================================================
   9) INTERSECTION BEHAVIOR (simple version)
========================================================= */
#define INTERSECTION_BLACK_RATIO_PCT   35u
#define INTERSECTION_COMMIT_MS         600u
#define INTERSECTION_SPEED             22


/* =========================================================
   10) CONTROLLER (PID) SETTINGS
========================================================= */
#define KP                             2.2f
#define KD                             8.0f
#define KI                             0.0f
#define ITERM_CLAMP                    0.6f
#define STEER_LPF_ALPHA                0.35f


/* =========================================================
   11) SPEED SETTINGS
   ---------------------------------------------------------
   Pot will control base speed in full mode.
========================================================= */
#define SPEED_MIN                      18
#define SPEED_MAX                      60

/* Reduce speed when steering is large (stability) */
#define SPEED_SLOW_PER_STEER           25
