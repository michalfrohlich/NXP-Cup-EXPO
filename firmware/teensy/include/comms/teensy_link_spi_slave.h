#pragma once

#include <Arduino.h>
#include <stdint.h>

#include "telemetry/teensy_link_telemetry.h"

class TeensyLinkSpiSlave
{
public:
    void begin(uint8_t csPin, uint8_t sckPin, uint8_t mosiPin, uint8_t misoPin, uint8_t readyPin);
    void publishFrame(const uint8_t frame[TEENSY_LINK_FRAME_BYTES]);
    bool service();

    bool getLatestS32kFrame(TeensyLinkS32kSnapshot &outSnapshot) const;
    uint16_t completedTransfers() const;
    uint16_t protocolErrorCount() const;
    uint16_t timeoutCount() const;

private:
    static void irqTrampoline();
    bool configurePins();
    void configureLpspi();
    void handleInterrupt();
    void primeTxFifo();
    void prepareNextTransfer();
    void copyPendingToActive();
    void markReady(bool ready);

    static TeensyLinkSpiSlave *activeInstance_;

    uint8_t csPin_ = 10U;
    uint8_t sckPin_ = 13U;
    uint8_t mosiPin_ = 11U;
    uint8_t misoPin_ = 12U;
    uint8_t readyPin_ = 31U;

    bool ready_ = false;
    bool initialized_ = false;

    uint8_t txActiveFrame_[TEENSY_LINK_FRAME_BYTES] = {};
    uint8_t txPendingFrame_[TEENSY_LINK_FRAME_BYTES] = {};
    uint8_t rxActiveFrame_[TEENSY_LINK_FRAME_BYTES] = {};
    uint8_t rxCompleteFrame_[TEENSY_LINK_FRAME_BYTES] = {};
    TeensyLinkS32kSnapshot latestS32k_ = {};

    volatile bool txPendingDirty_ = false;
    volatile bool rxCompleteReady_ = false;
    volatile bool transferActive_ = false;
    volatile uint16_t rxIndex_ = 0U;
    volatile uint16_t txWriteIndex_ = 0U;
    volatile uint16_t completedTransfers_ = 0U;
    volatile uint16_t incompleteTransfers_ = 0U;
    volatile uint16_t rxOverruns_ = 0U;
    volatile uint16_t txUnderruns_ = 0U;
    uint16_t protocolErrors_ = 0U;
};
