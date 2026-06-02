#include "drivers/esp_uart_link.h"

#include <string.h>

#include "CDD_Uart.h"
#include "S32K144.h"
#include "config/board_config.h"
#include "../../../../shared/protocol/esp_s32k_uart_protocol.h"

#define ESP_UART_LINK_WRITE_TIMEOUT_US  (10000U)
#define ESP_UART_LINK_RX_FRAME_MAX      (ESP_S32K_ACK_FRAME_LEN)

static EspUartLink_AckFrame_t g_espUartLastAck;
static EspUartLink_Diagnostics_t g_espUartDiag;
static uint8 g_espUartRxFrame[ESP_UART_LINK_RX_FRAME_MAX];
static uint8 g_espUartRxLen;
static boolean g_espUartRxActive;

static void reset_rx_parser(void)
{
    g_espUartRxLen = 0U;
    g_espUartRxActive = FALSE;
}

static void consume_rx_byte(uint8 byte, uint32 nowMs)
{
    EspS32kAckFrame_t decoded;

    if (byte == (uint8)ESP_S32K_PID_FRAME_START)
    {
        g_espUartRxFrame[0] = byte;
        g_espUartRxLen = 1U;
        g_espUartRxActive = TRUE;
        return;
    }

    if (g_espUartRxActive != TRUE)
    {
        return;
    }

    if (g_espUartRxLen >= ESP_UART_LINK_RX_FRAME_MAX)
    {
        g_espUartDiag.rxProtocolErrors++;
        reset_rx_parser();
        return;
    }

    g_espUartRxFrame[g_espUartRxLen] = byte;
    g_espUartRxLen++;

    if (byte != (uint8)ESP_S32K_PID_FRAME_END)
    {
        return;
    }

    if ((g_espUartRxLen == ESP_S32K_ACK_FRAME_LEN) &&
        (EspS32kAckFrame_Decode(g_espUartRxFrame,
                                (size_t)g_espUartRxLen,
                                &decoded) == true))
    {
        g_espUartLastAck.sw2Pressed = decoded.sw2Pressed ? TRUE : FALSE;
        g_espUartLastAck.sw3Pressed = decoded.sw3Pressed ? TRUE : FALSE;
        g_espUartLastAck.swPcbOn = decoded.swPcbOn ? TRUE : FALSE;
        g_espUartLastAck.sequence = decoded.sequence;
        g_espUartDiag.rxAckFrames++;
        g_espUartDiag.lastAckMs = nowMs;
        g_espUartDiag.ackValid = TRUE;
    }
    else
    {
        g_espUartDiag.rxProtocolErrors++;
        g_espUartDiag.ackValid = FALSE;
    }

    reset_rx_parser();
}

void EspUartLink_Init(void)
{
    (void)memset(&g_espUartLastAck, 0, sizeof(g_espUartLastAck));
    (void)memset(&g_espUartDiag, 0, sizeof(g_espUartDiag));
    (void)memset(g_espUartRxFrame, 0, sizeof(g_espUartRxFrame));
    reset_rx_parser();

    IP_LPUART2->CTRL |= (LPUART_CTRL_RE_MASK | LPUART_CTRL_TE_MASK);
    while ((IP_LPUART2->STAT & LPUART_STAT_RDRF_MASK) != 0U)
    {
        (void)IP_LPUART2->DATA;
    }

    g_espUartDiag.initialized = TRUE;
}

Std_ReturnType EspUartLink_SendButtons(const EspUartLink_ButtonFrame_t *buttons)
{
    uint8 frame[ESP_S32K_BUTTON_FRAME_LEN];
    EspS32kButtonFrame_t encoded;

    if ((buttons == NULL_PTR) || (g_espUartDiag.initialized != TRUE))
    {
        g_espUartDiag.txErrors++;
        return E_NOT_OK;
    }

    encoded.sw2Pressed = (buttons->sw2Pressed == TRUE) ? true : false;
    encoded.sw3Pressed = (buttons->sw3Pressed == TRUE) ? true : false;
    encoded.swPcbOn = (buttons->swPcbOn == TRUE) ? true : false;
    encoded.sequence = buttons->sequence;

    if (EspS32kButtonFrame_Encode(&encoded, frame, sizeof(frame)) != true)
    {
        g_espUartDiag.txErrors++;
        return E_NOT_OK;
    }

    if (Uart_SyncSend((uint8)ESP_UART_LINK_UART_CHANNEL,
                      frame,
                      (uint16)sizeof(frame),
                      ESP_UART_LINK_WRITE_TIMEOUT_US) != E_OK)
    {
        g_espUartDiag.txErrors++;
        return E_NOT_OK;
    }

    g_espUartDiag.txFrames++;
    return E_OK;
}

void EspUartLink_Poll(uint32 nowMs)
{
    if (g_espUartDiag.initialized != TRUE)
    {
        return;
    }

    while ((IP_LPUART2->STAT & LPUART_STAT_RDRF_MASK) != 0U)
    {
        consume_rx_byte((uint8)IP_LPUART2->DATA, nowMs);
    }
}

boolean EspUartLink_GetLastAck(EspUartLink_AckFrame_t *outAck)
{
    if ((outAck == NULL_PTR) || (g_espUartDiag.ackValid != TRUE))
    {
        return FALSE;
    }

    *outAck = g_espUartLastAck;
    return TRUE;
}

void EspUartLink_GetDiagnostics(EspUartLink_Diagnostics_t *outDiagnostics)
{
    if (outDiagnostics == NULL_PTR)
    {
        return;
    }

    *outDiagnostics = g_espUartDiag;
}
