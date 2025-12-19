#ifndef CAMERA_EMULATOR_H
#define CAMERA_EMULATOR_H

#include "Mcal.h"  /* for uint8, uint16, uint32, sint16, etc. */
#include "Std_Types.h"

/* Number of pixels in the emulated linear camera frame */
#define CAMERA_EMU_NUM_PIXELS   (128U)

/* Initialise internal state (random generator etc.) */
void CameraEmulator_Init(void);

/* Generate one emulated frame.
 *  - Pixels: output buffer of size CAMERA_EMU_NUM_PIXELS
 *  - baseLevel: nominal brightness 0..255 that the caller chooses
 *    (e.g. from the onboard potentiometer)
 */
void CameraEmulator_GetFrame(uint8 Pixels[CAMERA_EMU_NUM_PIXELS],
                             uint8 baseLevel);

#endif /* CAMERA_EMULATOR_H */
