#include "vision_linear.h"

uint8 Vision_FindBlackRegions(const uint8 *buff,
                              BlackRegion *regions,
                              uint8 maxRegions)
{
    uint8 regionIndex = 0U;
    uint8 inRegion    = 0U;
    uint16 i;

    if ((buff == NULL_PTR) || (regions == NULL_PTR) || (maxRegions == 0U))
    {
        return 0U;
    }

    /* scan from 1 .. VISION_BUFFER_SIZE-2, keeping 0 and last for safety */
    for (i = 1U; i < (VISION_BUFFER_SIZE - 1U); i++)
    {
        if (buff[i] < VISION_THRESHOLD)
        {
            if (inRegion == 0U)
            {
                inRegion = 1U;
                regions[regionIndex].start = (uint8)i;
            }
            regions[regionIndex].end = (uint8)i;
        }
        else
        {
            if (inRegion != 0U)
            {
                inRegion = 0U;
                regionIndex++;

                /* stop if we filled maxRegions */
                if (regionIndex >= maxRegions)
                {
                    break;
                }
            }
        }
    }

    /* If buffer ends while still in a black region, close it */
    if ((inRegion != 0U) && (regionIndex < maxRegions))
    {
        regions[regionIndex].end = (uint8)(VISION_BUFFER_SIZE - 1U);
        regionIndex++;
    }

    return regionIndex;
}

float Vision_CalculateCenter(const BlackRegion *regions,
                             uint8 regionCount)
{
    uint32 totalCenter = 0U;
    uint8 i;

    /* No regions: return middle of sensor as a neutral value */
    if (regionCount == 0U)
    {
        return (float)(VISION_BUFFER_SIZE / 2U);
    }

    for (i = 0U; i < regionCount; i++)
    {
        uint32 regionCenter =
            ((uint32)regions[i].start + (uint32)regions[i].end) / 2U;

        totalCenter += regionCenter;
    }

    return (float)totalCenter / (float)regionCount;
}
