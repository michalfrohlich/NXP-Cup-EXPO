#pragma once
/*
  Default steering-control tunings shared by app modes and the control service.

  App modes may override SteeringLinearState_t at runtime; this header keeps the
  control service independent from src/app/car_config.h.
*/

#define STEER_SIGN                        (+1)
#define STEER_CMD_CLAMP                   140

/* PID defaults. Vision V2 error is normalized to [-1..+1]. */
#define KP                                4.5f
#define KD                                2.0f
#define KI                                0.03f
#define ITERM_CLAMP                       0.3f

/* Steering signal shaping. */
#define STEER_CENTER_ERR_DEADBAND         0.07f
#define STEER_LPF_ALPHA                   0.60f
#define STEER_ERROR_LPF_ALPHA             0.30f
#define STEER_D_INPUT_ALPHA               0.55f
#define STEER_DTERM_LPF_ALPHA             0.35f
#define STEER_DTERM_CLAMP                 4.0f

/* Default speed policy applied by SteeringLinear_UpdateV2(). */
#define SPEED_MIN                         18
#define SPEED_MAX                         60
#define SPEED_SLOW_PER_STEER              25
