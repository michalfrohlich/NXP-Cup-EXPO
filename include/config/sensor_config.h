#pragma once
/*
  Sensor geometry, timing, and calibration constants.

  Board channel routing lives in config/board_config.h.
*/

/* Linear camera frame timing and geometry. */
#define CAM_FRAME_INTERVAL_TICKS          160000U

#define CAM_N_PIXELS                      128u
#define CAM_TRIM_LEFT_PX                  2u
#define CAM_TRIM_RIGHT_PX                 2u
#define CAM_EFFECTIVE_PIXELS              (CAM_N_PIXELS - CAM_TRIM_LEFT_PX - CAM_TRIM_RIGHT_PX)

/* Ultrasonic sensor conversion and validity limits. */
#define ULTRA_FTM_TICK_HZ                 (2000000u) /* SIRC 8 MHz / prescaler 4 = 2 MHz */
#define ULTRA_CM_PER_TICK                 (34300.0f / (2.0f * (float)ULTRA_FTM_TICK_HZ))

#define ULTRA_MIN_DISTANCE_CM             (3.0f)
#define ULTRA_MAX_DISTANCE_CM             (400.0f)

/* 25 ms timeout: (25 ms * 34300 cm/s) / 2 = 428 cm max detection.
   At 2 MHz, this is 50,000 ticks and fits in 16-bit FTM. */
#define ULTRA_TIMEOUT_MS                  (25u)
