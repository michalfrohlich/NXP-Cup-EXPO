/*
  steering_control_linear.c
  =========================
  PID controller for line following using Vision V2.

  Input:
    VisionLinear_ResultType.Error: normalized [-1..+1]

  Output (from main_types.h):
    SteeringOutput_t.steer_cmd: [-STEER_CMD_CLAMP..+STEER_CMD_CLAMP]
*/

#include "steering_control_linear.h"

/* helpers */
static float clamp_f(float x, float lo, float hi)
{
    if (x < lo) return lo;
    if (x > hi) return hi;
    return x;
}

static sint32 clamp_s32(sint32 x, sint32 lo, sint32 hi)
{
    if (x < lo) return lo;
    if (x > hi) return hi;
    return x;
}

static sint32 abs_s32(sint32 x) { return (x < 0) ? -x : x; }

void SteeringLinear_Init(SteeringLinearState_t *s)
{
    if (s == (SteeringLinearState_t*)0) return;
    s->prev_err = 0.0f;
    s->i_term = 0.0f;
    s->steer_filtered = 0.0f;
}

void SteeringLinear_Reset(SteeringLinearState_t *s)
{
    SteeringLinear_Init(s);
}

SteeringOutput_t SteeringLinear_UpdateV2(SteeringLinearState_t *s,
                                        const VisionLinear_ResultType *r,
                                        float dt_seconds,
                                        uint8 base_speed)
{
    SteeringOutput_t out;

    /* these field names must match your existing main_types.h */
    out.steer_cmd = 0;
    out.throttle_pct = 0u;
    out.brake = FALSE;

    if ((s == (SteeringLinearState_t*)0) || (r == (const VisionLinear_ResultType*)0))
    {
        out.brake = TRUE;
        return out;
    }

    float dt = (dt_seconds > 0.000001f) ? dt_seconds : 0.000001f;

    /* line lost */
    if (r->Status == VISION_LINEAR_LOST)
    {
        out.brake = TRUE;
        out.throttle_pct = (uint8)SPEED_LOST_LINE;
        out.steer_cmd = 0;
        return out;
    }

    /* PID on normalized error */
    float error = clamp_f(r->Error, -1.0f, +1.0f);

    float dErr = (error - s->prev_err) / dt;
    s->prev_err = error;

    /* integral */
    s->i_term += error * dt;
    s->i_term = clamp_f(s->i_term, -ITERM_CLAMP, +ITERM_CLAMP);

    float steer = (KP * error) + (KD * dErr) + (KI * s->i_term);
    steer = clamp_f(steer, -1.0f, +1.0f);

    /* Low-pass filter (reduces jitter) */
    s->steer_filtered = (STEER_LPF_ALPHA * steer)
                      + ((1.0f - STEER_LPF_ALPHA) * s->steer_filtered);

    /* normalized -> servo command units */
    {
        float cmdF = s->steer_filtered * (float)STEER_CMD_CLAMP * (float)STEER_SIGN;
        sint32 cmd = (sint32)cmdF;
        cmd = clamp_s32(cmd, -(sint32)STEER_CMD_CLAMP, +(sint32)STEER_CMD_CLAMP);
        out.steer_cmd = (sint16)cmd;
    }

    /* speed policy (used later in full-car mode) */
    {
        sint32 absSteer = abs_s32((sint32)out.steer_cmd);
        sint32 speed = (sint32)base_speed
                     - ((sint32)SPEED_SLOW_PER_STEER * absSteer) / 100;

        speed = clamp_s32(speed, SPEED_MIN, SPEED_MAX);
        out.throttle_pct = (uint8)speed;
    }

    return out;
}
