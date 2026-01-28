/*
  steering_control_linear.c
  =========================
  PURE LOGIC PID controller for line following.

  Inputs:
    - error_px from vision (lane center - camera center)
  Outputs:
    - steer_cmd: -STEER_CMD_CLAMP .. +STEER_CMD_CLAMP
    - throttle_pct: 0..100
    - brake: TRUE/FALSE

  Uses constants from car_config.h (KP, KD, etc.)
*/

#include "steering_control_linear.h"

/* Config (gains, clamps, speed rules) */
#include "../app/car_config.h"

/* ---------- tiny helpers ---------- */
static float clamp_f(float v, float lo, float hi)
{
    return (v < lo) ? lo : ((v > hi) ? hi : v);
}

static sint32 abs_s32(sint32 x) { return (x < 0) ? -x : x; }

static sint32 clamp_s32(sint32 v, sint32 lo, sint32 hi)
{
    return (v < lo) ? lo : ((v > hi) ? hi : v);
}

/* ---------- API ---------- */
void SteeringLinear_Init(SteeringLinearState_t *s)
{
    s->prev_err = 0.0f;
    s->i_term = 0.0f;
    s->steer_filtered = 0.0f;
}

void SteeringLinear_Reset(SteeringLinearState_t *s)
{
    SteeringLinear_Init(s);
}

SteeringOutput_t SteeringLinear_Update(SteeringLinearState_t *s,
                                      const LinearVision_t *v,
                                      float dt,
                                      uint8 base_speed)
{
    SteeringOutput_t out;

    /* Start with a safe default (stop + brake). */
    out.steer_cmd = 0;
    out.throttle_pct = 0;
    out.brake = TRUE;

    /* ---------------------------------------------------------
       1) Line not visible?
       ---------------------------------------------------------
       If vision says invalid, we DO NOT do PID.
       We request "slow forward" and keep wheels straight.
       The APP layer handles the 2s timeout and stops if needed.
    --------------------------------------------------------- */
    if (v->valid == FALSE)
    {
        out.steer_cmd = 0;
        out.throttle_pct = (uint8)SPEED_LOST_LINE;
        out.brake = FALSE;
        return out;
    }

    /* ---------------------------------------------------------
       2) PID steering (normalized error)
       ---------------------------------------------------------
       error_px is pixels. We normalize to around [-1..+1]
       because camera is 0..127 and center is ~63.
    --------------------------------------------------------- */
    float error = ((float)v->error_px) / 63.0f;
    error = clamp_f(error, -1.0f, +1.0f);

    float dErr = (error - s->prev_err) / (dt + 1e-9f);
    s->prev_err = error;

    /* integral term */
    s->i_term += error * dt;
    s->i_term = clamp_f(s->i_term, -ITERM_CLAMP, +ITERM_CLAMP);

    /* raw pid output */
    float steer = (KP * error) + (KD * dErr) + (KI * s->i_term);
    steer = clamp_f(steer, -1.0f, +1.0f);

    /* low pass filter to reduce jitter */
    s->steer_filtered = (STEER_LPF_ALPHA * steer)
                      + ((1.0f - STEER_LPF_ALPHA) * s->steer_filtered);

    /* convert to steer command units */
    float cmdF = s->steer_filtered * (float)STEER_CMD_CLAMP * (float)STEER_SIGN;

    sint32 cmd = (sint32)cmdF;
    cmd = clamp_s32(cmd, -STEER_CMD_CLAMP, +STEER_CMD_CLAMP);

    out.steer_cmd = (sint16)cmd;
    out.brake = FALSE;

    /* ---------------------------------------------------------
       3) Speed policy
       ---------------------------------------------------------
       base_speed is given by the app (from the potentiometer),
       so you can limit power and avoid wheel slip.

       Then we reduce speed when steering is large,
       because hard turns at high speed makes the car unstable.
    --------------------------------------------------------- */
    sint32 absSteer = abs_s32(cmd);

    sint32 speed = (sint32)base_speed
                 - ((sint32)SPEED_SLOW_PER_STEER * absSteer) / 100;

    speed = clamp_s32(speed, SPEED_MIN, SPEED_MAX);

    out.throttle_pct = (uint8)speed;

    return out;
}
