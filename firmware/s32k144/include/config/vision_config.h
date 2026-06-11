#pragma once
/*
  Vision V2 line-detection tunables.

  Camera geometry/timing lives in sensor_config.h. These values tune the
  software detector and are shared by app debug code and vision processing.
*/

#include "config/sensor_config.h"

#define VISION_LINEAR_BUFFER_SIZE         (CAM_EFFECTIVE_PIXELS)

#define VISION_LINEAR_CONF_CONTRAST_THRESHOLD  (450U)
#define VISION_LINEAR_CONF_EDGE_LOW            (70U)
#define VISION_LINEAR_CONF_EDGE_HIGH           (220U)
#define VISION_LINEAR_CONF_EDGE_WEIGHT_PCT     (80U)
#define VISION_LINEAR_CONF_CONTRAST_WEIGHT_PCT (20U)

/* Line detector candidate buffer sizes.
   Increase if the scene can legitimately contain more edge/region candidates. */
#define VLIN_MAX_EDGE_CANDIDATES          12U

/* Minimum filtered brightness span in one frame.
   Raise this to ignore weak/flat scenes more aggressively.
   Lower it if the detector drops the line in dim lighting. */
#define VISION_LINEAR_MIN_CONTRAST        650U

/* Minimum accepted weak edge magnitude.
   Raise this to reject noise and tiny reflections.
   Lower it if real line edges are too often missed. */
#define VISION_LINEAR_MIN_WEAK_EDGE       32U

/* Minimum accepted strong edge magnitude.
   This is the floor for the hysteresis "strong edge" threshold.
   Raise it to demand cleaner edges, lower it for weaker signals. */
#define VISION_LINEAR_MIN_STRONG_EDGE     40U

/* Strong edge threshold as percent of the strongest gradient in the frame.
   Higher = fewer edge candidates.
   Lower = more edge candidates. */
#define VISION_LINEAR_EDGE_HIGH_PCT       40U

/* Weak edge threshold as percent of the strong threshold.
   Higher = only candidates close to strong edges survive.
   Lower = hysteresis becomes more permissive. */
#define VISION_LINEAR_EDGE_LOW_PCT        55U

/* Expected distance between the two detected inner track edges in pixels.
   Used both for lane-pair selection and to estimate track center when only
   one edge is visible. Tune this when camera height changes. */
#define VISION_LINEAR_NOMINAL_LANE_WIDTH  90U

/* Allowed lane-width deviation around VISION_LINEAR_NOMINAL_LANE_WIDTH.
   Candidate left/right edge pairs outside this percentage window are rejected
   outright instead of being considered as a valid lane. */
#define VISION_LINEAR_LANE_WIDTH_TOL_PCT  20U

/* Single-edge recovery confidence handling.
   If only one lane edge is visible for a few consecutive frames, keep the
   current geometry estimate but reduce confidence to indicate degraded track
   quality. */
#define VISION_LINEAR_SINGLE_EDGE_STREAK_LIMIT   3U
#define VISION_LINEAR_SINGLE_EDGE_LOW_CONFIDENCE 35U

/* Keep the dynamic left/right split point away from the extreme image edges. */
#define VISION_LINEAR_SPLIT_MARGIN_PX     10U

/* Finish-line detector.
   Geometry is referenced to the inner lane width between the two detected
   black lane borders, measured approximately through the middle of the 20 mm
   border stripes:
     10 mm + 124 mm + 94 mm + 74 mm + 94 mm + 124 mm + 10 mm
   = 530 mm effective width. */
#define VISION_FINISH_INNER_WIDTH_MM      530U
#define VISION_FINISH_CENTER_GAP_MM        74U

/* Finish-line detector tolerance. */
#define VISION_FINISH_WIDTH_MIN_PCT       70U
#define VISION_FINISH_WIDTH_MAX_PCT       130U

/* Maximum allowed offset of the detected finish-gap midpoint from the lane
   midpoint, expressed as a percent of the current lane width. */
#define VISION_FINISH_CENTER_TOL_PCT      15U
