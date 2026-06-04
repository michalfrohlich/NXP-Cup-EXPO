#include "drivers/esp_uart_link.h"

#include <string.h>

#include "CDD_Uart.h"
#include "S32K144.h"
#include "config/board_config.h"
#include "../../../../shared/protocol/esp_s32k_uart_protocol.h"

#define ESP_UART_LINK_WRITE_TIMEOUT_US (2000U)
#define ESP_UART_LINK_RX_BUDGET_BYTES (16U)
#define ESP_UART_LINK_TX_BUDGET_BYTES (4U)
#define ESP_UART_LINK_RX_FRAME_MAX (ESP_S32K_ACK_FRAME_LEN)

static EspUartLink_AckFrame_t g_espUartLastAck;
static EspUartLink_ButtonFrame_t g_espUartPendingTx;
static EspUartLink_Diagnostics_t g_espUartDiag;
static uint8 g_espUartRxFrame[ESP_UART_LINK_RX_FRAME_MAX];
static uint8 g_espUartTxFrame[ESP_S32K_BUTTON_FRAME_LEN];
static uint8 g_espUartRxLen;
static uint8 g_espUartTxIndex;
static boolean g_espUartRxActive;
static boolean g_espUartTxPending;
static boolean g_espUartTxBusy;
static uint32 g_espUartNextTxMs;

static boolean time_due(uint32 nowMs, uint32 dueMs)
{
    return (boolean)(((sint32)(nowMs - dueMs) >= 0) ? TRUE : FALSE);
}

static uint16 elapsed_modulo_ms(uint16 nowMs, uint16 thenMs)
{
    return (nowMs >= thenMs) ? (uint16)(nowMs - thenMs)
                             : (uint16)((ESP_S32K_TIME_MODULO_MS - thenMs) + nowMs);
}

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
        (EspS32kAckFrame_Decode(g_espUartRxFrame, (size_t)g_espUartRxLen, &decoded) == true))
    {
        g_espUartLastAck.sw2Pressed = decoded.sw2Pressed ? TRUE : FALSE;
        g_espUartLastAck.sw3Pressed = decoded.sw3Pressed ? TRUE : FALSE;
        g_espUartLastAck.swPcbOn = decoded.swPcbOn ? TRUE : FALSE;
        g_espUartLastAck.sequence = decoded.sequence;
        g_espUartLastAck.timestampMs = decoded.timestampMs;
        g_espUartDiag.rxAckFrames++;
        g_espUartDiag.lastAckMs = nowMs;
        g_espUartDiag.lastAckAgeMs = elapsed_modulo_ms((uint16)(nowMs % ESP_S32K_TIME_MODULO_MS),
                                                       g_espUartLastAck.timestampMs);
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
    (void)memset(&g_espUartPendingTx, 0, sizeof(g_espUartPendingTx));
    (void)memset(&g_espUartDiag, 0, sizeof(g_espUartDiag));
    (void)memset(g_espUartRxFrame, 0, sizeof(g_espUartRxFrame));
    (void)memset(g_espUartTxFrame, 0, sizeof(g_espUartTxFrame));
    reset_rx_parser();
    g_espUartTxIndex = 0U;
    g_espUartTxPending = FALSE;
    g_espUartTxBusy = FALSE;
    g_espUartNextTxMs = 0U;

    IP_LPUART2->CTRL |= (LPUART_CTRL_RE_MASK | LPUART_CTRL_TE_MASK);
    while ((IP_LPUART2->STAT & LPUART_STAT_RDRF_MASK) != 0U)
    {
        (void)IP_LPUART2->DATA;
    }

    g_espUartDiag.initialized = TRUE;
}

static Std_ReturnType send_buttons_now(const EspUartLink_ButtonFrame_t *buttons)
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
    encoded.timestampMs = buttons->timestampMs;

    if (EspS32kButtonFrame_Encode(&encoded, frame, sizeof(frame)) != true)
    {
        g_espUartDiag.txErrors++;
        return E_NOT_OK;
    }

    if (Uart_SyncSend((uint8)ESP_UART_LINK_UART_CHANNEL, frame, (uint16)sizeof(frame),
                      ESP_UART_LINK_WRITE_TIMEOUT_US) != E_OK)
    {
        g_espUartDiag.txErrors++;
        return E_NOT_OK;
    }

    g_espUartDiag.txFrames++;
    return E_OK;
}

static void poll_rx_budget(uint32 nowMs, uint8 maxBytes)
{
    uint8 processed = 0U;

    if (g_espUartDiag.initialized != TRUE)
    {
        return;
    }

    while (((IP_LPUART2->STAT & LPUART_STAT_RDRF_MASK) != 0U) && (processed < maxBytes))
    {
        consume_rx_byte((uint8)IP_LPUART2->DATA, nowMs);
        processed++;
    }
}

static Std_ReturnType encode_buttons_frame(const EspUartLink_ButtonFrame_t *buttons, uint8 *frame,
                                           uint16 frameLen)
{
    EspS32kButtonFrame_t encoded;

    if ((buttons == NULL_PTR) || (frame == NULL_PTR) || (frameLen < ESP_S32K_BUTTON_FRAME_LEN))
    {
        return E_NOT_OK;
    }

    encoded.sw2Pressed = (buttons->sw2Pressed == TRUE) ? true : false;
    encoded.sw3Pressed = (buttons->sw3Pressed == TRUE) ? true : false;
    encoded.swPcbOn = (buttons->swPcbOn == TRUE) ? true : false;
    encoded.sequence = buttons->sequence;
    encoded.timestampMs = buttons->timestampMs;

    return (EspS32kButtonFrame_Encode(&encoded, frame, (size_t)frameLen) == true) ? E_OK : E_NOT_OK;
}

static void start_pending_tx(uint32 nowMs)
{
    EspUartLink_ButtonFrame_t frameToSend;

    frameToSend = g_espUartPendingTx;
    frameToSend.timestampMs = (uint16)(nowMs % ESP_S32K_TIME_MODULO_MS);

    if (encode_buttons_frame(&frameToSend, g_espUartTxFrame, (uint16)sizeof(g_espUartTxFrame)) !=
        E_OK)
    {
        g_espUartDiag.txErrors++;
        g_espUartTxPending = FALSE;
        g_espUartDiag.txPending = FALSE;
        g_espUartNextTxMs = nowMs + ESP_UART_LINK_TX_PERIOD_MS;
        return;
    }

    g_espUartTxIndex = 0U;
    g_espUartTxBusy = TRUE;
    g_espUartTxPending = FALSE;
    g_espUartDiag.txBusy = TRUE;
    g_espUartDiag.txPending = FALSE;
    g_espUartDiag.lastTxMs = nowMs;
    g_espUartNextTxMs = nowMs + ESP_UART_LINK_TX_PERIOD_MS;
}

static void pump_tx_budget(uint8 maxBytes)
{
    uint8 written = 0U;

    while ((g_espUartTxBusy == TRUE) && (written < maxBytes) &&
           ((IP_LPUART2->STAT & LPUART_STAT_TDRE_MASK) != 0U))
    {
        IP_LPUART2->DATA = g_espUartTxFrame[g_espUartTxIndex];
        g_espUartTxIndex++;
        written++;

        if (g_espUartTxIndex >= (uint8)sizeof(g_espUartTxFrame))
        {
            g_espUartTxBusy = FALSE;
            g_espUartDiag.txBusy = FALSE;
            g_espUartDiag.txFrames++;
        }
    }
}

Std_ReturnType EspUartLink_QueueButtons(const EspUartLink_ButtonFrame_t *buttons)
{
    if ((buttons == NULL_PTR) || (g_espUartDiag.initialized != TRUE))
    {
        g_espUartDiag.txErrors++;
        return E_NOT_OK;
    }

    if (g_espUartTxPending == TRUE)
    {
        g_espUartDiag.txOverwrites++;
    }

    g_espUartPendingTx = *buttons;
    g_espUartTxPending = TRUE;
    g_espUartDiag.txPending = TRUE;
    g_espUartDiag.txQueuedFrames++;
    return E_OK;
}

void EspUartLink_Service(uint32 nowMs)
{
    if (g_espUartDiag.initialized != TRUE)
    {
        return;
    }

    poll_rx_budget(nowMs, ESP_UART_LINK_RX_BUDGET_BYTES);
    pump_tx_budget(ESP_UART_LINK_TX_BUDGET_BYTES);

    if ((g_espUartTxPending != TRUE) || (g_espUartTxBusy == TRUE) ||
        (time_due(nowMs, g_espUartNextTxMs) != TRUE))
    {
        return;
    }

    start_pending_tx(nowMs);
    pump_tx_budget(ESP_UART_LINK_TX_BUDGET_BYTES);
}

Std_ReturnType EspUartLink_SendButtons(const EspUartLink_ButtonFrame_t *buttons)
{
    return send_buttons_now(buttons);
}

void EspUartLink_Poll(uint32 nowMs)
{
    poll_rx_budget(nowMs, 255U);
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
