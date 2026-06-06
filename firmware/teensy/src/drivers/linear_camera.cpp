#include "drivers/linear_camera.h"

#include <imxrt.h>
#include <string.h>

static constexpr uint32_t MIN_PIXEL_CLOCK_HZ = 5000UL;
static constexpr uint32_t MAX_BRINGUP_PIXEL_CLOCK_HZ = 250000UL;
static constexpr uint32_t MIN_FRAME_RATE_HZ = 1UL;
static constexpr uint32_t MAX_FRAME_RATE_HZ = 500UL;
static constexpr uint32_t SI_SETUP_US = 1UL;
static constexpr uint32_t SI_HOLD_US = 1UL;
static constexpr uint32_t ADC_ETC_SAMPLE_TIMEOUT_US = 200UL;
static constexpr uint8_t TEENSY_PIN15_ADC1_CHANNEL = 8U;
static constexpr uint8_t ADC_ETC_TRIGGER_INDEX = 0U;
static constexpr uint32_t ADC_ETC_DONE0_FLAG =
    ADC_ETC_DONE0_1_IRQ_TRIG_DONE0(ADC_ETC_TRIGGER_INDEX);
static constexpr uint32_t ADC_ETC_ERROR_FLAG =
    ADC_ETC_DONE2_ERR_IRQ_TRIG_ERR(ADC_ETC_TRIGGER_INDEX);
static constexpr uint16_t FLEXPWM_TRIGGER_VAL4_ENABLE = (1U << 4);
static constexpr uint16_t FLEXPWM2_SM2_MASK = (1U << 2);
static constexpr uint8_t PWM_DUTY_50_PERCENT_8BIT = 128U;

extern "C" void xbar_connect(unsigned int input, unsigned int output);

bool LinearCamera::begin(const LinearCameraConfig &config)
{
    if (configIsValid(config) != true)
    {
        status_ = LinearCameraStatus::Fault;
        return false;
    }

    stop();
    config_ = config;
    (void)memset(&counters_, 0, sizeof(counters_));
    (void)memset(frameBuffers_, 0, sizeof(frameBuffers_));
    writeBufferIndex_ = 0U;
    readyBufferIndex_ = 0U;
    sequence_ = 0U;
    latestFrameReady_ = false;
    adcEtcReady_ = false;

    configurePins();
    configureAdc();
    adcEtcReady_ = configureAdcEtcTrigger();
    if (adcEtcReady_ != true)
    {
        status_ = LinearCameraStatus::Fault;
        return false;
    }

    initialized_ = true;
    status_ = LinearCameraStatus::Idle;
    phase_ = CapturePhase::Idle;
    return true;
}

bool LinearCamera::start()
{
    if ((initialized_ != true) || (status_ == LinearCameraStatus::Fault))
    {
        return false;
    }

    latestFrameReady_ = false;
    phase_ = CapturePhase::WaitingForFrame;
    status_ = LinearCameraStatus::Running;
    nextFrameUs_ = micros();
    return true;
}

void LinearCamera::stop()
{
    stopPixelClock();
    if (config_.siPin != 0U)
    {
        digitalWriteFast(config_.siPin, LOW);
    }
    phase_ = CapturePhase::Idle;
    status_ = initialized_ ? LinearCameraStatus::Idle : status_;
}

void LinearCamera::service(uint32_t nowUs)
{
    if (status_ != LinearCameraStatus::Running)
    {
        return;
    }

    if ((phase_ == CapturePhase::WaitingForFrame) &&
        ((int32_t)(nowUs - nextFrameUs_) >= 0))
    {
        requestFrame(nowUs);
    }
}

bool LinearCamera::getLatestFrame(LinearCameraFrame &outFrame)
{
    if (latestFrameReady_ != true)
    {
        return false;
    }

    noInterrupts();
    outFrame = frameBuffers_[readyBufferIndex_];
    latestFrameReady_ = false;
    interrupts();
    return true;
}

LinearCameraStatus LinearCamera::status() const
{
    return status_;
}

bool LinearCamera::isRunning() const
{
    return status_ == LinearCameraStatus::Running;
}

bool LinearCamera::isReadoutActive() const
{
    return phase_ == CapturePhase::ReadoutWindow;
}

void LinearCamera::getDebugCounters(LinearCameraDebugCounters &outCounters) const
{
    outCounters = counters_;
}

uint32_t LinearCamera::pixelClockHz() const
{
    return config_.pixelClockHz;
}

uint32_t LinearCamera::frameRateHz() const
{
    return config_.frameRateHz;
}

uint32_t LinearCamera::framePeriodUs() const
{
    if (config_.frameRateHz == 0UL)
    {
        return 0UL;
    }

    return 1000000UL / config_.frameRateHz;
}

uint32_t LinearCamera::pixelClockPeriodUs() const
{
    if (config_.pixelClockHz == 0UL)
    {
        return 0UL;
    }

    return 1000000UL / config_.pixelClockHz;
}

uint32_t LinearCamera::readoutWindowUs() const
{
    if (config_.pixelClockHz == 0UL)
    {
        return 0UL;
    }

    return (TEENSY_LINEAR_CAMERA_READOUT_CLOCKS * 1000000UL) / config_.pixelClockHz;
}

bool LinearCamera::configIsValid(const LinearCameraConfig &config) const
{
    if ((config.pixelClockHz < MIN_PIXEL_CLOCK_HZ) ||
        (config.pixelClockHz > MAX_BRINGUP_PIXEL_CLOCK_HZ) ||
        (config.frameRateHz < MIN_FRAME_RATE_HZ) ||
        (config.frameRateHz > MAX_FRAME_RATE_HZ))
    {
        return false;
    }

    if ((config.siPin == config.clkPin) ||
        (config.siPin == config.analogPin) ||
        (config.clkPin == config.analogPin))
    {
        return false;
    }

    uint32_t framePeriodUs = 1000000UL / config.frameRateHz;
    uint32_t readoutWindowUs =
        (TEENSY_LINEAR_CAMERA_READOUT_CLOCKS * 1000000UL) / config.pixelClockHz;

    return framePeriodUs > readoutWindowUs;
}

void LinearCamera::configurePins()
{
    pinMode(config_.siPin, OUTPUT);
    pinMode(config_.clkPin, OUTPUT);
    pinMode(config_.analogPin, INPUT);
    digitalWriteFast(config_.siPin, LOW);
    digitalWriteFast(config_.clkPin, LOW);
}

void LinearCamera::stopPixelClock()
{
    if (config_.clkPin != 0U)
    {
        IMXRT_FLEXPWM2.MCTRL &= ~FLEXPWM_MCTRL_RUN(FLEXPWM2_SM2_MASK);
        IMXRT_FLEXPWM2.SM[2].TCTRL &= ~FLEXPWM_SMTCTRL_OUT_TRIG_EN(0x3FU);
        IMXRT_FLEXPWM2.MCTRL |= FLEXPWM_MCTRL_LDOK(FLEXPWM2_SM2_MASK);
        analogWrite(config_.clkPin, 0);
        pinMode(config_.clkPin, OUTPUT);
        digitalWriteFast(config_.clkPin, LOW);
    }
}

void LinearCamera::configureAdc()
{
    analogReadResolution(TEENSY_LINEAR_CAMERA_ADC_BITS);
    analogReadAveraging(1U);
    (void)analogRead(config_.analogPin);

    adcChannel_ = 0U;
    if (config_.analogPin == 15U)
    {
        adcChannel_ = TEENSY_PIN15_ADC1_CHANNEL;
    }
}

void LinearCamera::requestFrame(uint32_t nowUs)
{
    counters_.frameRequestCount++;

    phase_ = CapturePhase::ReadoutWindow;
    if (captureFrameHardwareTriggered(frameBuffers_[writeBufferIndex_], nowUs) == true)
    {
        publishCapturedFrame(micros());
    }
    else
    {
        counters_.adcErrorCount++;
        finishReadoutWindow(micros());
    }
}

void LinearCamera::finishReadoutWindow(uint32_t nowUs)
{
    stopPixelClock();
    uint32_t periodUs = framePeriodUs();
    nextFrameUs_ += periodUs;
    if ((periodUs == 0UL) || ((uint32_t)(nowUs - nextFrameUs_) > periodUs))
    {
        nextFrameUs_ = nowUs + periodUs;
    }

    phase_ = CapturePhase::WaitingForFrame;
}

void LinearCamera::publishCapturedFrame(uint32_t nowUs)
{
    LinearCameraFrame &frame = frameBuffers_[writeBufferIndex_];

    frame.sequence = sequence_++;
    frame.timestampUs = nowUs;

    if (latestFrameReady_ == true)
    {
        counters_.droppedFrameCount++;
    }

    readyBufferIndex_ = writeBufferIndex_;
    writeBufferIndex_ = (uint8_t)((writeBufferIndex_ + 1U) % 2U);
    latestFrameReady_ = true;
    counters_.frameReadyCount++;
    finishReadoutWindow(nowUs);
}

bool LinearCamera::configureAdcEtcTrigger()
{
    if ((config_.analogPin != 15U) || (adcChannel_ != TEENSY_PIN15_ADC1_CHANNEL))
    {
        return false;
    }

    CCM_CCGR1 |= CCM_CCGR1_ADC1(CCM_CCGR_ON);
    CCM_CCGR2 |= CCM_CCGR2_XBAR1(CCM_CCGR_ON);

    xbar_connect(XBARA1_IN_FLEXPWM2_PWM3_OUT_TRIG0, XBARA1_OUT_ADC_ETC_TRIG00);

    if ((ADC_ETC_CTRL & (ADC_ETC_CTRL_SOFTRST | ADC_ETC_CTRL_TSC_BYPASS)) != 0U)
    {
        ADC_ETC_CTRL = 0U;
        ADC_ETC_CTRL = 0U;
    }

    clearAdcEtcFlags();
    IMXRT_ADC_ETC.TRIG[ADC_ETC_TRIGGER_INDEX].CTRL =
        ADC_ETC_TRIG_CTRL_TRIG_CHAIN(0U) |
        ADC_ETC_TRIG_CTRL_TRIG_PRIORITY(7U);
    IMXRT_ADC_ETC.TRIG[ADC_ETC_TRIGGER_INDEX].COUNTER = 0U;
    IMXRT_ADC_ETC.TRIG[ADC_ETC_TRIGGER_INDEX].CHAIN_1_0 =
        ADC_ETC_TRIG_CHAIN_HWTS0(1U) |
        ADC_ETC_TRIG_CHAIN_CSEL0(adcChannel_) |
        ADC_ETC_TRIG_CHAIN_IE0(1U);
    IMXRT_ADC_ETC.TRIG[ADC_ETC_TRIGGER_INDEX].CHAIN_3_2 = 0U;
    IMXRT_ADC_ETC.TRIG[ADC_ETC_TRIGGER_INDEX].CHAIN_5_4 = 0U;
    IMXRT_ADC_ETC.TRIG[ADC_ETC_TRIGGER_INDEX].CHAIN_7_6 = 0U;

    ADC_ETC_DMA_CTRL &= ~ADC_ETC_DMA_CTRL_TRIQ_ENABLE(ADC_ETC_TRIGGER_INDEX);
    ADC_ETC_CTRL |= ADC_ETC_CTRL_TSC_BYPASS |
                    ADC_ETC_CTRL_TRIG_ENABLE(1U << ADC_ETC_TRIGGER_INDEX);

    ADC1_CFG = ADC_CFG_ADTRG |
               ADC_CFG_MODE(2U) |
               ADC_CFG_AVGS(0U) |
               ADC_CFG_ADSTS(3U) |
               ADC_CFG_ADHSC |
               ADC_CFG_ADICLK(0U) |
               ADC_CFG_ADIV(2U);
    ADC1_GC &= ~(ADC_GC_AVGE | ADC_GC_DMAEN | ADC_GC_ADCO);
    ADC1_HC0 = ADC_HC_ADCH(16U);

    return true;
}

void LinearCamera::configurePixelClockTrigger()
{
    IMXRT_FLEXPWM2.MCTRL &= ~FLEXPWM_MCTRL_RUN(FLEXPWM2_SM2_MASK);

    analogWriteResolution(8U);
    analogWriteFrequency(config_.clkPin, (float)config_.pixelClockHz);
    analogWrite(config_.clkPin, PWM_DUTY_50_PERCENT_8BIT);

    IMXRT_FLEXPWM2.SM[2].VAL4 = (int16_t)(IMXRT_FLEXPWM2.SM[2].VAL1 / 4);
    IMXRT_FLEXPWM2.SM[2].TCTRL =
        (IMXRT_FLEXPWM2.SM[2].TCTRL & ~FLEXPWM_SMTCTRL_OUT_TRIG_EN(0x3FU)) |
        FLEXPWM_SMTCTRL_TRGFRQ |
        FLEXPWM_SMTCTRL_OUT_TRIG_EN(FLEXPWM_TRIGGER_VAL4_ENABLE);
    IMXRT_FLEXPWM2.SM[2].CNT = IMXRT_FLEXPWM2.SM[2].INIT;
    IMXRT_FLEXPWM2.MCTRL |= FLEXPWM_MCTRL_LDOK(FLEXPWM2_SM2_MASK);
    IMXRT_FLEXPWM2.MCTRL |= FLEXPWM_MCTRL_RUN(FLEXPWM2_SM2_MASK);
}

void LinearCamera::startPixelClock()
{
    configurePixelClockTrigger();
}

void LinearCamera::clearAdcEtcFlags()
{
    ADC_ETC_DONE0_1_IRQ = ADC_ETC_DONE0_FLAG;
    ADC_ETC_DONE2_ERR_IRQ = ADC_ETC_ERROR_FLAG;
}

bool LinearCamera::waitForTriggeredSample(uint16_t &sample, uint32_t timeoutUs)
{
    uint32_t startUs = micros();

    while ((ADC_ETC_DONE0_1_IRQ & ADC_ETC_DONE0_FLAG) == 0U)
    {
        if ((ADC_ETC_DONE2_ERR_IRQ & ADC_ETC_ERROR_FLAG) != 0U)
        {
            ADC_ETC_DONE2_ERR_IRQ = ADC_ETC_ERROR_FLAG;
            return false;
        }

        if ((uint32_t)(micros() - startUs) > timeoutUs)
        {
            return false;
        }
    }

    sample =
        (uint16_t)(IMXRT_ADC_ETC.TRIG[ADC_ETC_TRIGGER_INDEX].RESULT_1_0 & 0x0FFFU);
    ADC_ETC_DONE0_1_IRQ = ADC_ETC_DONE0_FLAG;
    return true;
}

bool LinearCamera::captureFrameHardwareTriggered(LinearCameraFrame &frame, uint32_t nowUs)
{
    uint32_t captureStartUs = 0U;
    uint32_t captureDurationUs = 0U;
    (void)nowUs;

    if (adcEtcReady_ != true)
    {
        return false;
    }

    stopPixelClock();
    clearAdcEtcFlags();
    counters_.readoutWindowCount++;
    counters_.siPulseCount++;
    captureStartUs = micros();

    digitalWriteFast(config_.siPin, HIGH);
    delayMicroseconds(SI_SETUP_US);
    startPixelClock();
    delayMicroseconds(SI_HOLD_US);
    digitalWriteFast(config_.siPin, LOW);

    for (uint16_t clockIndex = 0U; clockIndex < TEENSY_LINEAR_CAMERA_READOUT_CLOCKS; clockIndex++)
    {
        uint16_t sample = 0U;
        if (waitForTriggeredSample(sample, ADC_ETC_SAMPLE_TIMEOUT_US) != true)
        {
            counters_.timingOverrunCount++;
            stopPixelClock();
            digitalWriteFast(config_.siPin, LOW);
            return false;
        }

        if (clockIndex < TEENSY_LINEAR_CAMERA_PIXEL_COUNT)
        {
            frame.values[clockIndex] = sample;
            counters_.adcSampleCount++;
        }
    }

    stopPixelClock();
    digitalWriteFast(config_.siPin, LOW);

    captureDurationUs = micros() - captureStartUs;
    counters_.lastCaptureUs = captureDurationUs;
    if (captureDurationUs > counters_.maxCaptureUs)
    {
        counters_.maxCaptureUs = captureDurationUs;
    }

    return true;
}
