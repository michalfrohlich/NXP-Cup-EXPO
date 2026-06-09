#pragma once

#include <stdint.h>

#include "drivers/linear_camera.h"
#include "domain/vehicle_types.h"
#include "services/line_detector.h"
#include "telemetry/teensy_link_telemetry.h"

struct CameraVisionConfig
{
    LinearCameraConfig camera;
};

struct CameraVisionCounters
{
    uint32_t rawFrameCount;
    uint32_t processedFrameCount;
    uint32_t telemetryValidCount;
    uint32_t telemetryStaleCount;
};

class CameraVision
{
public:
    bool begin(const CameraVisionConfig &config);
    bool start();
    void stop();
    void service(uint32_t nowUs);

    bool getLatestVision(VisionOutput &out,
                         uint32_t &sequence,
                         uint32_t &timestampUs) const;
    bool getLatestFrameAndVision(LinearCameraFrame &frame,
                                 VisionOutput &out,
                                 uint32_t &sequence,
                                 uint32_t &timestampUs) const;
    void getTelemetryResult(TeensyLinkCameraResult &outCamera, uint32_t nowUs);
    void getCounters(CameraVisionCounters &outCounters) const;
    void getCameraDebugCounters(LinearCameraDebugCounters &outCounters) const;
    bool isCameraReadoutActive() const;

private:
    void resetLatest();
    void processFrame(const LinearCameraFrame &frame);

    CameraVisionConfig config_ = {};
    LinearCamera camera_;
    LineDetector detector_;
    LinearCameraFrame rawFrame_ = {};
    uint16_t processedPixels_[VISION_LINEAR_BUFFER_SIZE] = {};
    VisionOutput latestOutput_ = {};
    uint32_t latestSequence_ = 0U;
    uint32_t latestTimestampUs_ = 0U;
    bool initialized_ = false;
    bool latestValid_ = false;
    CameraVisionCounters counters_ = {};
};
