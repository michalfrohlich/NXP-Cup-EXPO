#include "nxp_vision_linear.h"

uint8 Vision_FindBlackRegions(const uint8* Buff, BlackRegionType* Regions, uint8 MaxRegions)
{
    uint8 RegionIndex = 0U;
    uint8 BRegFlag = 0U;
    uint16 i;

    for(i = 1U; i < (VISION_BUFFER_SIZE - 1U); i++)
    {
        if(Buff[i] < VISION_THRESHOLD)
        {
            if(BRegFlag == 0U)
            {
                BRegFlag = 1U;
                Regions[RegionIndex].start = (uint8)i;
            }
            Regions[RegionIndex].end = (uint8)i;
        }
        else
        {
            if(BRegFlag != 0U)
            {
                BRegFlag = 0U;
                RegionIndex++;
                if(RegionIndex >= MaxRegions)
                {
                    break;
                }
            }
        }
    }

    /* If the buffer ends and we are still in the black region, close it */
    if(BRegFlag != 0U)
    {
        Regions[RegionIndex].end = (uint8)(VISION_BUFFER_SIZE - 1U);
        RegionIndex++;
    }

    return RegionIndex;
}

float Vision_CalculateCenter(const BlackRegionType* Regions, uint8 RegionCount)
{
    uint32 TotalCenter = 0U;
    uint8 i;

    if (RegionCount == 0U)
    {
        /* Return center of screen if nothing found, or handle error as needed */
        return (float)(VISION_BUFFER_SIZE / 2U);
    }

    for (i = 0U; i < RegionCount; i++)
    {
        uint32 RegionCenter = ((uint32)Regions[i].start + (uint32)Regions[i].end) / 2U;
        TotalCenter += RegionCenter;
    }

    return (float)TotalCenter / (float)RegionCount;
}
