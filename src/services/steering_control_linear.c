#include "steering_control_linear.h"

#include "../app/car_config.h"

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
    s->i_term = 0.0f;
    s->prev_error = 0.0f;

    /* defaults (overridden by UI at runtime) */
    s->kp = 1.2f;
    s->kd = 0.1f;
    s->steerScale = 1.4f;
}

void SteeringLinear_Reset(SteeringLinearState_t *s)
{
    s->i_term = 0.0f;
    s->prev_error = 0.0f;
}

void SteeringLinear_SetTunings(SteeringLinearState_t *s, float kp, float kd, float steerScale)
{
    /* clamp for safety */
    s->kp = clamp_f(kp, 0.0f, 10.0f);
    s->kd = clamp_f(kd, 0.0f, 10.0f);
    s->steerScale = clamp_f(steerScale, 0.5f, 4.0f);
}

SteeringOutput_t SteeringLinear_UpdateV2(SteeringLinearState_t *s,
                                        const VisionLinear_ResultType *r,
                                        float dt_seconds,
                                        uint8 base_speed)
{
    SteeringOutput_t out;
    out.brake = FALSE;
    out.throttle_pct = 0u;
    out.steer_cmd = 0;

    /* if vision says lost => brake request */
    if ((r == (const VisionLinear_ResultType*)0) || (r->Status == VISION_LINEAR_LOST))
    {
        out.brake = TRUE;
        out.throttle_pct = 0u;
        out.steer_cmd = 0;
        return out;
    }

    float err = r->Error; /* [-1..+1] */
    float d = 0.0f;

    if (dt_seconds > 0.0001f)
    {
        d = (err - s->prev_error) / dt_seconds;
    }
    s->prev_error = err;

    /* PD only (stable for your case) */
    float u = (s->kp * err) + (s->kd * d);

    /* apply extra scale so it “commits” more to one side */
    u *= s->steerScale;

    /* convert to steer command units */
    {
        float cmd = u * (float)STEER_CMD_CLAMP;
        cmd = clamp_f(cmd, (float)(-STEER_CMD_CLAMP), (float)(+STEER_CMD_CLAMP));
        out.steer_cmd = (sint16)cmd;
    }

    /* speed policy: slower when steering hard */
    {
        sint32 absSteer = abs_s32((sint32)out.steer_cmd);
        sint32 speed = (sint32)base_speed
                     - ((sint32)SPEED_SLOW_PER_STEER * absSteer) / 100;

        speed = clamp_s32(speed, SPEED_MIN, SPEED_MAX);
        out.throttle_pct = (uint8)speed;
    }

    return out;
}
