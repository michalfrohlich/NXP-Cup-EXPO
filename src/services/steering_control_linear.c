#include "services/steering_control_linear.h"
#include "config/control_defaults.h"

/* =========================================================
   Helpers
========================================================= */
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

static float lpf_f(float y_prev, float x, float alpha)
{
    alpha = clamp_f(alpha, 0.0f, 1.0f);
    return y_prev + alpha * (x - y_prev);
}

static float deadzone_f(float x, float dz)
{
    dz = clamp_f(dz, 0.0f, 0.95f);

    if (x > dz)
    {
        return (x - dz) / (1.0f - dz);
    }
    if (x < -dz)
    {
        return (x + dz) / (1.0f - dz);
    }

    return 0.0f;
}

/* =========================================================
   Init / Reset
========================================================= */
void SteeringLinear_Init(SteeringLinearState_t *s)
{
    if (s == (SteeringLinearState_t*)0) { return; }

    SteeringLinear_Reset(s);

    /* Defaults come from control_defaults.h, but app modes may override the state. */
    s->kp = (float)KP;
    s->kd = (float)KD;
    s->ki = (float)KI;
    s->iTermClamp = (float)ITERM_CLAMP;

#ifdef STEER_SCALE
    s->steerScale = (float)STEER_SCALE;
#else
    s->steerScale = 1.0f;
#endif
}

void SteeringLinear_Reset(SteeringLinearState_t *s)
{
    if (s == (SteeringLinearState_t*)0) { return; }

    s->i_term = 0.0f;
    s->prev_error = 0.0f;
    s->err_filt = 0.0f;
    s->d_error = 0.0f;
    s->d_term = 0.0f;
}

/* =========================================================
   Runtime tuning
========================================================= */
void SteeringLinear_SetTunings(SteeringLinearState_t *s, float kp, float kd, float steerScale)
{
    if (s == (SteeringLinearState_t*)0) { return; }

    s->kp = clamp_f(kp, 0.0f, 20.0f);
    s->kd = clamp_f(kd, 0.0f, 20.0f);
    s->steerScale = clamp_f(steerScale, 0.5f, 4.0f);

    /* KI and iTermClamp are stored in the state for per-profile overrides. */
}

/* =========================================================
   SteeringLinear_UpdateV2
   ---------------------------------------------------------
   r->error is expected in [-1..+1].
   Output:
     - out.steer_cmd in [-STEER_CMD_CLAMP..+STEER_CMD_CLAMP]
     - out.throttle_pct slowed down when steering hard
     - out.brake TRUE if line is lost
========================================================= */
SteeringOutput_t SteeringLinear_UpdateV2(SteeringLinearState_t *s,
                                        const VisionOutput_t *r,
                                        float dt_seconds,
                                        uint8 base_speed)
{
    SteeringOutput_t out;
    out.brake = FALSE;
    out.throttle_pct = 0u;
    out.steer_cmd = 0;

    if ((s == (SteeringLinearState_t*)0) ||
        (r == (const VisionOutput_t*)0) ||
        (r->status == VISION_TRACK_LOST))
    {
        if (s != (SteeringLinearState_t*)0)
        {
            SteeringLinear_Reset(s);
        }

        out.brake = TRUE;
        out.throttle_pct = 0u;
        out.steer_cmd = 0;
        return out;
    }

    /* Confidence-aware error conditioning before PID. */
    float raw_err = clamp_f(r->error, -1.0f, 1.0f);
    float confScale = clamp_f(((float)r->confidence) / 100.0f, 0.20f, 1.0f);
    float errAlpha = (float)STEER_ERROR_LPF_ALPHA;
    float err;

    raw_err = deadzone_f(raw_err, (float)STEER_CENTER_ERR_DEADBAND);

    if (r->status != VISION_TRACK_BOTH)
    {
        confScale *= 0.75f;
    }

    errAlpha = clamp_f(errAlpha * confScale, 0.05f, 1.0f);
    err = lpf_f(s->err_filt, raw_err, errAlpha);
    s->err_filt = err;

    /* Filter the derivative input and output to avoid one-frame steering kicks. */
    float d = 0.0f;
    float d_err;

    if (dt_seconds > 0.0001f)
    {
        d_err = lpf_f(s->d_error, err, (float)STEER_D_INPUT_ALPHA);
        d = (d_err - s->prev_error) / dt_seconds;
        d = lpf_f(s->d_term, d, (float)STEER_DTERM_LPF_ALPHA);
        d = clamp_f(d, -(float)STEER_DTERM_CLAMP, (float)STEER_DTERM_CLAMP);

        s->d_error = d_err;
        s->d_term = d;
        s->prev_error = d_err;
    }
    else
    {
        s->d_term = 0.0f;
    }

    if (dt_seconds > 0.0001f)
    {
        s->i_term += err * dt_seconds;

        if (s->i_term >  s->iTermClamp) s->i_term =  s->iTermClamp;
        if (s->i_term < -s->iTermClamp) s->i_term = -s->iTermClamp;
    }

    {
        float u = (s->kp * err) + (s->ki * s->i_term) + (s->kd * d);
        float cmd;

        u *= s->steerScale;
        cmd = u * (float)STEER_CMD_CLAMP;
        cmd = clamp_f(cmd, (float)(-STEER_CMD_CLAMP), (float)(+STEER_CMD_CLAMP));
        cmd *= (float)STEER_SIGN;

        out.steer_cmd = (sint16)cmd;
    }

    {
        sint32 absSteer = abs_s32((sint32)out.steer_cmd);
        sint32 speed = (sint32)base_speed
                     - ((sint32)SPEED_SLOW_PER_STEER * absSteer) / 100;

        speed = clamp_s32(speed, (sint32)SPEED_MIN, (sint32)SPEED_MAX);
        out.throttle_pct = (uint8)speed;
    }

    return out;
}
