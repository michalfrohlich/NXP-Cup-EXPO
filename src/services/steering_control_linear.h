#pragma once
/*
  steering_control_linear.h
  =========================
  PID controller using VisionLinear V2 output.

  IMPORTANT UNITS:
  - Vision V2:
      result.Error in [-1.0 .. +1.0]
  - Output:
      SteeringOutput_t.steer_cmd in [-STEER_CMD_CLAMP .. +STEER_CMD_CLAMP]
      (typically -100..+100) which is SERVO COMMAND UNITS.

  NOTE:
  - SteeringOutput_t is ALREADY defined in src/app/main_types.h.
    DO NOT redefine it here (that caused your compile error).
*/

#ifdef __cplusplus
extern "C" {
#endif

#include "Platform_Types.h"

/* app-level types (contains SteeringOutput_t) */
#include "../app/main_types.h"

/* config is in app/ */
#include "../app/car_config.h"

/* vision v2 is in services/ */
#include "vision_linear_v2.h"

/* Controller internal state */
typedef struct
{
    float prev_err;         /* previous error [-1..+1] */
    float i_term;           /* integral term (clamped) */
    float steer_filtered;   /* filtered steer (normalized) [-1..+1] */
} SteeringLinearState_t;

void SteeringLinear_Init(SteeringLinearState_t *s);
void SteeringLinear_Reset(SteeringLinearState_t *s);

/* Main update (VISION V2)
   Returns SteeringOutput_t from main_types.h
*/
SteeringOutput_t SteeringLinear_UpdateV2(SteeringLinearState_t *s,
                                        const VisionLinear_ResultType *r,
                                        float dt_seconds,
                                        uint8 base_speed);

#ifdef __cplusplus
}
#endif
