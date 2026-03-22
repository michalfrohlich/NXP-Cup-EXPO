# Steering Tuning Notes

This file is only for tuning reference. It is not compiled.

## 5050 old-equivalent visible values

These are the main `5050` values that intentionally match the older Feb-2 style path as closely as the current code allows:

- `NXP_CUP_5050_STEER_CLAMP = 100`
- `NXP_CUP_5050_STEER_LPF_ALPHA = 0.45f`
- `NXP_CUP_5050_STEER_RATE_MAX = 8`
- `NXP_CUP_5050_SPEED_PCT = 25`

`STEER_CLAMP = 100` is the effective old limit because `Steer()` in `src/servo.c` already saturates to `-100..100`.

## Smoothness logic

The camera steering path uses three layers:

1. Center dead-zone
   File: `src/app/app_modes.c`, `gNxpCupProfiles[NXP_CUP_PROFILE_5050]`
   Hidden value: `.centerErrDeadband`
   Effect: makes the car tolerate a small center error instead of correcting every tiny line movement.

2. Command dead-zone
   File: `src/app/app_modes.c`, `gNxpCupProfiles[NXP_CUP_PROFILE_5050]`
   Hidden value: `.cmdDeadband`
   Effect: small steering commands are forced to zero, which reduces servo chatter near straight.

3. Command shaping
   File: `src/app/app_modes.c`, `gNxpCupProfiles[NXP_CUP_PROFILE_5050]`
   Hidden value: `.cmdShapeBlendPct`
   Effect: small and medium steering commands are softened; large commands are still allowed.

Then the visible output smoothness is controlled mostly by:

- `NXP_CUP_5050_STEER_LPF_ALPHA`
- `NXP_CUP_5050_STEER_RATE_MAX`

## Simple tuning order

If the car still does `RLRLRL` on slight curves or mild zigzags:

1. Raise `.centerErrDeadband` by `0.01`
2. Raise `.cmdDeadband` by `1`
3. Raise `.cmdShapeBlendPct` by `10`

If the car becomes too lazy entering a real curve:

1. Lower `.centerErrDeadband` by `0.01`
2. Lower `.cmdDeadband` by `1`
3. Lower `.cmdShapeBlendPct` by `10`
4. If needed, raise `NXP_CUP_5050_STEER_RATE_MAX`

If the motion is smooth but reacts too slowly overall:

1. Raise `NXP_CUP_5050_STEER_RATE_MAX`
2. Raise `NXP_CUP_5050_STEER_LPF_ALPHA` only a little

If the car commits too hard once it decides to turn:

1. Lower `NXP_CUP_5050_KP`
2. Lower `NXP_CUP_5050_KD`
3. Lower `NXP_CUP_5050_STEER_CLAMP`

## Source ideas

The smoothing structure was based on standard control blocks, not a servo-specific magic trick:

- MathWorks Simulink: Dead Zone
- MathWorks Simulink: Rate Limiter
- WPILib: SlewRateLimiter

Why those ideas fit here:

- Dead-zone gives the car a larger "sweet spot" around center.
- Rate limiting prevents large command jumps that make the servo look clunky.
- Shaping keeps small corrections gentle without removing large steering authority.
