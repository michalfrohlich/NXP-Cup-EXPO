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

typedef struct
{
    uint32 nextButtonsMs;
    uint32 nextQueueMs;
    uint32 nextDiagMs;
#if (ESP_LINK_DISPLAY_ENABLED == 1U)
    uint32 nextDisplayMs;
#endif
    uint32 lastErrorMs;
    uint32 lastTuneMs;
    uint16 lastRxErrors;
    uint16 lastRxHardwareErrors;
    uint16 lastTxErrors;
    uint8 nextSequence;
    EspUartLink_ButtonFrame_t currentButtons;
    EspUartLink_ButtonFrame_t lastQueued;
#if (ESP_LINK_DISPLAY_ENABLED == 1U)
    EspUartLink_TuneFrame_t lastTune;
    boolean showVisionPage;
#endif
    boolean tuneReceived;
} EspLinkTestState_t;

static EspLinkTestState_t g_espLinkTest;

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

boolean esp_link_service_drive_commands(uint32 nowMs,
                                        EspUartLink_DriveCommandFrame_t *outLastCommand)
{
    EspUartLink_DriveCommandFrame_t command;
    boolean received = FALSE;

    EspUartLink_Service(nowMs);
    while (EspUartLink_TakeDriveCommand(&command) == TRUE)
    {
        (void)EspUartLink_QueueTuneResult(command.sequence, TRUE);
        if (outLastCommand != NULL_PTR)
        {
            *outLastCommand = command;
        }
        received = TRUE;
    }

    return received;
}

void esp_link_test_enter(uint32 nowMs)
{
    (void)memset(&g_espLinkTest, 0, sizeof(g_espLinkTest));

    EspUartLink_Init();
    StatusLed_Blue();

#if (ESP_LINK_DISPLAY_ENABLED == 1U)
    g_espLinkTest.lastTune.sequence = 0U;
    g_espLinkTest.lastTune.proportionalMilli =
        (uint32)((g_runtimeTune.profile.kp * 1000.0f) + 0.5f);
    g_espLinkTest.lastTune.integralMilli = (uint32)((g_runtimeTune.profile.ki * 1000.0f) + 0.5f);
    g_espLinkTest.lastTune.derivativeMilli = (uint32)((g_runtimeTune.profile.kd * 1000.0f) + 0.5f);
    g_espLinkTest.lastTune.steerClamp = (uint8)g_runtimeTune.profile.steerClamp;
    g_espLinkTest.lastTune.steerLpfMilli =
        (uint16)((g_runtimeTune.profile.steerLpfAlpha * 1000.0f) + 0.5f);
    g_espLinkTest.lastTune.minContrast = g_runtimeTune.lineDetector.minContrast;
    g_espLinkTest.lastTune.edgeHighPct = g_runtimeTune.lineDetector.edgeHighPct;
    g_espLinkTest.lastTune.edgeLowPct = g_runtimeTune.lineDetector.edgeLowPct;
#endif

    g_espLinkTest.nextButtonsMs = nowMs;
    g_espLinkTest.nextQueueMs = nowMs;
    g_espLinkTest.nextDiagMs = nowMs;
#if (ESP_LINK_DISPLAY_ENABLED == 1U)
    g_espLinkTest.nextDisplayMs = nowMs;
#endif
    g_espLinkTest.lastErrorMs = nowMs - ESP_LINK_ERROR_HOLD_MS;
}

void esp_link_test_update(uint32 nowMs)
{
    EspUartLink_Diagnostics_t diag;
    EspUartLink_TuneFrame_t tune;
    EspUartLink_DriveCommandFrame_t command;
    char diagLine[112];
    boolean stateChanged;
    boolean rxLive;
#if (ESP_LINK_DISPLAY_ENABLED == 1U)
    boolean sw2Pressed;
#endif

    while (time_reached(nowMs, g_espLinkTest.nextButtonsMs) == TRUE)
    {
        Buttons_Update();
        g_espLinkTest.nextButtonsMs += BUTTONS_PERIOD_MS;
    }

#if (ESP_LINK_DISPLAY_ENABLED == 1U)
    sw2Pressed = Buttons_WasPressed(BUTTON_ID_SW2);

    if (sw2Pressed == TRUE)
    {
        g_espLinkTest.showVisionPage =
            (boolean)((g_espLinkTest.showVisionPage == TRUE) ? FALSE : TRUE);
        g_espLinkTest.nextDisplayMs = nowMs;
    }
#endif

    g_espLinkTest.currentButtons.sw2Pressed = Buttons_IsPressed(BUTTON_ID_SW2);
    g_espLinkTest.currentButtons.sw3Pressed = Buttons_IsPressed(BUTTON_ID_SW3);
    g_espLinkTest.currentButtons.swPcbOn = Buttons_IsOn(BUTTON_ID_SWPCB);

    stateChanged =
        (boolean)(((g_espLinkTest.currentButtons.sw2Pressed !=
                    g_espLinkTest.lastQueued.sw2Pressed) ||
                   (g_espLinkTest.currentButtons.sw3Pressed !=
                    g_espLinkTest.lastQueued.sw3Pressed) ||
                   (g_espLinkTest.currentButtons.swPcbOn != g_espLinkTest.lastQueued.swPcbOn))
                      ? TRUE
                      : FALSE);

    if ((stateChanged == TRUE) || (time_reached(nowMs, g_espLinkTest.nextQueueMs) == TRUE))
    {
        g_espLinkTest.currentButtons.sequence = g_espLinkTest.nextSequence;
        g_espLinkTest.currentButtons.timestampMs = 0U;
        if (EspUartLink_QueueButtons(&g_espLinkTest.currentButtons) == E_OK)
        {
            g_espLinkTest.lastQueued = g_espLinkTest.currentButtons;
            g_espLinkTest.nextSequence++;
            if (g_espLinkTest.nextSequence > 99U)
            {
                g_espLinkTest.nextSequence = 0U;
            }
        }

        g_espLinkTest.nextQueueMs = nowMs + ESP_UART_LINK_TX_PERIOD_MS;
    }

    if (esp_link_service_tune_frames(nowMs, &tune) == TRUE)
    {
#if (ESP_LINK_DISPLAY_ENABLED == 1U)
        g_espLinkTest.lastTune = tune;
#endif
        g_espLinkTest.lastTuneMs = nowMs;
        g_espLinkTest.tuneReceived = TRUE;
#if (ESP_LINK_DISPLAY_ENABLED == 1U)
        g_espLinkTest.nextDisplayMs = nowMs;
#endif
    }

    if (esp_link_service_drive_commands(nowMs, &command) == TRUE)
    {
        (void)command;
        g_espLinkTest.lastTuneMs = nowMs;
        g_espLinkTest.tuneReceived = TRUE;
#if (ESP_LINK_DISPLAY_ENABLED == 1U)
        g_espLinkTest.nextDisplayMs = nowMs;
#endif
    }

    EspUartLink_GetDiagnostics(&diag);

    if ((diag.rxProtocolErrors != g_espLinkTest.lastRxErrors) ||
        (diag.rxHardwareErrors != g_espLinkTest.lastRxHardwareErrors) ||
        (diag.txErrors != g_espLinkTest.lastTxErrors))
    {
        g_espLinkTest.lastRxErrors = diag.rxProtocolErrors;
        g_espLinkTest.lastRxHardwareErrors = diag.rxHardwareErrors;
        g_espLinkTest.lastTxErrors = diag.txErrors;
        g_espLinkTest.lastErrorMs = nowMs;
    }

    rxLive = (boolean)(((diag.ackValid == TRUE) &&
                        ((uint32)(nowMs - diag.lastAckMs) < ESP_LINK_RX_LIVE_MS))
                           ? TRUE
                           : FALSE);

    if ((time_reached(nowMs, g_espLinkTest.nextDiagMs) == TRUE) && (diag.ackValid == TRUE) &&
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
        g_espLinkTest.nextDiagMs = nowMs + ESP_LINK_DIAG_PERIOD_MS;
    }

#if (ESP_LINK_DISPLAY_ENABLED == 1U)
    if ((time_reached(nowMs, g_espLinkTest.nextDisplayMs) == TRUE) && (diag.txBusy != TRUE) &&
        ((diag.txFrames == 0U) ||
         ((uint32)(nowMs - diag.lastTxMs) >= ESP_LINK_DISPLAY_RX_GUARD_MS)))
    {
        esp_link_draw_tune(&diag, &g_espLinkTest.lastTune, g_espLinkTest.tuneReceived,
                           g_espLinkTest.showVisionPage);
        g_espLinkTest.nextDisplayMs = nowMs + ESP_LINK_DISPLAY_PERIOD_MS;
    }
#endif

    if ((uint32)(nowMs - g_espLinkTest.lastErrorMs) < ESP_LINK_ERROR_HOLD_MS)
    {
        StatusLed_Red();
    }
    else if ((g_espLinkTest.tuneReceived == TRUE) &&
             ((uint32)(nowMs - g_espLinkTest.lastTuneMs) < ESP_LINK_TUNE_FLASH_MS))
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

void esp_link_test_exit(void)
{
    StatusLed_Off();
}

void mode_esp_link_test(void)
{
    App_InitRuntimeCore();
    esp_link_test_enter(Timebase_GetMs());

    while (1)
    {
        esp_link_test_update(Timebase_GetMs());
    }
}
