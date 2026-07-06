#include "services/tracking_status_led.h"

#include <Arduino.h>

#include "drivers/status_led.h"

namespace
{
constexpr uint32_t kUpdatePeriodMs = 50U;
constexpr uint32_t kMinimumStateMs = 100U;
constexpr uint32_t kCameraStaleUs = 100000U;
constexpr float kCenterEnter = 0.08f;
constexpr float kCorrectionEnter = 0.12f;
constexpr uint8_t kBrightness = 128U;
} // namespace

void TrackingStatusLed::begin(bool enabled)
{
    StatusLed_Init();
    startedMs_ = millis();
    nextUpdateMs_ = startedMs_;
    enabled_ = enabled;
    state_ = enabled ? State::Initializing : State::Disabled;
    candidate_ = state_;
    candidateSinceMs_ = startedMs_;
    apply(state_);
}

void TrackingStatusLed::setEnabled(bool enabled)
{
    if (enabled_ == enabled)
    {
        return;
    }

    enabled_ = enabled;
    startedMs_ = millis();
    request(enabled ? State::Initializing : State::Disabled, startedMs_, true);
}

void TrackingStatusLed::update(uint32_t nowMs, uint32_t nowUs, bool cameraReady,
                               bool visionAvailable, uint32_t visionTimestampUs,
                               const VisionOutput &vision)
{
    State requested;

    if (enabled_ != true)
    {
        return;
    }
    if ((int32_t)(nowMs - nextUpdateMs_) < 0)
    {
        return;
    }
    nextUpdateMs_ = nowMs + kUpdatePeriodMs;

    if (cameraReady != true)
    {
        request(State::CameraFault, nowMs, true);
        return;
    }
    if ((visionAvailable != true) || ((uint32_t)(nowUs - visionTimestampUs) > kCameraStaleUs))
    {
        if ((uint32_t)(nowMs - startedMs_) >= (kCameraStaleUs / 1000U))
        {
            request(State::CameraFault, nowMs, true);
        }
        return;
    }
    if (vision.status == VISION_TRACK_LOST)
    {
        request(State::LineLost, nowMs, false);
        return;
    }

    if ((state_ == State::CorrectLeft) && (vision.error < -kCenterEnter))
    {
        requested = State::CorrectLeft;
    }
    else if ((state_ == State::CorrectRight) && (vision.error > kCenterEnter))
    {
        requested = State::CorrectRight;
    }
    else if (vision.error < -kCorrectionEnter)
    {
        requested = State::CorrectLeft;
    }
    else if (vision.error > kCorrectionEnter)
    {
        requested = State::CorrectRight;
    }
    else
    {
        requested = State::Centered;
    }

    request(requested, nowMs, false);
}

void TrackingStatusLed::request(State requested, uint32_t nowMs, bool immediate)
{
    if (requested == state_)
    {
        candidate_ = requested;
        candidateSinceMs_ = nowMs;
        return;
    }
    if (requested != candidate_)
    {
        candidate_ = requested;
        candidateSinceMs_ = nowMs;
    }
    if ((immediate != true) && ((uint32_t)(nowMs - candidateSinceMs_) < kMinimumStateMs))
    {
        return;
    }

    state_ = requested;
    apply(state_);
}

void TrackingStatusLed::apply(State state)
{
    switch (state)
    {
        case State::Initializing:
            StatusLed_SetBrightness(StatusLedColor::White, kBrightness);
            break;
        case State::CameraFault:
            StatusLed_SetBrightness(StatusLedColor::Red, kBrightness);
            break;
        case State::LineLost:
            StatusLed_SetBrightness(StatusLedColor::White, kBrightness);
            break;
        case State::Centered:
            StatusLed_SetBrightness(StatusLedColor::Green, kBrightness);
            break;
        case State::CorrectLeft:
            StatusLed_SetRgb(0U, kBrightness, kBrightness);
            break;
        case State::CorrectRight:
            StatusLed_SetBrightness(StatusLedColor::Blue, kBrightness);
            break;
        case State::Disabled:
        default:
            StatusLed_Set(StatusLedColor::Off);
            break;
    }
}
