#pragma once

/* IMPORTANT:
   SteeringOutput_t is already defined in main_types.h.
   Do NOT redefine it here, or you get "conflicting types" errors.
*/
#include "../app/main_types.h"

/* Vision output type (VisionLinear_ResultType) */
#include "vision_linear_v2.h"

/* =========================================================
   Controller internal state
========================================================= */
typedef struct
{
    /* PID memory */
    float i_term;          /* integral accumulator */
    float prev_error;      /* last error for derivative */

    /* Tunings (defaults come from car_config.h in Init) */
    float kp;
    float ki;
    float kd;

    /* Extra steering aggressiveness multiplier */
    float steerScale;

} SteeringLinearState_t;

/* =========================================================
   API
========================================================= */
void SteeringLinear_Init(SteeringLinearState_t *s);
void SteeringLinear_Reset(SteeringLinearState_t *s);

/* Keep SAME signature as your original project (so UI won’t break) */
void SteeringLinear_SetTunings(SteeringLinearState_t *s, float kp, float kd, float steerScale);

SteeringOutput_t SteeringLinear_UpdateV2(SteeringLinearState_t *s,
                                        const VisionLinear_ResultType *r,
                                        float dt_seconds,
                                        uint8 base_speed);
