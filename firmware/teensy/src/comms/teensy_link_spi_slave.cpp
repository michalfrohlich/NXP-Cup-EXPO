#include "comms/teensy_link_spi_slave.h"

#include <string.h>

static constexpr uint32_t SPI_CLEAR_STATUS_FLAGS =
    LPSPI_SR_DMF | LPSPI_SR_REF | LPSPI_SR_TEF | LPSPI_SR_TCF | LPSPI_SR_FCF | LPSPI_SR_WCF;

TeensyLinkSpiSlave *TeensyLinkSpiSlave::activeInstance_ = nullptr;

void TeensyLinkSpiSlave::irqTrampoline()
{
    if (activeInstance_ != nullptr)
    {
        activeInstance_->handleInterrupt();
    }
}

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

    pinMode(readyPin_, OUTPUT);
    markReady(false);

    activeInstance_ = this;
    if (configurePins() != true)
    {
        return;
    }

    configureLpspi();
    initialized_ = true;
}

bool TeensyLinkSpiSlave::configurePins()
{
    if ((csPin_ != 10U) || (mosiPin_ != 11U) || (misoPin_ != 12U) || (sckPin_ != 13U))
    {
        initialized_ = false;
        return false;
    }

    IOMUXC_LPSPI4_PCS0_SELECT_INPUT = 0U;
    IOMUXC_LPSPI4_SCK_SELECT_INPUT = 0U;
    IOMUXC_LPSPI4_SDI_SELECT_INPUT = 0U;
    IOMUXC_LPSPI4_SDO_SELECT_INPUT = 0U;

    IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_00 = 3U | 0x10U; /* pin 10, PCS0 */
    IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_01 = 3U | 0x10U; /* pin 12, MISO/SDI */
    IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_02 = 3U | 0x10U; /* pin 11, MOSI/SDO */
    IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_03 = 3U | 0x10U; /* pin 13, SCK */
    return true;
}

void TeensyLinkSpiSlave::configureLpspi()
{
    CCM_CCGR1 |= CCM_CCGR1_LPSPI4(CCM_CCGR_ON);

    NVIC_DISABLE_IRQ(IRQ_LPSPI4);
    LPSPI4_IER = 0U;
    LPSPI4_DER = 0U;

    LPSPI4_CR = LPSPI_CR_RST;
    LPSPI4_CR = 0U;
    LPSPI4_FCR = LPSPI_FCR_RXWATER(0U) | LPSPI_FCR_TXWATER(0U);
    LPSPI4_CFGR0 = 0U;
    LPSPI4_CFGR1 = 0U;
    LPSPI4_TCR = LPSPI_TCR_FRAMESZ(7U);
    LPSPI4_SR = SPI_CLEAR_STATUS_FLAGS;
    LPSPI4_CR = LPSPI_CR_MEN | LPSPI_CR_DBGEN | LPSPI_CR_RRF | LPSPI_CR_RTF;
    LPSPI4_CR = LPSPI_CR_MEN | LPSPI_CR_DBGEN;

    rxIndex_ = 0U;
    txWriteIndex_ = 0U;
    (void)memset(txActiveFrame_, 0, sizeof(txActiveFrame_));
    (void)memset(txPendingFrame_, 0, sizeof(txPendingFrame_));
    (void)memset(rxActiveFrame_, 0, sizeof(rxActiveFrame_));
    (void)memset(rxCompleteFrame_, 0, sizeof(rxCompleteFrame_));
    primeTxFifo();

    attachInterruptVector(IRQ_LPSPI4, TeensyLinkSpiSlave::irqTrampoline);
    NVIC_SET_PRIORITY(IRQ_LPSPI4, 64);
    LPSPI4_IER = LPSPI_IER_WCIE | LPSPI_IER_FCIE | LPSPI_IER_TEIE | LPSPI_IER_REIE;
    NVIC_ENABLE_IRQ(IRQ_LPSPI4);
}

void TeensyLinkSpiSlave::publishFrame(const uint8_t frame[TEENSY_LINK_FRAME_BYTES])
{
    if (frame == nullptr)
    {
        return;
    }

    noInterrupts();
    (void)memcpy(txPendingFrame_, frame, TEENSY_LINK_FRAME_BYTES);
    txPendingDirty_ = true;
    if ((initialized_ == true) && ((LPSPI4_SR & LPSPI_SR_MBF) == 0U) && (transferActive_ == false))
    {
        copyPendingToActive();
        prepareNextTransfer();
    }
    interrupts();

    if (initialized_ == true)
    {
        markReady(true);
    }
}

void TeensyLinkSpiSlave::copyPendingToActive()
{
    if (txPendingDirty_ == true)
    {
        (void)memcpy(txActiveFrame_, txPendingFrame_, TEENSY_LINK_FRAME_BYTES);
        txPendingDirty_ = false;
    }
}

void TeensyLinkSpiSlave::prepareNextTransfer()
{
    copyPendingToActive();
    rxIndex_ = 0U;
    txWriteIndex_ = 0U;
    (void)memset(rxActiveFrame_, 0, sizeof(rxActiveFrame_));

    LPSPI4_CR = LPSPI_CR_MEN | LPSPI_CR_DBGEN | LPSPI_CR_RRF | LPSPI_CR_RTF;
    LPSPI4_CR = LPSPI_CR_MEN | LPSPI_CR_DBGEN;
    LPSPI4_SR = SPI_CLEAR_STATUS_FLAGS;
    primeTxFifo();
}

void TeensyLinkSpiSlave::primeTxFifo()
{
    while (((LPSPI4_SR & LPSPI_SR_TDF) != 0U) && (txWriteIndex_ < TEENSY_LINK_FRAME_BYTES))
    {
        LPSPI4_TDR = txActiveFrame_[txWriteIndex_];
        txWriteIndex_++;
    }
}

void TeensyLinkSpiSlave::handleInterrupt()
{
    uint32_t status = LPSPI4_SR;

    if ((status & (LPSPI_SR_TEF | LPSPI_SR_REF)) != 0U)
    {
        txUnderruns_++;
        LPSPI4_SR = status & (LPSPI_SR_TEF | LPSPI_SR_REF);
    }

    while ((LPSPI4_RSR & LPSPI_RSR_RXEMPTY) == 0U)
    {
        uint8_t rxByte = (uint8_t)LPSPI4_RDR;
        transferActive_ = true;

        if (rxIndex_ < TEENSY_LINK_FRAME_BYTES)
        {
            rxActiveFrame_[rxIndex_] = rxByte;
            rxIndex_++;
        }
        else
        {
            rxOverruns_++;
        }
    }

    primeTxFifo();

    status = LPSPI4_SR;
    if ((status & LPSPI_SR_FCF) != 0U)
    {
        transferActive_ = false;

        if (rxIndex_ == TEENSY_LINK_FRAME_BYTES)
        {
            if (rxCompleteReady_ == true)
            {
                rxOverruns_++;
            }
            else
            {
                (void)memcpy(rxCompleteFrame_, rxActiveFrame_, TEENSY_LINK_FRAME_BYTES);
                rxCompleteReady_ = true;
                completedTransfers_++;
            }
        }
        else if (rxIndex_ > 0U)
        {
            incompleteTransfers_++;
        }

        prepareNextTransfer();
    }
    else if ((status & LPSPI_SR_WCF) != 0U)
    {
        LPSPI4_SR = LPSPI_SR_WCF;
    }

    asm volatile("dsb" ::: "memory");
}

void TeensyLinkSpiSlave::markReady(bool ready)
{
    ready_ = ready;
    digitalWriteFast(readyPin_, ready ? HIGH : LOW);
}

bool TeensyLinkSpiSlave::service()
{
    uint8_t localRx[TEENSY_LINK_FRAME_BYTES];
    bool hasFrame = false;
    TeensyLinkS32kSnapshot decoded;

    noInterrupts();
    if (rxCompleteReady_ == true)
    {
        (void)memcpy(localRx, rxCompleteFrame_, TEENSY_LINK_FRAME_BYTES);
        rxCompleteReady_ = false;
        hasFrame = true;
    }
    interrupts();

    if (hasFrame != true)
    {
        return false;
    }

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
    return (uint16_t)(incompleteTransfers_ + rxOverruns_ + txUnderruns_);
}
