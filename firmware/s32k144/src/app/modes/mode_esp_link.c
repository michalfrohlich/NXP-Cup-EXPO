#include "../app_internal.h"

#define ESP_LINK_ERROR_HOLD_MS (300U)
#define ESP_LINK_RX_LIVE_MS (250U)
#define ESP_LINK_TUNE_FLASH_MS (300U)
#define ESP_LINK_DIAG_PERIOD_MS (1000U)
#define ESP_LINK_DISPLAY_ENABLED (0U)

#if (ESP_LINK_DISPLAY_ENABLED == 1U)
#define ESP_LINK_DISPLAY_PERIOD_MS (100U)
#define ESP_LINK_DISPLAY_RX_GUARD_MS (5U)

static void esp_link_draw_tune(const EspUartLink_Diagnostics_t *diag,
                               const EspUartLink_TuneFrame_t *tune, boolean tuneReceived,
                               boolean showVisionPage)
{
    char lineBuf[17];
    uint32 whole;
    uint32 fraction;

    DisplayClear();

    if (showVisionPage != TRUE)
    {
        if (tuneReceived == TRUE)
        {
            (void)snprintf(lineBuf, sizeof(lineBuf), "TUNE RX:%5u",
                           (unsigned int)diag->rxTuneFrames);
        }
        else
        {
            (void)snprintf(lineBuf, sizeof(lineBuf), "WAIT RX:%5u", (unsigned int)diag->rxBytes);
        }
        DisplayTextPadded(0U, lineBuf);

        whole = tune->proportionalMilli / 1000U;
        fraction = tune->proportionalMilli % 1000U;
        (void)snprintf(lineBuf, sizeof(lineBuf), "KP:%2lu.%03lu", (unsigned long)whole,
                       (unsigned long)fraction);
        DisplayTextPadded(1U, lineBuf);

        whole = tune->integralMilli / 1000U;
        fraction = tune->integralMilli % 1000U;
        (void)snprintf(lineBuf, sizeof(lineBuf), "KI:%2lu.%03lu", (unsigned long)whole,
                       (unsigned long)fraction);
        DisplayTextPadded(2U, lineBuf);

        whole = tune->derivativeMilli / 1000U;
        fraction = tune->derivativeMilli % 1000U;
        (void)snprintf(lineBuf, sizeof(lineBuf), "KD:%2lu.%03lu", (unsigned long)whole,
                       (unsigned long)fraction);
        DisplayTextPadded(3U, lineBuf);
    }
    else
    {
        (void)snprintf(lineBuf, sizeof(lineBuf), "ERR P:%03u H:%03u",
                       (unsigned int)(diag->rxProtocolErrors % 1000U),
                       (unsigned int)(diag->rxHardwareErrors % 1000U));
        DisplayTextPadded(0U, lineBuf);

        whole = tune->steerLpfMilli / 1000U;
        fraction = tune->steerLpfMilli % 1000U;
        (void)snprintf(lineBuf, sizeof(lineBuf), "CL:%3u L:%lu.%03lu",
                       (unsigned int)tune->steerClamp, (unsigned long)whole,
                       (unsigned long)fraction);
        DisplayTextPadded(1U, lineBuf);
        (void)snprintf(lineBuf, sizeof(lineBuf), "MIN:%4u", (unsigned int)tune->minContrast);
        DisplayTextPadded(2U, lineBuf);
        (void)snprintf(lineBuf, sizeof(lineBuf), "HI:%3u LO:%3u", (unsigned int)tune->edgeHighPct,
                       (unsigned int)tune->edgeLowPct);
        DisplayTextPadded(3U, lineBuf);
    }
    DisplayRefresh();
}
#endif

void esp_link_apply_tune_frame(const EspUartLink_TuneFrame_t *tune)
{
    if (tune == NULL_PTR)
    {
        return;
    }

    g_runtimeTune.profile.kp = (float)tune->proportionalMilli / 1000.0f;
    g_runtimeTune.profile.ki = (float)tune->integralMilli / 1000.0f;
    g_runtimeTune.profile.kd = (float)tune->derivativeMilli / 1000.0f;
    g_runtimeTune.profile.steerClamp = (sint16)tune->steerClamp;
    g_runtimeTune.profile.steerLpfAlpha = (float)tune->steerLpfMilli / 1000.0f;
    g_runtimeTune.lineDetector.minContrast = tune->minContrast;
    g_runtimeTune.lineDetector.edgeHighPct = tune->edgeHighPct;
    g_runtimeTune.lineDetector.edgeLowPct = tune->edgeLowPct;
}

boolean esp_link_service_tune_frames(uint32 nowMs, EspUartLink_TuneFrame_t *outLastTune)
{
    EspUartLink_TuneFrame_t tune;
    boolean received = FALSE;

    EspUartLink_Service(nowMs);
    while (EspUartLink_TakeTune(&tune) == TRUE)
    {
        esp_link_apply_tune_frame(&tune);
        (void)EspUartLink_QueueTuneResult(tune.sequence, TRUE);
        if (outLastTune != NULL_PTR)
        {
            *outLastTune = tune;
        }
        received = TRUE;
    }

    return received;
}

void mode_esp_link_test(void)
{
    uint32 nowMs;
    uint32 nextButtonsMs;
    uint32 nextQueueMs;
    uint32 nextDiagMs;
#if (ESP_LINK_DISPLAY_ENABLED == 1U)
    uint32 nextDisplayMs;
#endif
    uint32 lastErrorMs = 0U;
    uint32 lastTuneMs = 0U;
    uint16 lastRxErrors = 0U;
    uint16 lastRxHardwareErrors = 0U;
    uint16 lastTxErrors = 0U;
    uint8 nextSequence = 0U;
    EspUartLink_ButtonFrame_t currentButtons = {FALSE, FALSE, FALSE, 0U, 0U};
    EspUartLink_ButtonFrame_t lastQueued = {FALSE, FALSE, FALSE, 0U, 0U};
#if (ESP_LINK_DISPLAY_ENABLED == 1U)
    EspUartLink_TuneFrame_t lastTune;
#endif
    boolean tuneReceived = FALSE;
#if (ESP_LINK_DISPLAY_ENABLED == 1U)
    boolean showVisionPage = FALSE;
#endif

    App_InitRuntimeCore();
    EspUartLink_Init();
    StatusLed_Blue();

#if (ESP_LINK_DISPLAY_ENABLED == 1U)
    lastTune.sequence = 0U;
    lastTune.proportionalMilli = (uint32)((g_runtimeTune.profile.kp * 1000.0f) + 0.5f);
    lastTune.integralMilli = (uint32)((g_runtimeTune.profile.ki * 1000.0f) + 0.5f);
    lastTune.derivativeMilli = (uint32)((g_runtimeTune.profile.kd * 1000.0f) + 0.5f);
    lastTune.steerClamp = (uint8)g_runtimeTune.profile.steerClamp;
    lastTune.steerLpfMilli = (uint16)((g_runtimeTune.profile.steerLpfAlpha * 1000.0f) + 0.5f);
    lastTune.minContrast = g_runtimeTune.lineDetector.minContrast;
    lastTune.edgeHighPct = g_runtimeTune.lineDetector.edgeHighPct;
    lastTune.edgeLowPct = g_runtimeTune.lineDetector.edgeLowPct;
#endif

    nowMs = Timebase_GetMs();
    nextButtonsMs = nowMs;
    nextQueueMs = nowMs;
    nextDiagMs = nowMs;
#if (ESP_LINK_DISPLAY_ENABLED == 1U)
    nextDisplayMs = nowMs;
#endif
    lastErrorMs = nowMs - ESP_LINK_ERROR_HOLD_MS;

    while (1)
    {
        EspUartLink_Diagnostics_t diag;
        EspUartLink_TuneFrame_t tune;
        char diagLine[112];
        boolean stateChanged;
        boolean rxLive;
#if (ESP_LINK_DISPLAY_ENABLED == 1U)
        boolean sw2Pressed;
#endif

        nowMs = Timebase_GetMs();

        while (time_reached(nowMs, nextButtonsMs) == TRUE)
        {
            Buttons_Update();
            nextButtonsMs += BUTTONS_PERIOD_MS;
        }

#if (ESP_LINK_DISPLAY_ENABLED == 1U)
        sw2Pressed = Buttons_WasPressed(BUTTON_ID_SW2);

        if (sw2Pressed == TRUE)
        {
            showVisionPage = (boolean)((showVisionPage == TRUE) ? FALSE : TRUE);
            nextDisplayMs = nowMs;
        }
#endif

        currentButtons.sw2Pressed = Buttons_IsPressed(BUTTON_ID_SW2);
        currentButtons.sw3Pressed = Buttons_IsPressed(BUTTON_ID_SW3);
        currentButtons.swPcbOn = Buttons_IsOn(BUTTON_ID_SWPCB);

        stateChanged = (boolean)(((currentButtons.sw2Pressed != lastQueued.sw2Pressed) ||
                                  (currentButtons.sw3Pressed != lastQueued.sw3Pressed) ||
                                  (currentButtons.swPcbOn != lastQueued.swPcbOn))
                                     ? TRUE
                                     : FALSE);

        if ((stateChanged == TRUE) || (time_reached(nowMs, nextQueueMs) == TRUE))
        {
            currentButtons.sequence = nextSequence;
            currentButtons.timestampMs = 0U;
            if (EspUartLink_QueueButtons(&currentButtons) == E_OK)
            {
                lastQueued = currentButtons;
                nextSequence++;
                if (nextSequence > 99U)
                {
                    nextSequence = 0U;
                }
            }

            nextQueueMs = nowMs + ESP_UART_LINK_TX_PERIOD_MS;
        }

        if (esp_link_service_tune_frames(nowMs, &tune) == TRUE)
        {
#if (ESP_LINK_DISPLAY_ENABLED == 1U)
            lastTune = tune;
#endif
            lastTuneMs = nowMs;
            tuneReceived = TRUE;
#if (ESP_LINK_DISPLAY_ENABLED == 1U)
            nextDisplayMs = nowMs;
#endif
        }

        EspUartLink_GetDiagnostics(&diag);

        if ((diag.rxProtocolErrors != lastRxErrors) ||
            (diag.rxHardwareErrors != lastRxHardwareErrors) || (diag.txErrors != lastTxErrors))
        {
            lastRxErrors = diag.rxProtocolErrors;
            lastRxHardwareErrors = diag.rxHardwareErrors;
            lastTxErrors = diag.txErrors;
            lastErrorMs = nowMs;
        }

        rxLive = (boolean)(((diag.ackValid == TRUE) &&
                            ((uint32)(nowMs - diag.lastAckMs) < ESP_LINK_RX_LIVE_MS))
                               ? TRUE
                               : FALSE);

        if ((time_reached(nowMs, nextDiagMs) == TRUE) && (diag.ackValid == TRUE) &&
            (diag.lastAckMs == nowMs))
        {
            (void)snprintf(
                diagLine, sizeof(diagLine),
                "ESPDIAG rx=%u ack=%u proto=%u hw=%u ov=%u noise=%u frame=%u parity=%u tx=%u",
                (unsigned int)diag.rxBytes, (unsigned int)diag.rxAckFrames,
                (unsigned int)diag.rxProtocolErrors, (unsigned int)diag.rxHardwareErrors,
                (unsigned int)diag.rxOverrunErrors, (unsigned int)diag.rxNoiseErrors,
                (unsigned int)diag.rxFramingErrors, (unsigned int)diag.rxParityErrors,
                (unsigned int)diag.txErrors);
            UartHost_WriteLine(diagLine);
            nextDiagMs = nowMs + ESP_LINK_DIAG_PERIOD_MS;
        }

#if (ESP_LINK_DISPLAY_ENABLED == 1U)
        if ((time_reached(nowMs, nextDisplayMs) == TRUE) && (diag.txBusy != TRUE) &&
            ((diag.txFrames == 0U) ||
             ((uint32)(nowMs - diag.lastTxMs) >= ESP_LINK_DISPLAY_RX_GUARD_MS)))
        {
            esp_link_draw_tune(&diag, &lastTune, tuneReceived, showVisionPage);
            nextDisplayMs = nowMs + ESP_LINK_DISPLAY_PERIOD_MS;
        }
#endif

        if ((uint32)(nowMs - lastErrorMs) < ESP_LINK_ERROR_HOLD_MS)
        {
            StatusLed_Red();
        }
        else if ((tuneReceived == TRUE) && ((uint32)(nowMs - lastTuneMs) < ESP_LINK_TUNE_FLASH_MS))
        {
            StatusLed_Cyan();
        }
        else if (rxLive == TRUE)
        {
            StatusLed_Green();
        }
        else
        {
            StatusLed_Blue();
        }
    }
}
