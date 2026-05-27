#pragma once
/*
  Servo and ESC physical calibration values for this car build.

  Generated PWM channel routing lives in config/board_config.h.
*/

#define SERVO_DUTY_MIN                    1650U
#define SERVO_DUTY_MED                    2700U
#define SERVO_DUTY_MAX                    3550U

/* ESC calibration values (timer compare) that arm correctly on this setup.
   DO NOT change these to "25% speed".
   Old attempt (did NOT arm): 409/614/819
*/
#define ESC_DUTY_MIN                      1638U   /* was: 409U */
#define ESC_DUTY_MED                      2457U   /* was: 614U */
#define ESC_DUTY_MAX                      3276U   /* was: 819U */

/* Logical ESC command that corresponds to physical neutral on this setup.
   Keep at 0 if no offset compensation is needed. */
#define ESC_TRUE_NEUTRAL_CMD              (-6)
