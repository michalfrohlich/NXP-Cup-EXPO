#include "services/teensy_camera_source.h"

#include "config/vision_config.h"

#define TEENSY_CAMERA_SOURCE_COMPONENT_CAMERA0  ((uint16)(1U << 2U))
#define TEENSY_CAMERA_SOURCE_INVALID_INDEX      ((uint8)255U)

static boolean teensy_camera_source_index_valid(uint8 index)
{
    return (boolean)((index == TEENSY_CAMERA_SOURCE_INVALID_INDEX) ||
                     (index < (uint8)VISION_LINEAR_BUFFER_SIZE));
}

boolean TeensyCameraSource_GetCamera0Vision(const TeensyLinkSnapshot_t *snapshot,
                                            uint32 nowMs,
                                            uint32 maxAgeMs,
                                            VisionOutput_t *outVision)
{
    const TeensyLinkCameraResult_t *camera;
    uint8 requiredFlags =
        (uint8)(TEENSY_LINK_CAMERA_FLAG_VALID |
                TEENSY_LINK_CAMERA_FLAG_SOURCE_TEENSY);

    if ((snapshot == NULL_PTR) || (outVision == NULL_PTR))
    {
        return FALSE;
    }

    if ((snapshot->haveFrame != TRUE) ||
        (snapshot->live != TRUE) ||
        ((uint32)(nowMs - snapshot->lastRxMs) > maxAgeMs) ||
        ((snapshot->componentMask & TEENSY_CAMERA_SOURCE_COMPONENT_CAMERA0) == 0U))
    {
        return FALSE;
    }

    camera = &snapshot->camera[0];

    if (((camera->flags & requiredFlags) != requiredFlags) ||
        ((camera->flags & (uint8)TEENSY_LINK_CAMERA_FLAG_STALE) != 0U) ||
        ((uint32)camera->ageMs > maxAgeMs) ||
        (camera->status > (uint8)VISION_TRACK_RIGHT) ||
        (camera->feature > (uint8)VISION_FEATURE_FINISH_LINE) ||
        (camera->confidence > 100U) ||
        (teensy_camera_source_index_valid(camera->leftLineIdx) != TRUE) ||
        (teensy_camera_source_index_valid(camera->rightLineIdx) != TRUE))
    {
        return FALSE;
    }

    outVision->error = (float)camera->errorPct * 0.01f;
    outVision->status = (VisionTrackStatus_t)camera->status;
    outVision->feature = (VisionFeature_t)camera->feature;
    outVision->confidence = camera->confidence;
    outVision->leftLineIdx = camera->leftLineIdx;
    outVision->rightLineIdx = camera->rightLineIdx;
    return TRUE;
}
