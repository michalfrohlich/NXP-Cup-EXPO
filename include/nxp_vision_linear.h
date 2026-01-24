#ifndef NXP_VISION_LINEAR_H
#define NXP_VISION_LINEAR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Platform_Types.h" /* For uint8, etc. */

/* Configuration Macros */
#define VISION_THRESHOLD    (40U)
#define VISION_BUFFER_SIZE  (128U)

/* Data Types */
typedef struct {
    uint8 start;
    uint8 end;
} BlackRegionType;

/* Function Prototypes */

/**
 * @brief Scans the line buffer to find regions darker than the threshold.
 * @param Buff Pointer to the array of 128 pixel values.
 * @param Regions Pointer to an array where detected regions will be stored.
 * @param MaxRegions The maximum number of regions to store (buffer safety).
 * @return The number of regions found.
 */
uint8 Vision_FindBlackRegions(const uint8* Buff, BlackRegionType* Regions, uint8 MaxRegions);

/**
 * @brief Calculates the average center point of all detected black regions.
 * @param Regions Pointer to the array of detected regions.
 * @param RegionCount The number of regions to process.
 * @return The calculated floating-point center (0.0 to 127.0).
 */
float Vision_CalculateCenter(const BlackRegionType* Regions, uint8 RegionCount);

#ifdef __cplusplus
}
#endif

#endif /* NXP_EXAMPL_VISION_H */
