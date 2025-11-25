#ifndef CAMERA_EMULATOR_H
#define CAMERA_EMULATOR_H

#include "Mcal.h"  /* for uint8, uint16, uint32, sint16, etc. */

#define CAMERA_EMU_NUM_PIXELS  128U

/* Initialize the emulator (reset RNG, etc.) */
void CameraEmulator_Init(void);

/* Get one emulated camera frame as 128 values in [0..255].
 * Each call generates a fresh "straight line + noise" frame,
 * with no internal timing or delay.
 */
void CameraEmulator_GetFrame(uint8 Pixels[CAMERA_EMU_NUM_PIXELS]);

#endif /* CAMERA_EMULATOR_H */
