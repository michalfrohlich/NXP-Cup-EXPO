#pragma once

#include <stdint.h>

#include "drivers/linear_camera.h"
#include "domain/vehicle_types.h"

static constexpr uint32_t CAMERA_STREAM_MAGIC = 0x314D4354UL; /* TCM1 */
static constexpr uint16_t CAMERA_STREAM_VERSION = 2U;
static constexpr uint8_t CAMERA_STREAM_FLAG_VISION_VALID = 1U << 0;

struct __attribute__((packed)) CameraStreamPacket
{
    uint32_t magic;
    uint16_t version;
    uint16_t sampleCount;
    uint32_t sequence;
    uint32_t timestampUs;
    uint16_t values[TEENSY_LINEAR_CAMERA_PIXEL_COUNT];
    int16_t visionErrorMilli;
    uint8_t visionStatus;
    uint8_t visionFeature;
    uint8_t visionConfidence;
    uint8_t visionLeftLineIdx;
    uint8_t visionRightLineIdx;
    uint8_t visionFlags;
    uint16_t crc16;
};

void CameraStream_WriteRawFrame(const LinearCameraFrame &frame);
void CameraStream_WriteFrameVision(const LinearCameraFrame &frame,
                                   const VisionOutput &vision,
                                   bool visionValid);
