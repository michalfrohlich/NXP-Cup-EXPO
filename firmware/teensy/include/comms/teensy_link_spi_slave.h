#pragma once

#include <Arduino.h>
#include <stdint.h>

#include "telemetry/teensy_link_telemetry.h"

class TeensyLinkSpiSlave
{
public:
    void begin(uint8_t csPin, uint8_t sckPin, uint8_t mosiPin, uint8_t misoPin, uint8_t readyPin);
    void publishFrame(const uint8_t frame[TEENSY_LINK_FRAME_BYTES]);
    void setReady(bool ready);
    bool service();

    bool getLatestS32kFrame(TeensyLinkS32kSnapshot &outSnapshot) const;
    uint16_t completedTransfers() const;
    uint16_t protocolErrorCount() const;
    uint16_t timeoutCount() const;

private:
    void driveBit(uint8_t txByte, uint8_t bitMask);
    void markReady(bool ready);

    uint8_t csPin_ = 10U;
    uint8_t sckPin_ = 13U;
    uint8_t mosiPin_ = 11U;
    uint8_t misoPin_ = 12U;
    uint8_t readyPin_ = 31U;

    bool ready_ = false;
    uint8_t txFrame_[TEENSY_LINK_FRAME_BYTES] = {};
    TeensyLinkS32kSnapshot latestS32k_ = {};

    uint16_t completedTransfers_ = 0U;
    uint16_t protocolErrors_ = 0U;
    uint16_t timeouts_ = 0U;
};
