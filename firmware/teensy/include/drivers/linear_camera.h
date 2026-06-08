#pragma once

#include <Arduino.h>
#include <DMAChannel.h>
#include <stdint.h>

#include "config/camera_config.h"

struct LinearCameraFrame
{
    uint16_t values[TEENSY_LINEAR_CAMERA_PIXEL_COUNT];
    uint32_t sequence;
    uint32_t timestampUs;
};

struct LinearCameraConfig
{
    uint8_t siPin;
    uint8_t clkPin;
    uint8_t analogPin;
    uint32_t pixelClockHz;
    uint32_t frameRateHz;
};

struct LinearCameraDebugCounters
{
    uint32_t frameRequestCount;
    uint32_t siPulseCount;
    uint32_t readoutWindowCount;
    uint32_t frameReadyCount;
    uint32_t droppedFrameCount;
    uint32_t adcSampleCount;
    uint32_t adcErrorCount;
    uint32_t dmaFrameCount;
    uint32_t dmaErrorCount;
    uint32_t timingOverrunCount;
    uint32_t lastCaptureUs;
    uint32_t maxCaptureUs;
};

enum class LinearCameraStatus : uint8_t
{
    Idle = 0,
    Running,
    Fault
};

class LinearCamera
{
public:
    bool begin(const LinearCameraConfig &config);
    bool start();
    void stop();
    void service(uint32_t nowUs);

    bool getLatestFrame(LinearCameraFrame &outFrame);
    LinearCameraStatus status() const;
    bool isRunning() const;
    bool isReadoutActive() const;
    void getDebugCounters(LinearCameraDebugCounters &outCounters) const;

    uint32_t pixelClockHz() const;
    uint32_t frameRateHz() const;
    uint32_t framePeriodUs() const;
    uint32_t pixelClockPeriodUs() const;
    uint32_t readoutWindowUs() const;

private:
    enum class CapturePhase : uint8_t
    {
        Idle = 0,
        WaitingForFrame,
        ReadoutWindow
    };

    bool configIsValid(const LinearCameraConfig &config) const;
    void configurePins();
    void stopPixelClock();
    void configureAdc();
    void requestFrame(uint32_t nowUs);
    void serviceReadoutWindow(uint32_t nowUs);
    void finishReadoutWindow(uint32_t nowUs);
    void publishCapturedFrame(uint32_t nowUs);
    bool configureAdcEtcTrigger();
    bool configureAdcDma();
    void prepareAdcDmaTransfer();
    void configurePixelClockTrigger();
    void startPixelClock();
    void clearAdcEtcFlags();
    bool startFrameHardwareTriggered(uint32_t nowUs);
    bool completeFrameHardwareTriggered(uint32_t nowUs);
    void abortFrameHardwareTriggered(uint32_t nowUs);

    LinearCameraConfig config_ = {};
    LinearCameraStatus status_ = LinearCameraStatus::Idle;
    CapturePhase phase_ = CapturePhase::Idle;
    LinearCameraDebugCounters counters_ = {};

    uint32_t nextFrameUs_ = 0U;
    uint32_t readoutStartUs_ = 0U;
    uint32_t readoutTimeoutUs_ = 0U;
    uint32_t sequence_ = 0U;
    bool initialized_ = false;
    bool latestFrameReady_ = false;
    bool adcEtcReady_ = false;
    bool adcDmaReady_ = false;
    uint8_t adcChannel_ = 0U;
    DMAChannel adcDma_{false};
    LinearCameraFrame frameBuffers_[2] = {};
    uint8_t writeBufferIndex_ = 0U;
    uint8_t readyBufferIndex_ = 0U;
};
