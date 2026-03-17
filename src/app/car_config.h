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
#define APP_TEST_FINAL_DUMMY              1

#define APP_MODE_COUNT ( \
    (APP_TEST_LINEAR_CAMERA_TEST) + \
    (APP_TEST_RECEIVER_TEST) + \
    (APP_TEST_SERVO_TEST) + \
    (APP_TEST_ESC_TEST) + \
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
/* Hardware routing for the line scan camera. */
#define CAM_CLK_PWM_CH                    4U
#define CAM_SHUTTER_GPT_CH                1U
#define CAM_ADC_GROUP                     0U
#define CAM_SHUTTER_PCR                   97U

/* Camera timing.
   CAM_SHUTTER_HIGH_TIME_TICKS controls the SI pulse width.
   CAM_FRAME_INTERVAL_TICKS controls the time between frame readouts. */
#define CAM_SHUTTER_HIGH_TIME_TICKS       100U
#define CAM_FRAME_INTERVAL_TICKS          50000U

/* Sensor geometry.
   The camera still captures all 128 physical pixels, but vision/debug use a
   symmetric centered window and ignore a couple of pixels on both edges. */
#define CAM_N_PIXELS                      128u
#define CAM_TRIM_LEFT_PX                  2u
#define CAM_TRIM_RIGHT_PX                 2u
#define CAM_EFFECTIVE_PIXELS             (CAM_N_PIXELS - CAM_TRIM_LEFT_PX - CAM_TRIM_RIGHT_PX)
#define CAM_CENTER_PX                    ((CAM_EFFECTIVE_PIXELS - 1u) / 2u)

/* Legacy threshold path; not used by the current V2 edge detector. */
#define BLACK_THRESHOLD_DEFAULT           40u
#define USE_POT_FOR_THRESHOLD             0
#define EXPECTED_TRACK_WIDTH_PX           82 //important to adjust when camera height is changed

/* Camera test / debug loop settings. */
#define V2_LOOP_PERIOD_MS                 5u
#define V2_TEST_EXPOSURE_TICKS            100u
#define V2_WHITE_SAT_U8                   220u
#define CAM_DEBUG_PAUSE_HOLD_MS           1000U

/* Line detector candidate buffer sizes.
   Increase if the scene can legitimately contain more edge/region candidates. */
#define VLIN_MAX_EDGE_CANDIDATES          12U
#define VLIN_MAX_BLACK_REGIONS            6U

/* Vision V2 line-detection tuning. */
/* Minimum filtered brightness span in one frame.
   Raise this to ignore weak/flat scenes more aggressively.
   Lower it if the detector drops the line in dim lighting. */
#define VISION_LINEAR_MIN_CONTRAST        320U

/* Minimum accepted weak edge magnitude.
   Raise this to reject noise and tiny reflections.
   Lower it if real line edges are too often missed. */
#define VISION_LINEAR_MIN_WEAK_EDGE       32U

/* Minimum accepted strong edge magnitude.
   This is the floor for the hysteresis "strong edge" threshold.
   Raise it to demand cleaner edges, lower it for weaker signals. */
#define VISION_LINEAR_MIN_STRONG_EDGE     50U

/* Strong edge threshold as percent of the strongest gradient in the frame.
   Higher = fewer edge candidates.
   Lower = more edge candidates. */
#define VISION_LINEAR_EDGE_HIGH_PCT       42U

/* Weak edge threshold as percent of the strong threshold.
   Higher = only candidates close to strong edges survive.
   Lower = hysteresis becomes more permissive. */
#define VISION_LINEAR_EDGE_LOW_PCT        55U

/* Expected distance between the two detected inner track edges in pixels.
   Used to estimate track center when only one edge is visible. */
#define VISION_LINEAR_NOMINAL_LANE_WIDTH  88U

/* Keep the dynamic left/right split point away from the extreme image edges.
   Raise this if the edge pixels are unreliable or noisy. */
#define VISION_LINEAR_SPLIT_MARGIN_PX     10U

/* Finish-line detector.
   Geometry is referenced to the inner lane width between the two detected
   black lane borders, measured approximately through the middle of the 20 mm
   border stripes:
     10 mm + 124 mm + 94 mm + 74 mm + 94 mm + 124 mm + 10 mm
   = 530 mm effective width.
   Expected finish dimensions are scaled from the currently detected lane width. */
#define VISION_FINISH_INNER_WIDTH_MM      530U
#define VISION_FINISH_BAR_WIDTH_MM         94U
#define VISION_FINISH_CENTER_GAP_MM        74U

/* Finish-line detector tolerance.
   Expected bar width and center gap are scaled from the current detected lane
   width using the geometry above. A measured width/gap is accepted if it lies within:
     expected * MIN_PCT / 100  ..  expected * MAX_PCT / 100 */
#define VISION_FINISH_WIDTH_MIN_PCT       50U
#define VISION_FINISH_WIDTH_MAX_PCT       150U

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
#define KP                                4.5f
// - KD: higher = more damping, too high = twitchy/jitter (amplifies noise)
#define KD                                2.0f
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
