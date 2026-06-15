#include "comms/teensy_link_spi_slave.h"

#include <string.h>

#include "teensy_config.h"

static constexpr uint32_t SPI_SLAVE_TIMEOUT_US = 2000UL;
static constexpr uint32_t SPI_TIMEOUT_POLL_MASK = 0x3FFU;

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

    digitalWriteFast(TEENSY_LINK_SPI_MISO_PIN, LOW);
    markReady(false);
}

void TeensyLinkSpiSlave::publishFrame(const uint8_t frame[TEENSY_LINK_FRAME_BYTES])
{
    if (frame == nullptr)
    {
        return;
    }

    noInterrupts();
    (void)memcpy(txFrame_, frame, TEENSY_LINK_FRAME_BYTES);
    interrupts();
    markReady(true);
}

void TeensyLinkSpiSlave::driveBit(uint8_t txByte, uint8_t bitMask)
{
    digitalWriteFast(TEENSY_LINK_SPI_MISO_PIN,
                     ((txByte & bitMask) != 0U) ? HIGH : LOW);
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
    uint32_t timeoutPoll = 0U;
    TeensyLinkS32kSnapshot decoded;

    if (ready_ != true)
    {
        return false;
    }

    if (digitalReadFast(TEENSY_LINK_SPI_CS_PIN) == HIGH)
    {
        return false;
    }

    markReady(false);
    noInterrupts();
    (void)memcpy(localTx, txFrame_, TEENSY_LINK_FRAME_BYTES);

    /* At 2 MHz each half-cycle is 250 ns. Keep the CPU dedicated to the
       software SPI slave for the bounded 336 us transfer. Camera PWM, ADC,
       and DMA continue in hardware while interrupt delivery is deferred. */
    driveBit(localTx[byteIndex], bitMask);
    startUs = micros();

    while ((digitalReadFast(TEENSY_LINK_SPI_CS_PIN) == LOW) &&
           (byteIndex < TEENSY_LINK_FRAME_BYTES))
    {
        while ((digitalReadFast(TEENSY_LINK_SPI_CS_PIN) == LOW) &&
               (digitalReadFast(TEENSY_LINK_SPI_SCK_PIN) == LOW))
        {
            timeoutPoll++;
            if (((timeoutPoll & SPI_TIMEOUT_POLL_MASK) == 0U) &&
                ((uint32_t)(micros() - startUs) > SPI_SLAVE_TIMEOUT_US))
            {
                timeouts_++;
                digitalWriteFast(TEENSY_LINK_SPI_MISO_PIN, LOW);
                interrupts();
                return false;
            }
        }

        if (digitalReadFast(TEENSY_LINK_SPI_CS_PIN) == HIGH)
        {
            break;
        }

        if (digitalReadFast(TEENSY_LINK_SPI_MOSI_PIN) == HIGH)
        {
            localRx[byteIndex] |= bitMask;
        }

        while ((digitalReadFast(TEENSY_LINK_SPI_CS_PIN) == LOW) &&
               (digitalReadFast(TEENSY_LINK_SPI_SCK_PIN) == HIGH))
        {
            timeoutPoll++;
            if (((timeoutPoll & SPI_TIMEOUT_POLL_MASK) == 0U) &&
                ((uint32_t)(micros() - startUs) > SPI_SLAVE_TIMEOUT_US))
            {
                timeouts_++;
                digitalWriteFast(TEENSY_LINK_SPI_MISO_PIN, LOW);
                interrupts();
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

    digitalWriteFast(TEENSY_LINK_SPI_MISO_PIN, LOW);
    interrupts();
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
