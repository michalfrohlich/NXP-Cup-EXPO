#ifndef VISION_H
#define VISION_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Mcal.h"   /* for uint8, uint16, uint32, etc. */

/* Linear camera / vision constants */
#define VISION_BUFFER_SIZE   128U
#define VISION_THRESHOLD     40U

typedef struct
{
    uint8 start;
    uint8 end;
} BlackRegion;

/**
 * @brief Find contiguous "black" regions in a 1D buffer.
 *
 * A "black" pixel is defined as buff[i] < VISION_THRESHOLD.
 * Detected regions are stored into 'regions' up to maxRegions.
 *
 * @param buff        Pointer to the buffer of size VISION_BUFFER_SIZE.
 * @param regions     Output array of size >= maxRegions.
 * @param maxRegions  Maximum number of regions to store.
 *
 * @return Number of regions actually found (0..maxRegions).
 */
uint8 Vision_FindBlackRegions(const uint8 *buff,
                              BlackRegion *regions,
                              uint8 maxRegions);

/**
 * @brief Compute the average center of all detected regions.
 *
 * If regionCount == 0, this returns the middle of the sensor
 * (VISION_BUFFER_SIZE / 2).
 *
 * @param regions      Array of regions.
 * @param regionCount  Number of valid regions in the array.
 *
 * @return Average center in pixel coordinates (0..VISION_BUFFER_SIZE-1).
 */
float Vision_CalculateCenter(const BlackRegion *regions,
                             uint8 regionCount);

#ifdef __cplusplus
}
#endif

#endif /* VISION_H */
