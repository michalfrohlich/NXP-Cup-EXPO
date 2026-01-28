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
   This describes what the camera "sees".
========================================================= */
typedef struct
{
    /* TRUE = we see a line / lane edges
       FALSE = we do NOT see it (line lost) */
    boolean valid;

    /* Where the lane edges are in pixels (0..127)
       -1 means "not found" */
    sint16 left_edge_px;
    sint16 right_edge_px;

    /* Where we think the middle of the lane is (0..127)
       Goal is: lane_center_px == CAM_CENTER_PX (63) */
    sint16 lane_center_px;

    /* Error from center:
       error_px = lane_center_px - CAM_CENTER_PX
       negative = lane center is left -> steer left
       positive = lane center is right -> steer right */
    sint16 error_px;

    /* How many black "regions" were detected
       0 = nothing, 1 = only one edge, 2 = both edges, 3+ = likely intersection/noise */
    uint8 region_count;

    /* How many pixels were black (0..100 %)
       High number often means intersection or big blob */
    uint8 black_ratio_pct;

    /* Confidence 0..100
       Higher = vision is more trustworthy */
    uint8 confidence;

    /* TRUE if it looks like an intersection */
    boolean intersection_likely;

} LinearVision_t;


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
