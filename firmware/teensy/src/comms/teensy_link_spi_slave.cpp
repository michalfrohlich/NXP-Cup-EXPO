#include "comms/teensy_link_spi_slave.h"

#include <string.h>

static constexpr uint32_t SPI_SLAVE_TIMEOUT_US = 15000UL;

void TeensyLinkSpiSlave::begin(uint8_t csPin,
                               uint8_t sckPin,
                               uint8_t mosiPin,
                               uint8_t misoPin,
                               uint8_t readyPin)
{
    csPin_ = csPin;
    sckPin_ = sckPin;
    mosiPin_ = mosiPin;
    misoPin_ = misoPin;
    readyPin_ = readyPin;

    pinMode(csPin_, INPUT_PULLUP);
    pinMode(sckPin_, INPUT);
    pinMode(mosiPin_, INPUT);
    pinMode(misoPin_, OUTPUT);
    pinMode(readyPin_, OUTPUT);

    digitalWriteFast(misoPin_, LOW);
    markReady(false);
}

void TeensyLinkSpiSlave::publishFrame(const uint8_t frame[TEENSY_LINK_FRAME_BYTES])
{
    noInterrupts();
    (void)memcpy(txFrame_, frame, TEENSY_LINK_FRAME_BYTES);
    interrupts();
    markReady(true);
}

void TeensyLinkSpiSlave::driveBit(uint8_t txByte, uint8_t bitMask)
{
    digitalWriteFast(misoPin_, ((txByte & bitMask) != 0U) ? HIGH : LOW);
}

void TeensyLinkSpiSlave::markReady(bool ready)
{
    ready_ = ready;
    digitalWriteFast(readyPin_, ready ? HIGH : LOW);
}

bool TeensyLinkSpiSlave::service()
{
    uint8_t localTx[TEENSY_LINK_FRAME_BYTES];
    uint8_t localRx[TEENSY_LINK_FRAME_BYTES] = {};
    uint16_t byteIndex = 0U;
    uint8_t bitMask = 0x80U;
    uint32_t startUs;
    TeensyLinkS32kSnapshot decoded;

    if (ready_ != true)
    {
        return false;
    }

    if (digitalReadFast(csPin_) == HIGH)
    {
        return false;
    }

    noInterrupts();
    (void)memcpy(localTx, txFrame_, TEENSY_LINK_FRAME_BYTES);
    interrupts();

    driveBit(localTx[byteIndex], bitMask);
    startUs = micros();

    while ((digitalReadFast(csPin_) == LOW) && (byteIndex < TEENSY_LINK_FRAME_BYTES))
    {
        while ((digitalReadFast(csPin_) == LOW) && (digitalReadFast(sckPin_) == LOW))
        {
            if ((uint32_t)(micros() - startUs) > SPI_SLAVE_TIMEOUT_US)
            {
                timeouts_++;
                digitalWriteFast(misoPin_, LOW);
                return false;
            }
        }

        if (digitalReadFast(csPin_) == HIGH)
        {
            break;
        }

        if (digitalReadFast(mosiPin_) == HIGH)
        {
            localRx[byteIndex] |= bitMask;
        }

        while ((digitalReadFast(csPin_) == LOW) && (digitalReadFast(sckPin_) == HIGH))
        {
            if ((uint32_t)(micros() - startUs) > SPI_SLAVE_TIMEOUT_US)
            {
                timeouts_++;
                digitalWriteFast(misoPin_, LOW);
                return false;
            }
        }

        if (bitMask > 0x01U)
        {
            bitMask >>= 1U;
        }
        else
        {
            bitMask = 0x80U;
            byteIndex++;
        }

        if (byteIndex < TEENSY_LINK_FRAME_BYTES)
        {
            driveBit(localTx[byteIndex], bitMask);
        }
    }

    digitalWriteFast(misoPin_, LOW);

    if (byteIndex < TEENSY_LINK_FRAME_BYTES)
    {
        return false;
    }

    completedTransfers_++;

    if (TeensyLinkTelemetry_DecodeS32kFrame(localRx, decoded) == true)
    {
        latestS32k_ = decoded;
    }
    else
    {
        protocolErrors_++;
    }

    return true;
}

bool TeensyLinkSpiSlave::getLatestS32kFrame(TeensyLinkS32kSnapshot &outSnapshot) const
{
    outSnapshot = latestS32k_;
    return latestS32k_.valid;
}

uint16_t TeensyLinkSpiSlave::completedTransfers() const
{
    return completedTransfers_;
}

uint16_t TeensyLinkSpiSlave::protocolErrorCount() const
{
    return protocolErrors_;
}

uint16_t TeensyLinkSpiSlave::timeoutCount() const
{
    return timeouts_;
}
