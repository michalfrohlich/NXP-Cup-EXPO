#pragma once

/* VehicleControlOutput_t and VisionOutput_t are defined in vehicle_types.h. */
#include "domain/vehicle_types.h"

/* =========================================================
   Controller internal state
========================================================= */
typedef struct
{
    /* PID memory */
    float i_term;          /* integral accumulator */
    float prev_error;      /* last error for derivative */
    float err_filt;        /* filtered steering error */
    float d_error;         /* filtered error used by derivative */
    float d_term;          /* filtered derivative term */

    /* Tunings, initialized from control_config.h. */
    float kp;
    float ki;
    float kd;
    float iTermClamp;

    /* Extra steering aggressiveness multiplier. */
    float steerScale;

} SteeringControllerState_t;

/* =========================================================
   API
========================================================= */
void SteeringController_Init(SteeringControllerState_t *s);
void SteeringController_Reset(SteeringControllerState_t *s);

/* Keep SAME signature as the original project so UI callers do not break. */
void SteeringController_SetTunings(SteeringControllerState_t *s, float kp, float kd, float steerScale);

VehicleControlOutput_t SteeringController_Update(SteeringControllerState_t *s,
                                        const VisionOutput_t *r,
                                        float dt_seconds,
                                        uint8 base_speed);
