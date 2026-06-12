#include "services/camera_vision.h"

#include <string.h>

#include "../../../../shared/protocol/teensy_link_protocol.h"

static int8_t encodeErrorPct(float error)
{
    int32_t errorPct = (int32_t)(error * 100.0f);

    if (errorPct > 127)
    {
        errorPct = 127;
    }
    else if (errorPct < -127)
    {
        errorPct = -127;
    }

    return (int8_t)errorPct;
}

static uint8_t clampAgeMs(uint32_t ageMs)
{
    return (ageMs > 255UL) ? 255U : (uint8_t)ageMs;
}

bool CameraVision::begin(const CameraVisionConfig &config)
{
    config_ = config;
    (void)memset(processedPixels_, 0, sizeof(processedPixels_));
    (void)memset(&counters_, 0, sizeof(counters_));
    detector_.init();
    resetLatest();

    initialized_ = camera_.begin(config_.camera);
    return initialized_;
}

bool CameraVision::start()
{
    if (initialized_ != true)
    {
        return false;
    }

    detector_.init();
    resetLatest();
    return camera_.start();
}

void CameraVision::stop()
{
    camera_.stop();
    resetLatest();
}

void CameraVision::service(uint32_t nowUs)
{
    if (initialized_ != true)
    {
        return;
    }

    camera_.service(nowUs);
    if (camera_.getLatestFrame(rawFrame_) == true)
    {
        counters_.rawFrameCount++;
        processFrame(rawFrame_);
    }
}

bool CameraVision::getLatestVision(VisionOutput &out,
                                   uint32_t &sequence,
                                   uint32_t &timestampUs) const
{
    if (latestValid_ != true)
    {
        return false;
    }

    out = latestOutput_;
    sequence = latestSequence_;
    timestampUs = latestTimestampUs_;
    return true;
}

bool CameraVision::getLatestFrameAndVision(LinearCameraFrame &frame,
                                           VisionOutput &out,
                                           uint32_t &sequence,
                                           uint32_t &timestampUs) const
{
    if (latestValid_ != true)
    {
        return false;
    }

    frame = rawFrame_;
    out = latestOutput_;
    sequence = latestSequence_;
    timestampUs = latestTimestampUs_;
    return true;
}

void CameraVision::getTelemetryResult(TeensyLinkCameraResult &outCamera, uint32_t nowUs)
{
    uint32_t ageMs = 0UL;

    if (latestValid_ == true)
    {
        ageMs = (uint32_t)((nowUs - latestTimestampUs_) / 1000UL);
    }

    if ((latestValid_ != true) || (ageMs > TEENSY_LINK_STALE_MS))
    {
        TeensyLinkTelemetry_DefaultCamera(outCamera, TEENSY_LINK_CAMERA_FLAG_SOURCE_TEENSY);
        if (latestValid_ == true)
        {
            outCamera.ageMs = clampAgeMs(ageMs);
        }
        counters_.telemetryStaleCount++;
        return;
    }

    outCamera.errorPct = encodeErrorPct(latestOutput_.error);
    outCamera.status = (uint8_t)latestOutput_.status;
    outCamera.feature = (uint8_t)latestOutput_.feature;
    outCamera.confidence = latestOutput_.confidence;
    outCamera.leftLineIdx = latestOutput_.leftLineIdx;
    outCamera.rightLineIdx = latestOutput_.rightLineIdx;
    outCamera.ageMs = clampAgeMs(ageMs);
    outCamera.flags =
        (uint8_t)(TEENSY_LINK_CAMERA_FLAG_VALID | TEENSY_LINK_CAMERA_FLAG_SOURCE_TEENSY);
    outCamera.sequence = (uint16_t)latestSequence_;
    counters_.telemetryValidCount++;
}

void CameraVision::getCounters(CameraVisionCounters &outCounters) const
{
    outCounters = counters_;
}

void CameraVision::getCameraDebugCounters(LinearCameraDebugCounters &outCounters) const
{
    camera_.getDebugCounters(outCounters);
}

bool CameraVision::isCameraReadoutActive() const
{
    return camera_.isReadoutActive();
}

void CameraVision::resetLatest()
{
    latestOutput_.error = 0.0f;
    latestOutput_.status = VISION_TRACK_LOST;
    latestOutput_.feature = VISION_FEATURE_NONE;
    latestOutput_.confidence = 0U;
    latestOutput_.leftLineIdx = VISION_LINEAR_INVALID_IDX;
    latestOutput_.rightLineIdx = VISION_LINEAR_INVALID_IDX;
    latestSequence_ = 0U;
    latestTimestampUs_ = 0U;
    latestValid_ = false;
}

void CameraVision::processFrame(const LinearCameraFrame &frame)
{
    (void)memcpy(processedPixels_,
                 &frame.values[CAM_TRIM_LEFT_PX],
                 sizeof(processedPixels_));

    detector_.process(processedPixels_, latestOutput_);
    latestSequence_ = frame.sequence;
    latestTimestampUs_ = frame.timestampUs;
    latestValid_ = true;
    counters_.processedFrameCount++;
}
