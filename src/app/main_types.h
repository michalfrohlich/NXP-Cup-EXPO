#pragma once
/*
  main_types.h
  ============
  This file defines the "shared structs" (data packets) used between modules.

  Think of it like:
    Camera frame -> Vision module produces LINE INFO
    Line info -> Controller module produces STEER + SPEED
    Main uses steer + speed to drive the servo + motor

  Nothing here touches hardware. It's just definitions.
*/

#ifdef __cplusplus
extern "C" {
#endif

#include "Std_Types.h"

/* =========================================================
   Car state (your app state machine)
   ---------------------------------------------------------
   - IDLE: stopped, waiting for SW2
   - ARMING: waiting START_DELAY_MS
   - RUN: driving
   - INTERSECTION_COMMIT: special behavior for intersections
========================================================= */
typedef enum
{
    CAR_INIT = 0,
    CAR_IDLE,
    CAR_ARMING,
    CAR_RUN,
    CAR_INTERSECTION_COMMIT,
    CAR_ERROR
} CarMode_t;


/* =========================================================
   Output from the VISION module (camera -> lane info)
   ---------------------------------------------------------
   Shared packet filled by vision and consumed by steering/app modes.
========================================================= */
typedef enum
{
    VISION_TRACK_LOST  = 0U,
    VISION_TRACK_BOTH  = 1U,
    VISION_TRACK_LEFT  = 2U,
    VISION_TRACK_RIGHT = 3U
} VisionTrackStatus_t;

typedef enum
{
    VISION_FEATURE_NONE        = 0U,
    VISION_FEATURE_FINISH_LINE = 1U
} VisionFeature_t;

typedef struct
{
    /* Normalized steering error in [-1..+1]. */
    float error;

    /* Track detection state. */
    VisionTrackStatus_t status;

    /* Optional detected track feature. */
    VisionFeature_t feature;

    /* Confidence 0..100 for the current frame. */
    uint8 confidence;

    /* Selected left/right lane edges in the effective vision window.
       255 means "not found". */
    uint8 leftLineIdx;
    uint8 rightLineIdx;
} VisionOutput_t;


/* =========================================================
   Output from the CONTROLLER module (vision -> control)
   ---------------------------------------------------------
   This tells the MAIN what it should do with the actuators.
========================================================= */
typedef struct
{
    /* Steering command in "Steer()" units.
       We clamp it to [-STEER_CMD_CLAMP .. +STEER_CMD_CLAMP]. */
    sint16 steer_cmd;

    /* Motor throttle percent (0..100) */
    uint8 throttle_pct;

    /* TRUE = brake motor (stop fast)
       FALSE = allow driving */
    boolean brake;

} SteeringOutput_t;

#ifdef __cplusplus
}
#endif
