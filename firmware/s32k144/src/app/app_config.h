#pragma once
/*
  app_config.h
  ============
  Application build selection and behavior/profile constants.

  Board routing aliases live in config/board_config.h.
*/

#include "config/sensor_config.h"
#include "config/control_config.h"
#include "config/vision_config.h"

/* =========================================================
   BUILD MODE FLAGS (Enable EXACTLY ONE REAL MODE)
========================================================= */
#define APP_TEST_LINEAR_CAMERA_TEST 0
#define APP_TEST_ESC_TEST 0
#define APP_TEST_SERVO_RATE_TEST 0
#define APP_RACE_S32K_ONLY 0 // Race using the S32K linear camera
#define APP_TEST_TEENSY_LINK_TEST 0
#define APP_TEST_ESP_LINK_TEST 0
#define APP_TEST_NXP_CUP_TESTS 1    // Tuning and testing different tests here
#define APP_RACE 0 // Full race using Teensy camera data over SPI

#define APP_MODE_COUNT                                                                             \
    ((APP_TEST_LINEAR_CAMERA_TEST) + (APP_TEST_ESC_TEST) + (APP_RACE_S32K_ONLY) +                 \
     (APP_RACE) + (APP_TEST_SERVO_RATE_TEST) +                                                    \
     (APP_TEST_TEENSY_LINK_TEST) + (APP_TEST_ESP_LINK_TEST) +                                     \
     (APP_TEST_NXP_CUP_TESTS))

#if (APP_MODE_COUNT != 1)
#error "CONFIG ERROR: Enable EXACTLY ONE APP_TEST_* flag in app_config.h"
#endif

/* =========================================================
   Timing
========================================================= */
#define START_DELAY_MS 1000u
#define APP_BOOT_BANNER_MS 3000u
#define APP_BOOT_LED_STEP_MS 250u
#define BUTTONS_PERIOD_MS 5u
#define DISPLAY_PERIOD_MS 20u
#define CAM_DEBUG_DISPLAY_PERIOD_MS 100u
#define STEER_UPDATE_MS 10u
#define RACE_DISPLAY_PERIOD_MS 200u
#define RACE_FINISH_MIN_CONFIDENCE 50U
#define RACE_TEENSY_CAMERA_MAX_AGE_MS 100U
#define RACE_TEENSY_CAMERA_FAULT_MS 1000U
#define CAM_STEER_HOLD_MS 350u
#define CAM_UART_STREAM_PERIOD_MS 200u

/* =========================================================
   Servo test behavior
========================================================= */
#define SERVO_TEST_CMD_CLAMP 70
#define SERVO_TEST_RATE_MAX 8
#define SERVO_TEST_DEADBAND 3
#define SERVO_TEST_LPF_ALPHA 0.45f
#define SERVO_TEST_UPDATE_MS 5u

/* =========================================================
   ESC behavior
========================================================= */
#define MOTOR_DEADBAND_PCT 6U
#define ESC_ARM_TIME_MS 3000u /* Try lowering later; keep stable for now */
#define ULTRASONIC_TEST_TRIGGER_PERIOD_MS ULTRA_TRIGGER_PERIOD_MS
#define ULTRASONIC_TEST_ENABLE_DELAY_MS 2000u
#define ULTRASONIC_TEST_SLOW_DISTANCE_CM 45.0f
#define ULTRASONIC_TEST_STOP_DISTANCE_CM 8.0f
#define OBSTACLE_LOW_SPEED_PCT 5

/* =========================================================
   Camera app/test behavior
========================================================= */
#define CAM_DEBUG_UI_PERIOD_MS 5u
#define CAM_SERVO_CONTROL_PERIOD_MS 5u
#define V2_WHITE_SAT_U8 220u
#define CAM_DEBUG_PAUSE_HOLD_MS 1000U

/* Vision debug display scaling. */
#define VDBG_WHITE_MAX_FULL_SCALE (4095U)
#define VDBG_WHITE_MAX_MIN_ZOOM (400U)

/* Camera UART debug packet framing. */
#define CAM_UART_STREAM_SYNC0 (0xA5U)
#define CAM_UART_STREAM_SYNC1 (0x5AU)
#define CAM_UART_STREAM_TYPE_FRAME (0x43U)
#define CAM_UART_STREAM_PACKET_BYTES                                                               \
    (10U + (3U * VISION_LINEAR_BUFFER_SIZE) + 9U + 6U + (4U * VLIN_MAX_EDGE_CANDIDATES) + 1U)

/* Runtime bench tests */
#define TESTS_MENU_VISIBLE_LINES (3U)
#define TESTS_MENU_POT_STABLE_MS (90U)
#define TESTS_MENU_POT_HYSTERESIS (6U)
#define TESTS_MENU_SWITCH_BOOT_GUARD_MS (50U)

#define ESC_TEST_SPEED_PCT_LOW (30U)
#define ESC_TEST_SPEED_PCT_MED (50U)
#define ESC_TEST_SPEED_PCT_HIGH (100U)
#define ESC_TEST_LED_BLINK_MS (250U)

#define ULTRA_ESC_TEST_FAST_SPEED_PCT (50)
#define ULTRA_ESC_TEST_LOW_SPEED_PCT (OBSTACLE_LOW_SPEED_PCT)
#define ULTRA_ESC_TEST_SLOW_CM (45.0f)
#define ULTRA_ESC_TEST_STOP_CM (8.0f)

#define VICTORY_LAP_APPROACH_SPEED_PCT (HONOR_BASE_SPEED_PCT)
#define VICTORY_LAP_POLE_CM (8.0f)
#define VICTORY_LAP_DISPLAY_MS (100U)

#define SERVO_RATE_TEST_DISPLAY_MS (100U)
#define SERVO_RATE_TEST_FINE_STEP (1)
#define SERVO_RATE_TEST_COARSE_STEP (10)
#define SERVO_RATE_TEST_MAX_CMD (100)

#define SERIAL_TUNE_CONNECT_CHAR ('c')
#define SERIAL_TUNE_COMMIT_CHAR ('#')
#define SERIAL_TUNE_INPUT_MAX_LEN (12U)

/* =========================================================
   Full car safety
========================================================= */
#define LINE_LOST_COAST_MS 3000u
#define RACE_CAMERA_STALE_MS 500u
#define RACE_CONTROL_STALE_MS 500u
#define RACE_SERVO_TIMING_STALE_MS 500u
#define SPEED_LOST_LINE                                                                            \
    20 // I guess If the line is lost it should be zero test this because Idle mode seems to be this
       // speed but idk for now

#define FULLMODE_SLOW_SPEED_PCT 12

/* IMPORTANT:
   - POT (manual) mode should stay FULL RANGE (-100..+100)
   - FULL AUTO (Simple test drive runtime test) should be capped here
*/
#define FULL_AUTO_SPEED_PCT 25       /* was effectively: 100 (uncapped) */
#define FULL_AUTO_RAMP_STEP_PCT 2    /* was: (none) */
#define FULL_AUTO_RAMP_PERIOD_MS 20u /* was: (none) */

/* =========================================================
   Shared speed-mode profiles
========================================================= */
#define SPEED_MODE_DEFAULT_SETTING SPEED_MODE_5050

#define SPEED_MODE_SUPERFAST_KP 3.40f
#define SPEED_MODE_SUPERFAST_KD 0.88f
#define SPEED_MODE_SUPERFAST_KI 0.02f
#define SPEED_MODE_SUPERFAST_ITERM_CLAMP 0.06f
#define SPEED_MODE_SUPERFAST_STEER_CLAMP 60
#define SPEED_MODE_SUPERFAST_STEER_LPF_ALPHA 0.44f
#define SPEED_MODE_SUPERFAST_STEER_RATE_MAX 24
#define SPEED_MODE_SUPERFAST_SPEED_PCT 40

#define SPEED_MODE_5050_KP 4.5f
#define SPEED_MODE_5050_KD 2.0f
#define SPEED_MODE_5050_KI 0.03f
#define SPEED_MODE_5050_ITERM_CLAMP 0.30f
#define SPEED_MODE_5050_STEER_CLAMP 100
#define SPEED_MODE_5050_STEER_LPF_ALPHA 0.45f
#define SPEED_MODE_5050_STEER_RATE_MAX 8
#define SPEED_MODE_5050_SPEED_PCT 25

#define SPEED_MODE_SLOW_KP 0.72f
#define SPEED_MODE_SLOW_KD 0.22f
#define SPEED_MODE_SLOW_KI 0.00f
#define SPEED_MODE_SLOW_ITERM_CLAMP 0.03f
#define SPEED_MODE_SLOW_STEER_CLAMP 60
#define SPEED_MODE_SLOW_STEER_LPF_ALPHA 0.38f
#define SPEED_MODE_SLOW_STEER_RATE_MAX 6
#define SPEED_MODE_SLOW_SPEED_PCT 16

/* =========================================================
   Race honor-phase distance policy
========================================================= */
#define HONOR_BASE_SPEED_PCT 10
#define HONOR_SLOW1_SPEED_PCT OBSTACLE_LOW_SPEED_PCT
#define HONOR_SLOW2_SPEED_PCT 3
#define HONOR_STOP_SPEED_PCT 0

#define HONOR_ULTRA_TRIGGER_PERIOD_MS ULTRA_TRIGGER_PERIOD_MS
#define HONOR_SLOW1_DISTANCE_CM 45.0f
#define HONOR_SLOW2_DISTANCE_CM 15.0f
#define HONOR_STOP_DISTANCE_CM 8.0f
