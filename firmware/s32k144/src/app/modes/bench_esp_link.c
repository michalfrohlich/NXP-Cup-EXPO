#include "../app_internal.h"

#define ESP_LINK_BENCH_ERROR_HOLD_MS (300U)
#define ESP_LINK_BENCH_TUNE_FLASH_MS (300U)
#define ESP_LINK_BENCH_DISPLAY_MS (500U)

typedef struct
{
    uint32 nextDisplayMs;
    uint32 lastErrorMs;
    uint32 lastTuneMs;
    uint16 lastRxErrors;
    uint16 lastRxHardwareErrors;
    uint16 lastTxErrors;
    EspUartLink_TuneFrame_t lastTune;
    boolean tuneReceived;
} EspLinkBenchState_t;

static EspLinkBenchState_t g_espLinkBench;

static void esp_link_bench_snapshot_runtime_tune(EspUartLink_TuneFrame_t *outTune)
{
    if (outTune == NULL_PTR)
    {
        return;
    }

    outTune->sequence = 0U;
    outTune->proportionalMilli = (uint32)((g_runtimeTune.profile.kp * 1000.0f) + 0.5f);
    outTune->integralMilli = (uint32)((g_runtimeTune.profile.ki * 1000.0f) + 0.5f);
    outTune->derivativeMilli = (uint32)((g_runtimeTune.profile.kd * 1000.0f) + 0.5f);
    outTune->steerClamp = (uint8)g_runtimeTune.profile.steerClamp;
    outTune->steerLpfMilli = (uint16)((g_runtimeTune.profile.steerLpfAlpha * 1000.0f) + 0.5f);
    outTune->minContrast = g_runtimeTune.lineDetector.minContrast;
    outTune->edgeHighPct = g_runtimeTune.lineDetector.edgeHighPct;
    outTune->edgeLowPct = g_runtimeTune.lineDetector.edgeLowPct;
}

static uint32 esp_link_bench_display_milli(uint32 milli)
{
    if (milli > 99999U)
    {
        return 99999U;
    }

    return milli;
}

static void esp_link_bench_draw(const EspUartLink_Diagnostics_t *diag)
{
    char lineBuf[17];
    uint16 errorCount;
    uint32 kpMilli;
    uint32 kdMilli;

    if (diag == NULL_PTR)
    {
        return;
    }

    errorCount = (uint16)(diag->rxProtocolErrors + diag->rxHardwareErrors + diag->txErrors);

    DisplayClear();
    DisplayTextPadded(0U, "ESP TUNE SAFE");

    (void)snprintf(lineBuf, sizeof(lineBuf), "RX:%03u ERR:%03u",
                   (unsigned int)(diag->rxTuneFrames % 1000U), (unsigned int)(errorCount % 1000U));
    DisplayTextPadded(1U, lineBuf);

    kpMilli = esp_link_bench_display_milli(g_espLinkBench.lastTune.proportionalMilli);
    kdMilli = esp_link_bench_display_milli(g_espLinkBench.lastTune.derivativeMilli);
    (void)snprintf(lineBuf, sizeof(lineBuf), "K%lu.%02lu D%lu.%02lu",
                   (unsigned long)(kpMilli / 1000U), (unsigned long)((kpMilli % 1000U) / 10U),
                   (unsigned long)(kdMilli / 1000U), (unsigned long)((kdMilli % 1000U) / 10U));
    DisplayTextPadded(2U, lineBuf);

    (void)snprintf(lineBuf, sizeof(lineBuf), "M%u H%u L%u",
                   (unsigned int)g_espLinkBench.lastTune.minContrast,
                   (unsigned int)g_espLinkBench.lastTune.edgeHighPct,
                   (unsigned int)g_espLinkBench.lastTune.edgeLowPct);
    DisplayTextPadded(3U, lineBuf);

    DisplayRefresh();
}

void esp_link_bench_test_enter(uint32 nowMs)
{
    (void)memset(&g_espLinkBench, 0, sizeof(g_espLinkBench));

    EspUartLink_Init();
    esp_link_bench_snapshot_runtime_tune(&g_espLinkBench.lastTune);
    g_espLinkBench.nextDisplayMs = nowMs;
    g_espLinkBench.lastErrorMs = nowMs - ESP_LINK_BENCH_ERROR_HOLD_MS;
    StatusLed_Blue();
}

void esp_link_bench_test_update(uint32 nowMs)
{
    EspUartLink_Diagnostics_t diag;
    EspUartLink_TuneFrame_t tune;

    if (esp_link_service_tune_frames(nowMs, &tune) == TRUE)
    {
        g_espLinkBench.lastTune = tune;
        g_espLinkBench.lastTuneMs = nowMs;
        g_espLinkBench.tuneReceived = TRUE;
        g_espLinkBench.nextDisplayMs = nowMs;
    }

    EspUartLink_GetDiagnostics(&diag);
    if ((diag.rxProtocolErrors != g_espLinkBench.lastRxErrors) ||
        (diag.rxHardwareErrors != g_espLinkBench.lastRxHardwareErrors) ||
        (diag.txErrors != g_espLinkBench.lastTxErrors))
    {
        g_espLinkBench.lastRxErrors = diag.rxProtocolErrors;
        g_espLinkBench.lastRxHardwareErrors = diag.rxHardwareErrors;
        g_espLinkBench.lastTxErrors = diag.txErrors;
        g_espLinkBench.lastErrorMs = nowMs;
    }

    if ((uint32)(nowMs - g_espLinkBench.lastErrorMs) < ESP_LINK_BENCH_ERROR_HOLD_MS)
    {
        StatusLed_Red();
    }
    else if ((g_espLinkBench.tuneReceived == TRUE) &&
             ((uint32)(nowMs - g_espLinkBench.lastTuneMs) < ESP_LINK_BENCH_TUNE_FLASH_MS))
    {
        StatusLed_Cyan();
    }
    else
    {
        StatusLed_Blue();
    }

    if ((time_reached(nowMs, g_espLinkBench.nextDisplayMs) == TRUE) && (diag.txBusy != TRUE))
    {
        esp_link_bench_draw(&diag);
        g_espLinkBench.nextDisplayMs = nowMs + ESP_LINK_BENCH_DISPLAY_MS;
    }
}

void esp_link_bench_test_exit(void)
{
    StatusLed_Off();
}
