#ifndef TEENSY_CAMERA_SOURCE_H
#define TEENSY_CAMERA_SOURCE_H

#include "Platform_Types.h"
#include "domain/vehicle_types.h"
#include "drivers/teensy_link.h"

#ifdef __cplusplus
extern "C" {
#endif

boolean TeensyCameraSource_GetCamera0Vision(const TeensyLinkSnapshot_t *snapshot,
                                            uint32 nowMs,
                                            uint32 maxAgeMs,
                                            VisionOutput_t *outVision);

#ifdef __cplusplus
}
#endif

#endif /* TEENSY_CAMERA_SOURCE_H */
