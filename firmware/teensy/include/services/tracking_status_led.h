#pragma once

#include <stdint.h>

#include "domain/vehicle_types.h"

class TrackingStatusLed
{
  public:
    void begin(bool enabled = true);
    void setEnabled(bool enabled);
    void update(uint32_t nowMs, uint32_t nowUs, bool cameraReady, bool visionAvailable,
                uint32_t visionTimestampUs, const VisionOutput &vision);

  private:
    enum class State : uint8_t
    {
        Initializing,
        CameraFault,
        LineLost,
        Centered,
        CorrectLeft,
        CorrectRight,
        Disabled,
    };

    void request(State requested, uint32_t nowMs, bool immediate);
    void apply(State state);

    State state_ = State::Disabled;
    State candidate_ = State::Disabled;
    uint32_t startedMs_ = 0U;
    uint32_t candidateSinceMs_ = 0U;
    uint32_t nextUpdateMs_ = 0U;
    bool enabled_ = false;
};
