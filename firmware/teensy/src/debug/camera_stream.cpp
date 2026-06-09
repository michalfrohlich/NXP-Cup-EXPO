#include "debug/camera_stream.h"

#include <Arduino.h>
#include <string.h>

#include "config/camera_config.h"
#include "config/vision_config.h"

static uint16_t crc16CcittFalse(const uint8_t *data, size_t length)
{
    uint16_t crc = 0xFFFFU;

    for (size_t i = 0U; i < length; i++)
    {
        crc ^= (uint16_t)data[i] << 8U;
        for (uint8_t bit = 0U; bit < 8U; bit++)
        {
            if ((crc & 0x8000U) != 0U)
            {
                crc = (uint16_t)((crc << 1U) ^ 0x1021U);
            }
            else
            {
                crc = (uint16_t)(crc << 1U);
            }
        }
    }

    return crc;
}

static int16_t encodeVisionErrorMilli(float error)
{
    int32_t errorMilli = (int32_t)(error * 1000.0f);

    if (errorMilli > 32767L)
    {
        errorMilli = 32767L;
    }
    else if (errorMilli < -32768L)
    {
        errorMilli = -32768L;
    }

    return (int16_t)errorMilli;
}

static void fillPacket(CameraStreamPacket &packet,
                       const LinearCameraFrame &frame,
                       const VisionOutput &vision,
                       bool visionValid)
{
    packet.magic = CAMERA_STREAM_MAGIC;
    packet.version = CAMERA_STREAM_VERSION;
    packet.sampleCount = TEENSY_LINEAR_CAMERA_PIXEL_COUNT;
    packet.sequence = frame.sequence;
    packet.timestampUs = frame.timestampUs;
    memcpy(packet.values, frame.values, sizeof(packet.values));

    if (visionValid == true)
    {
        packet.visionErrorMilli = encodeVisionErrorMilli(vision.error);
        packet.visionStatus = (uint8_t)vision.status;
        packet.visionFeature = (uint8_t)vision.feature;
        packet.visionConfidence = vision.confidence;
        packet.visionLeftLineIdx = vision.leftLineIdx;
        packet.visionRightLineIdx = vision.rightLineIdx;
        packet.visionFlags = CAMERA_STREAM_FLAG_VISION_VALID;
    }
    else
    {
        packet.visionErrorMilli = 0;
        packet.visionStatus = (uint8_t)VISION_TRACK_LOST;
        packet.visionFeature = (uint8_t)VISION_FEATURE_NONE;
        packet.visionConfidence = 0U;
        packet.visionLeftLineIdx = VISION_LINEAR_INVALID_IDX;
        packet.visionRightLineIdx = VISION_LINEAR_INVALID_IDX;
        packet.visionFlags = 0U;
    }

    packet.crc16 = crc16CcittFalse(
        ((const uint8_t *)&packet) + sizeof(packet.magic),
        sizeof(packet) - sizeof(packet.magic) - sizeof(packet.crc16));
}

void CameraStream_WriteRawFrame(const LinearCameraFrame &frame)
{
    VisionOutput vision = {};
    CameraStream_WriteFrameVision(frame, vision, false);
}

void CameraStream_WriteFrameVision(const LinearCameraFrame &frame,
                                   const VisionOutput &vision,
                                   bool visionValid)
{
    if (!Serial)
    {
        return;
    }

    CameraStreamPacket packet = {};
    fillPacket(packet, frame, vision, visionValid);
    Serial.write((const uint8_t *)&packet, sizeof(packet));
}
