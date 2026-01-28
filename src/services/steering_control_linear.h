#pragma once
/*
  steering_control_linear.h
  =========================
  PURE LOGIC controller for the LINEAR CAMERA pipeline.

  What it does:
    - Input: LinearVision_t (lane center / error / confidence)
    - Output: SteeringOutput_t (steer_cmd + throttle_pct + brake)

  What it does NOT do:
    - NO servo calls
    - NO esc calls
    - NO display calls
    - NO timebase calls

  Main.c / app layer will use the output to drive hardware.
*/

#ifdef __cplusplus
extern "C" {
#endif

#include "Std_Types.h"

/* We need the shared structs */
#include "../app/main_types.h"

/* =========================================================
   Controller internal state
   ---------------------------------------------------------
   PID needs memory:
     - previous error
     - integral sum
     - filtered steer output
========================================================= */
typedef struct
{
    float prev_err;
    float i_term;
    float steer_filtered;
} SteeringLinearState_t;

/* Initialize state once at startup */
void SteeringLinear_Init(SteeringLinearState_t *s);

/* Reset state whenever you start driving (after SW2 delay) */
void SteeringLinear_Reset(SteeringLinearState_t *s);

/* Main controller update function
   - v: vision output from VisionLinear_Process()
   - dt_seconds: control loop period in seconds (ex: 0.002 for 2ms)
   - base_speed: main gives base speed (from pot) so user can limit power
*/
SteeringOutput_t SteeringLinear_Update(SteeringLinearState_t *s,
                                      const LinearVision_t *v,
                                      float dt_seconds,
                                      uint8 base_speed);

#ifdef __cplusplus
}
#endif
