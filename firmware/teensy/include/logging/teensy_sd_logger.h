#pragma once

#include <Arduino.h>
#include <RingBuf.h>
#include <SdFat.h>

#include "telemetry/teensy_link_telemetry.h"

class TeensySdLogger
{
  public:
    bool begin();
    void logRow(uint32_t nowMs, uint16_t txSeq, const TeensyLinkTelemetryInputs &telemetry,
                const TeensyLinkS32kSnapshot &s32k, uint16_t rxFrames, uint16_t rxErrors,
                uint16_t timeouts);
    bool serviceDue(uint32_t nowMs);
    void service(uint32_t nowMs);
    void end();

    bool isReady() const;
    bool hasError() const;
    uint16_t dropCount() const;
    const char *fileName() const;
    uint32_t bytesWritten() const;
    uint16_t linkLoggerFlags() const;

  private:
    bool openNextFile();
    void writeHeader();

    SdFs sd_;
    FsFile file_;
    RingBuf<FsFile, 32768> ringBuf_;
    char fileName_[16] = {};
    uint32_t nextSyncMs_ = 0U;
    uint32_t bytesWritten_ = 0U;
    uint16_t logSeq_ = 0U;
    uint16_t dropCount_ = 0U;
    bool ready_ = false;
    bool error_ = false;
};
