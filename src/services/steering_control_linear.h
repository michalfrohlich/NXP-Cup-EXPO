#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "Platform_Types.h"
#include "../app/main_types.h"
#include "../services/vision_linear_v2.h"

typedef struct
{
    float i_term;
    float prev_error;

    /* tunings (runtime) */
    float kp;
    float kd;
    float steerScale;
} SteeringLinearState_t;

void SteeringLinear_Init(SteeringLinearState_t *s);
void SteeringLinear_Reset(SteeringLinearState_t *s);

/* runtime tunings from UI */
void SteeringLinear_SetTunings(SteeringLinearState_t *s, float kp, float kd, float steerScale);

SteeringOutput_t SteeringLinear_UpdateV2(SteeringLinearState_t *s,
                                        const VisionLinear_ResultType *r,
                                        float dt_seconds,
                                        uint8 base_speed);

#ifdef __cplusplus
}
#endif
