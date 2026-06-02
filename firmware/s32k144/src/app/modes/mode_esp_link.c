#include "../app_internal.h"

#define ESP_LINK_BUTTON_SEND_PERIOD_MS    (50U)
#define ESP_LINK_ACK_LIVE_MS              (250U)
#define ESP_LINK_ERROR_HOLD_MS            (300U)

void mode_esp_link_test(void)
{
    uint32 nowMs;
    uint32 nextButtonsMs;
    uint32 nextSendMs;
    uint32 lastErrorMs = 0U;
    uint16 lastRxErrors = 0U;
    uint8 nextSequence = 0U;
    EspUartLink_ButtonFrame_t currentButtons = { FALSE, FALSE, FALSE, 0U };
    EspUartLink_ButtonFrame_t lastSent = { FALSE, FALSE, FALSE, 0U };

    App_InitRuntimeCore();
    EspUartLink_Init();
    StatusLed_Blue();

    nowMs = Timebase_GetMs();
    nextButtonsMs = nowMs;
    nextSendMs = nowMs;

    while (1)
    {
        EspUartLink_Diagnostics_t diag;
        boolean stateChanged;
        boolean ackLive;

        nowMs = Timebase_GetMs();

        while (time_reached(nowMs, nextButtonsMs) == TRUE)
        {
            Buttons_Update();
            nextButtonsMs += BUTTONS_PERIOD_MS;
        }

        currentButtons.sw2Pressed = Buttons_IsPressed(BUTTON_ID_SW2);
        currentButtons.sw3Pressed = Buttons_IsPressed(BUTTON_ID_SW3);
        currentButtons.swPcbOn = Buttons_IsOn(BUTTON_ID_SWPCB);

        stateChanged = (boolean)(((currentButtons.sw2Pressed != lastSent.sw2Pressed) ||
                                  (currentButtons.sw3Pressed != lastSent.sw3Pressed) ||
                                  (currentButtons.swPcbOn != lastSent.swPcbOn)) ? TRUE : FALSE);

        if ((stateChanged == TRUE) || (time_reached(nowMs, nextSendMs) == TRUE))
        {
            currentButtons.sequence = nextSequence;
            if (EspUartLink_SendButtons(&currentButtons) == E_OK)
            {
                lastSent = currentButtons;
                nextSequence++;
                if (nextSequence > 99U)
                {
                    nextSequence = 0U;
                }
            }

            nextSendMs = nowMs + ESP_LINK_BUTTON_SEND_PERIOD_MS;
        }

        EspUartLink_Poll(nowMs);
        EspUartLink_GetDiagnostics(&diag);

        if (diag.rxProtocolErrors != lastRxErrors)
        {
            lastRxErrors = diag.rxProtocolErrors;
            lastErrorMs = nowMs;
        }

        ackLive = (boolean)(((diag.ackValid == TRUE) &&
                             ((uint32)(nowMs - diag.lastAckMs) < ESP_LINK_ACK_LIVE_MS)) ? TRUE : FALSE);

        if ((uint32)(nowMs - lastErrorMs) < ESP_LINK_ERROR_HOLD_MS)
        {
            StatusLed_Red();
        }
        else if (ackLive == TRUE)
        {
            StatusLed_Green();
        }
        else
        {
            StatusLed_Blue();
        }
    }
}
