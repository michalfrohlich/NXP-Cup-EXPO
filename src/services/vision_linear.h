#pragma once
/*
  vision_linear.h
  ===============
  PURE logic module (no hardware calls):
    - Input:  128-pixel camera frame (LinearCameraFrame)
    - Output: LinearVision_t (lane center, error, confidence, etc.)

  This file only describes the function and required structs.
*/

#ifdef __cplusplus
extern "C" {
#endif

#include "Std_Types.h"
#include "linear_camera.h"        /* provides LinearCameraFrame with frame->Values[128] */

/* IMPORTANT:
   Your compiler does NOT have include paths for "app/...".
   So we include using a relative path from:
       src/services/  -->  src/app/
*/
#include "../app/main_types.h"

/* Process one camera frame.
   - threshold: pixel < threshold means "black"
   - lastLaneCenter: fallback center if we have weak/no detection
*/
LinearVision_t VisionLinear_Process(const LinearCameraFrame *frame,
                                   uint8 threshold,
                                   sint16 lastLaneCenter);

#ifdef __cplusplus
}
#endif
