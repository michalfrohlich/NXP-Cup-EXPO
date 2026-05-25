#pragma once
/*
  car_config.h
  ============
  Build-time mode selection plus app-level hardware aliases and profile constants.
*/

#include "config/camera_config.h"
#include "config/control_defaults.h"
#include "config/vision_config.h"
#include "Pwm_Cfg.h"

/* =========================================================
   BUILD MODE FLAGS (Enable EXACTLY ONE REAL MODE)
========================================================= */
#define APP_TEST_LINEAR_CAMERA_TEST       0
#define APP_TEST_NXP_CUP                  0
#define APP_TEST_RACE_MODE                0
#define APP_TEST_HONOR_LAP                0
#define APP_TEST_SERVO_RATE_TEST          0
#define APP_TEST_NXP_CUP_TESTS            1

#define APP_MODE_COUNT ( \
    (APP_TEST_LINEAR_CAMERA_TEST) + \
    (APP_TEST_NXP_CUP) + \
    (APP_TEST_RACE_MODE) + \
    (APP_TEST_HONOR_LAP) + \
    (APP_TEST_SERVO_RATE_TEST) + \
    (APP_TEST_NXP_CUP_TESTS) \
)

#if (APP_MODE_COUNT != 1)
  #error "CONFIG ERROR: Enable EXACTLY ONE APP_TEST_* flag in car_config.h"
#endif

/* =========================================================
   Timing
========================================================= */
#define START_DELAY_MS                    1000u
#define BUTTONS_PERIOD_MS                 5u
#define DISPLAY_PERIOD_MS                 20u
#define CAM_DEBUG_DISPLAY_PERIOD_MS       100u
#define STEER_UPDATE_MS                   10u
#define RACE_DISPLAY_PERIOD_MS            200u
#define RACE_FINISH_MIN_CONFIDENCE        50U
#define CAM_STEER_HOLD_MS                 350u
#define CAM_UART_STREAM_PERIOD_MS         200u

/* =========================================================
   Teensy IMU bridge test
========================================================= */
/* S32K receives a 45-byte packet from Teensy and displays it.
   Keep the timeout short so unplugged SPI shows WAIT quickly. */
#define TEENSY_IMU_PACKET_STALE_MS        200u
#define TEENSY_IMU_TEST_DISPLAY_MS        100u

/* =========================================================
   Servo
========================================================= */
#define SERVO_PWM_CH                      Servo_Pwm
#define SERVO_DUTY_MIN                    1650U
#define SERVO_DUTY_MED                    2700U
#define SERVO_DUTY_MAX                    3550U
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
#define ESC_PWM_CH                        Esc_Pwm
#define ESC_SECOND_PWM_CH                 Esc2_Pwm
/* Generated second ESC output: FTM3 CH2 on PTB10. */

/* ESC calibration values (timer compare) that arm correctly on your setup.
   DO NOT change these to "25% speed".
   Old attempt (did NOT arm): 409/614/819
*/
#define ESC_DUTY_MIN   1638U   /* was: 409U */
#define ESC_DUTY_MED   2457U   /* was: 614U */
#define ESC_DUTY_MAX   3276U   /* was: 819U */

#define MOTOR_DEADBAND_PCT                6U
#define ESC_ARM_TIME_MS                   3000u /* Try lowering later; keep stable for now */
/* Logical ESC command that corresponds to physical neutral on your setup.
   Keep at 0 if no offset compensation is needed. */
#define ESC_TRUE_NEUTRAL_CMD              (-6)
/* Extra launch writes improve ESC command latching on some setups. */
#define ESC_LAUNCH_PULSE_COUNT            3U
#define ESC_LAUNCH_PULSE_DELAY_TICKS      30000U
#define NXP_CUP_ULTRA_TRIGGER_PERIOD_MS   1u
#define NXP_ULTRA_ENABLE_AFTER_RUN_MS     2000u
#define NXP_CUP_ULTRA_STOP_CM             35.0f
#define NXP_CUP_ULTRA_CRAWL_STOP_CM       10.0f
#define NXP_CUP_ULTRA_STOP_HOLD_MS        350u

/* =========================================================
   Linear camera
========================================================= */
/* Hardware routing for the line scan camera. */
#define CAM_CLK_PWM_CH                    LinearCamera_Clk
#define CAM_SHUTTER_GPT_CH                1U
#define CAM_ADC_GROUP                     0U
#define CAM_SHUTTER_PCR                   97U

/* Camera test / debug loop settings. */
#define CAM_DEBUG_UI_PERIOD_MS            5u
#define CAM_SERVO_CONTROL_PERIOD_MS       5u
#define V2_WHITE_SAT_U8                   220u
#define CAM_DEBUG_PAUSE_HOLD_MS           1000U

/* =========================================================
   Full car safety
========================================================= */
#define LINE_LOST_COAST_MS                2000u
#define SPEED_LOST_LINE                   20   //I guess If the line is lost it should be zero test this because Idle mode seems to be this speed but idk for now

#define FULLMODE_SLOW_SPEED_PCT           12

/* IMPORTANT:
   - POT (manual) mode should stay FULL RANGE (-100..+100)
   - FULL AUTO (Simple test drive runtime test) should be capped here
*/
#define FULL_AUTO_SPEED_PCT               25   /* was effectively: 100 (uncapped) */
#define FULL_AUTO_RAMP_STEP_PCT           2    /* was: (none) */
#define FULL_AUTO_RAMP_PERIOD_MS          20u  /* was: (none) */

/* =========================================================
   NXP Cup mode
========================================================= */
#define NXP_CUP_DEFAULT_PROFILE           1U   /* 0=SUPERFAST, 1=5050, 2=SLOW */
#define NXP_ESC_EXTRA_SETTLE_MS           1500u

#define NXP_CUP_SUPERFAST_KP              3.40f
#define NXP_CUP_SUPERFAST_KD              0.88f
#define NXP_CUP_SUPERFAST_KI              0.02f
#define NXP_CUP_SUPERFAST_ITERM_CLAMP     0.06f
#define NXP_CUP_SUPERFAST_STEER_CLAMP     60
#define NXP_CUP_SUPERFAST_STEER_LPF_ALPHA 0.44f
#define NXP_CUP_SUPERFAST_STEER_RATE_MAX  24
#define NXP_CUP_SUPERFAST_SPEED_PCT       40

#define NXP_CUP_5050_KP                   4.5f
#define NXP_CUP_5050_KD                   2.0f
#define NXP_CUP_5050_KI                   0.03f
#define NXP_CUP_5050_ITERM_CLAMP          0.30f
#define NXP_CUP_5050_STEER_CLAMP          100
#define NXP_CUP_5050_STEER_LPF_ALPHA      0.45f
#define NXP_CUP_5050_STEER_RATE_MAX       8
#define NXP_CUP_5050_SPEED_PCT            25

#define NXP_CUP_SLOW_KP                   0.72f
#define NXP_CUP_SLOW_KD                   0.22f
#define NXP_CUP_SLOW_KI                   0.00f
#define NXP_CUP_SLOW_ITERM_CLAMP          0.03f
#define NXP_CUP_SLOW_STEER_CLAMP          60
#define NXP_CUP_SLOW_STEER_LPF_ALPHA      0.38f
#define NXP_CUP_SLOW_STEER_RATE_MAX       6
#define NXP_CUP_SLOW_SPEED_PCT            16

#define NXP_CUP_ULTRASONIC_ENABLE         1
#define NXP_CUP_ULTRA_CRAWL_LOGICAL_CMD   (-8)
#define NXP_CUP_RAMP_DOWN_STEP_PCT        100

/* =========================================================
   Honor Lap mode
========================================================= */
#define HONOR_BASE_SPEED_PCT              10
#define HONOR_SLOW1_SPEED_PCT             5
#define HONOR_SLOW2_SPEED_PCT             3
#define HONOR_STOP_SPEED_PCT              0

#define HONOR_ULTRA_TRIGGER_PERIOD_MS     60u
#define HONOR_SLOW1_DISTANCE_CM           30.0f
#define HONOR_SLOW2_DISTANCE_CM           15.0f
#define HONOR_STOP_DISTANCE_CM             8.0f

/* =========================================================
   Honor Lap PID
========================================================= */
#define HONOR_KP                          3.6f
#define HONOR_KD                          1.5f
#define HONOR_KI                          0.02f
#define HONOR_STEER_SCALE                 1.00f
#define HONOR_FAKE_SPEED                  10U
