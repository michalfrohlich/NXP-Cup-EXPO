#pragma once
/*
  Shared camera geometry/timing constants.

  These values are used by the S32K camera driver and by vision processing.
  Hardware channel routing remains in src/app/car_config.h because it depends
  on generated RTD/MCAL symbols.
*/

#define CAM_FRAME_INTERVAL_TICKS          160000U

#define CAM_N_PIXELS                      128u
#define CAM_TRIM_LEFT_PX                  2u
#define CAM_TRIM_RIGHT_PX                 2u
#define CAM_EFFECTIVE_PIXELS              (CAM_N_PIXELS - CAM_TRIM_LEFT_PX - CAM_TRIM_RIGHT_PX)
