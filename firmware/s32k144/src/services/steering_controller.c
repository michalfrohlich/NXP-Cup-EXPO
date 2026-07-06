#include "services/steering_controller.h"
#include "config/control_config.h"

/* =========================================================
   Helpers
========================================================= */
static float clamp_f(float x, float lo, float hi)
{
    if (x < lo)
        return lo;
    if (x > hi)
        return hi;
    return x;
}

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
void SteeringController_Init(SteeringControllerState_t *s)
{
    SteeringControllerConfig_t config;

    if (s == (SteeringControllerState_t *)0)
    {
        return;
    }

    SteeringController_GetDefaultConfig(&config);
    SteeringController_ApplyConfig(s, &config);
    SteeringController_Reset(s);
}

void SteeringController_GetDefaultConfig(SteeringControllerConfig_t *config)
{
    if (config == (SteeringControllerConfig_t *)0)
    {
        return;
    }

    config->kp = (float)STEERING_DEFAULT_KP;
    config->ki = (float)STEERING_DEFAULT_KI;
    config->kd = (float)STEERING_DEFAULT_KD;
    config->iTermClamp = (float)STEERING_DEFAULT_I_TERM_CLAMP;
    config->steerScale = 1.0f;
}

void SteeringController_Reset(SteeringControllerState_t *s)
{
    if (s == (SteeringControllerState_t *)0)
    {
        return;
    }

    s->i_term = 0.0f;
    s->prev_error = 0.0f;
    s->err_filt = 0.0f;
    s->d_error = 0.0f;
    s->d_term = 0.0f;
}

/* =========================================================
   Runtime tuning
========================================================= */
void SteeringController_ApplyConfig(SteeringControllerState_t *s,
                                    const SteeringControllerConfig_t *config)
{
    if ((s == (SteeringControllerState_t *)0) || (config == (const SteeringControllerConfig_t *)0))
    {
        return;
    }

    s->kp = clamp_f(config->kp, 0.0f, 20.0f);
    s->ki = clamp_f(config->ki, 0.0f, 100.0f);
    s->kd = clamp_f(config->kd, 0.0f, 20.0f);
    s->iTermClamp = clamp_f(config->iTermClamp, 0.0f, 100.0f);
    s->steerScale = clamp_f(config->steerScale, 0.5f, 4.0f);
}

/* =========================================================
   SteeringController_Update
   ---------------------------------------------------------
   r->error is expected in [-1..+1].
   Output:
     - out.steerCmd in
   [-STEER_CMD_CLAMP..+STEER_CMD_CLAMP]
========================================================= */
SteeringControllerOutput_t SteeringController_Update(SteeringControllerState_t *s,
                                                     const VisionOutput_t *r, float dt_seconds)
{
    SteeringControllerOutput_t out;
    out.steerCmd = 0;

    if ((s == (SteeringControllerState_t *)0) || (r == (const VisionOutput_t *)0) ||
        (r->status == VISION_TRACK_LOST))
    {
        if (s != (SteeringControllerState_t *)0)
        {
            SteeringController_Reset(s);
        }

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

        if (s->i_term > s->iTermClamp)
            s->i_term = s->iTermClamp;
        if (s->i_term < -s->iTermClamp)
            s->i_term = -s->iTermClamp;
    }

    {
        float u = (s->kp * err) + (s->ki * s->i_term) + (s->kd * d);
        float cmd;

        u *= s->steerScale;
        cmd = u * (float)STEER_CMD_CLAMP;
        cmd = clamp_f(cmd, (float)(-STEER_CMD_CLAMP), (float)(+STEER_CMD_CLAMP));
        cmd *= (float)STEER_SIGN;

        out.steerCmd = (sint16)cmd;
    }

    return out;
}
