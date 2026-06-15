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
#define APP_TEST_NXP_CUP 0
#define APP_TEST_RACE_MODE 0
#define APP_TEST_TEENSY_CAM0_RACE 1
#define APP_TEST_HONOR_LAP 0
#define APP_TEST_SERVO_RATE_TEST 0
#define APP_TEST_TEENSY_LINK_TEST 0
#define APP_TEST_ESP_LINK_TEST 0
#define APP_TEST_NXP_CUP_TESTS 0

#define APP_MODE_COUNT                                                                             \
    ((APP_TEST_LINEAR_CAMERA_TEST) + (APP_TEST_NXP_CUP) + (APP_TEST_RACE_MODE) +                   \
     (APP_TEST_TEENSY_CAM0_RACE) + (APP_TEST_HONOR_LAP) + (APP_TEST_SERVO_RATE_TEST) +             \
     (APP_TEST_TEENSY_LINK_TEST) + (APP_TEST_ESP_LINK_TEST) + (APP_TEST_NXP_CUP_TESTS))

#if (APP_MODE_COUNT != 1)
#error "CONFIG ERROR: Enable EXACTLY ONE APP_TEST_* flag in app_config.h"
#endif

/* =========================================================
   Timing
========================================================= */
#define START_DELAY_MS 1000u
#define BUTTONS_PERIOD_MS 5u
#define DISPLAY_PERIOD_MS 20u
#define CAM_DEBUG_DISPLAY_PERIOD_MS 100u
#define STEER_UPDATE_MS 10u
#define RACE_DISPLAY_PERIOD_MS 200u
#define RACE_FINISH_MIN_CONFIDENCE 50U
#define TEENSY_CAM0_CONTROL_MAX_AGE_MS 100U
#define TEENSY_CAM0_HARD_FAULT_MS 1000U
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
/* Extra launch writes improve ESC command latching on some setups. */
#define ESC_LAUNCH_PULSE_COUNT 3U
#define ESC_LAUNCH_PULSE_DELAY_TICKS 30000U
#define NXP_CUP_ULTRA_TRIGGER_PERIOD_MS ULTRA_TRIGGER_PERIOD_MS
#define NXP_ULTRA_ENABLE_AFTER_RUN_MS 2000u
#define NXP_CUP_ULTRA_STOP_CM 45.0f
#define NXP_CUP_ULTRA_CRAWL_STOP_CM 8.0f
#define NXP_CUP_ULTRA_STOP_HOLD_MS 350u
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

/* =========================================================
   Runtime bench tests
========================================================= */
#define RECEIVER_REFRESH_MS (50U)

#define TESTS_MENU_VISIBLE_LINES (3U)
#define TESTS_MENU_POT_STABLE_MS (90U)
#define TESTS_MENU_POT_HYSTERESIS (6U)

#define ESC_TEST_MAX_SPEED_PCT (30U)
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
#define LINE_LOST_COAST_MS 2000u
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
   NXP Cup mode
========================================================= */
#define NXP_CUP_DEFAULT_PROFILE 1U /* 0=SUPERFAST, 1=5050, 2=SLOW */
#define NXP_ESC_EXTRA_SETTLE_MS 1500u

#define NXP_CUP_SUPERFAST_KP 3.40f
#define NXP_CUP_SUPERFAST_KD 0.88f
#define NXP_CUP_SUPERFAST_KI 0.02f
#define NXP_CUP_SUPERFAST_ITERM_CLAMP 0.06f
#define NXP_CUP_SUPERFAST_STEER_CLAMP 60
#define NXP_CUP_SUPERFAST_STEER_LPF_ALPHA 0.44f
#define NXP_CUP_SUPERFAST_STEER_RATE_MAX 24
#define NXP_CUP_SUPERFAST_SPEED_PCT 40

#define NXP_CUP_5050_KP 4.5f
#define NXP_CUP_5050_KD 2.0f
#define NXP_CUP_5050_KI 0.03f
#define NXP_CUP_5050_ITERM_CLAMP 0.30f
#define NXP_CUP_5050_STEER_CLAMP 100
#define NXP_CUP_5050_STEER_LPF_ALPHA 0.45f
#define NXP_CUP_5050_STEER_RATE_MAX 8
#define NXP_CUP_5050_SPEED_PCT 25

#define NXP_CUP_SLOW_KP 0.72f
#define NXP_CUP_SLOW_KD 0.22f
#define NXP_CUP_SLOW_KI 0.00f
#define NXP_CUP_SLOW_ITERM_CLAMP 0.03f
#define NXP_CUP_SLOW_STEER_CLAMP 60
#define NXP_CUP_SLOW_STEER_LPF_ALPHA 0.38f
#define NXP_CUP_SLOW_STEER_RATE_MAX 6
#define NXP_CUP_SLOW_SPEED_PCT 16

#define NXP_CUP_ULTRASONIC_ENABLE 1
#define NXP_CUP_ULTRA_CRAWL_LOGICAL_CMD (-(OBSTACLE_LOW_SPEED_PCT))
#define NXP_CUP_RAMP_DOWN_STEP_PCT 100

/* =========================================================
   Honor Lap mode
========================================================= */
#define HONOR_BASE_SPEED_PCT 10
#define HONOR_SLOW1_SPEED_PCT OBSTACLE_LOW_SPEED_PCT
#define HONOR_SLOW2_SPEED_PCT 3
#define HONOR_STOP_SPEED_PCT 0

#define HONOR_ULTRA_TRIGGER_PERIOD_MS ULTRA_TRIGGER_PERIOD_MS
#define HONOR_SLOW1_DISTANCE_CM 45.0f
#define HONOR_SLOW2_DISTANCE_CM 15.0f
#define HONOR_STOP_DISTANCE_CM 8.0f

/* =========================================================
   Honor Lap PID
========================================================= */
#define HONOR_KP 3.6f
#define HONOR_KD 1.5f
#define HONOR_KI 0.02f
#define HONOR_STEER_SCALE 1.00f
#define HONOR_FAKE_SPEED 10U
