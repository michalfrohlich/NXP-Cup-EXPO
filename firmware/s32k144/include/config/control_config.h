#pragma once
/*
  Steering-control tunings shared by app modes and the control service.

  App modes apply runtime profiles through SteeringController_ApplyConfig(); this
  header keeps the control service independent from app-private configuration.
*/

#define STEER_SIGN (+1)
#define STEER_CMD_CLAMP 140

/* PID baseline. Vision V2 error is normalized to [-1..+1]. */
#define STEERING_DEFAULT_KP 4.5f
#define STEERING_DEFAULT_KD 2.0f
#define STEERING_DEFAULT_KI 0.03f
#define STEERING_DEFAULT_I_TERM_CLAMP 0.3f

/* Steering signal shaping. */
#define STEER_CENTER_ERR_DEADBAND 0.07f
#define STEER_ERROR_LPF_ALPHA 0.30f
#define STEER_D_INPUT_ALPHA 0.55f
#define STEER_DTERM_LPF_ALPHA 0.35f
#define STEER_DTERM_CLAMP 4.0f
