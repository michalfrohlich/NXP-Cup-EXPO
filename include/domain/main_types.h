#pragma once
/*
  main_types.h
  ============
  Shared domain packets used between modules.

  Camera frame -> vision produces line information.
  Line information -> controller produces steering and speed commands.

  Nothing here touches hardware.
*/

#ifdef __cplusplus
extern "C" {
#endif

#include "Std_Types.h"

/* Output from the vision module. */
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

/* Output from the controller module. */
typedef struct
{
    /* Steering command in Steer() units. */
    sint16 steer_cmd;

    /* Motor throttle percent (0..100). */
    uint8 throttle_pct;

    /* TRUE = brake motor, FALSE = allow driving. */
    boolean brake;
} SteeringOutput_t;

#ifdef __cplusplus
}
#endif
