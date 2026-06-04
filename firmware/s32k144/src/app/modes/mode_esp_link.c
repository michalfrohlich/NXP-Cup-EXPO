#include "../app_internal.h"

#define ESP_LINK_ERROR_HOLD_MS (300U)
#define ESP_LINK_TX_LIVE_MS (150U)

void mode_esp_link_test(void)
{
    uint32 nowMs;
    uint32 nextButtonsMs;
    uint32 nextQueueMs;
    uint32 lastErrorMs = 0U;
    uint16 lastRxErrors = 0U;
    uint16 lastTxErrors = 0U;
    uint8 nextSequence = 0U;
    EspUartLink_ButtonFrame_t currentButtons = {FALSE, FALSE, FALSE, 0U, 0U};
    EspUartLink_ButtonFrame_t lastQueued = {FALSE, FALSE, FALSE, 0U, 0U};

    App_InitRuntimeCore();
    EspUartLink_Init();
    StatusLed_Blue();

    nowMs = Timebase_GetMs();
    nextButtonsMs = nowMs;
    nextQueueMs = nowMs;

    while (1)
    {
        EspUartLink_Diagnostics_t diag;
        boolean stateChanged;
        boolean txLive;

        nowMs = Timebase_GetMs();

        while (time_reached(nowMs, nextButtonsMs) == TRUE)
        {
            Buttons_Update();
            nextButtonsMs += BUTTONS_PERIOD_MS;
        }

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

        EspUartLink_Service(nowMs);
        EspUartLink_GetDiagnostics(&diag);

        if ((diag.rxProtocolErrors != lastRxErrors) || (diag.txErrors != lastTxErrors))
        {
            lastRxErrors = diag.rxProtocolErrors;
            lastTxErrors = diag.txErrors;
            lastErrorMs = nowMs;
        }

        txLive = (boolean)(((diag.txFrames > 0U) &&
                            ((uint32)(nowMs - diag.lastTxMs) < ESP_LINK_TX_LIVE_MS))
                               ? TRUE
                               : FALSE);

        if ((uint32)(nowMs - lastErrorMs) < ESP_LINK_ERROR_HOLD_MS)
        {
            StatusLed_Red();
        }
        else if (txLive == TRUE)
        {
            StatusLed_Green();
        }
        else
        {
            StatusLed_Blue();
        }
    }
}
