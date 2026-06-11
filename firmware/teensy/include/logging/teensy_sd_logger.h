#pragma once

#include <Arduino.h>
#include <SdFat.h>
#include <RingBuf.h>

#include "telemetry/teensy_link_telemetry.h"

/*
  Teensy 4.1 built-in SD card logger (SdFat, FIFO SDIO).

  How it stays out of the SPI link's way:
  - logRow() only formats text into a RAM ring buffer. No SD access.
  - service() moves at most one 512-byte chunk per call to the card,
    and only when the card reports it is not busy.
  - Both are called from loop() only. Never from interrupts and never
    from the SPI slave transfer path.
  - If the SD card is missing or breaks, the logger goes inactive and
    the rest of the firmware keeps running. Rows that do not fit are
    counted in dropCount() instead of blocking.
*/
class TeensySdLogger
{
public:
    /* Mount the card, pick a fresh LOGnnn.CSV name, write the CSV
       header. Returns false (and stays inactive) if no card. */
    bool begin();

    /* Queue one CSV row built from the latest link state.
       Cheap: only RAM writes. Safe to call at sensor rate. */
    void logRow(uint32_t nowMs,
                uint16_t txSeq,
                const TeensyLinkTelemetryInputs &telemetry,
                const TeensyLinkS32kSnapshot &s32k,
                uint16_t rxFrames,
                uint16_t rxErrors,
                uint16_t timeouts);

    /* Push buffered bytes to the card in small chunks and sync the
       file once in a while. Call every loop() pass, ideally while the
       SPI chip-select line is idle. */
    void service(uint32_t nowMs);

    /* Flush everything and truncate the file. Optional; normal use is
       "pull the power", which loses at most the last sync interval. */
    void end();

    bool isReady() const { return ready_; }
    bool hasError() const { return error_; }
    uint16_t dropCount() const { return dropCount_; }
    const char *fileName() const { return fileName_; }
    uint32_t bytesWritten() const { return bytesWritten_; }

    /* Status bits for the shared SPI frame (TEENSY_LINK_LOGGER_FLAG_*). */
    uint16_t linkLoggerFlags() const;

private:
    bool openNextFile();
    void writeHeader();

    SdFs sd_;
    FsFile file_;
    /* 32 KiB of buffered rows is about 1.5 s at 100 Hz, enough to ride
       out slow SD moments without dropping. */
    RingBuf<FsFile, 32768> ringBuf_;

    char fileName_[16] = {0};
    uint32_t nextSyncMs_ = 0;
    uint32_t bytesWritten_ = 0;
    uint16_t logSeq_ = 0;
    uint16_t dropCount_ = 0;
    bool ready_ = false;
    bool error_ = false;
};
