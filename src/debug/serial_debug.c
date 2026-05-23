#include "debug/serial_debug.h"

/* Temporary S32K UART transport.
 * This module still reaches below the generated UART API for RX polling and
 * should be treated as board-specific bring-up/debug code, not portable logic.
 */

#include "CDD_Uart.h"
#include "S32K144.h"

#define SERIAL_DEBUG_UART_CHANNEL 0U
#define SERIAL_DEBUG_WRITE_TIMEOUT_US 10000U

static boolean g_serialDebugInitialized = FALSE;
static uint8 g_serialDebugTxBuffer[SERIAL_DEBUG_TX_BUFFER_SIZE];
static volatile uint16 g_serialDebugTxHead = 0U;
static volatile uint16 g_serialDebugTxTail = 0U;

void SerialDebug_Init(void)
{
    Uart_Init(NULL_PTR);

    /* The generated UART config is interrupt-mode only. Keep RX enabled here so
       the existing tuning shell can still poll single characters without an ISR. */
    /* TODO(serial): Replace this proof-of-concept RX polling with generated
       UART async receive plus an LPUART1 IRQ/callback ring buffer. */
    IP_LPUART1->CTRL |= LPUART_CTRL_RE_MASK;

    g_serialDebugTxHead = 0U;
    g_serialDebugTxTail = 0U;

    g_serialDebugInitialized = TRUE;
}

boolean SerialDebug_IsRxReady(void)
{
    if (g_serialDebugInitialized != TRUE)
    {
        return FALSE;
    }

    return ((IP_LPUART1->STAT & LPUART_STAT_RDRF_MASK) != 0U) ? TRUE : FALSE;
}

boolean SerialDebug_TryReadChar(char *ch)
{
    if ((ch == (char *)0) || (SerialDebug_IsRxReady() != TRUE))
    {
        return FALSE;
    }

    *ch = (char)IP_LPUART1->DATA;
    return TRUE;
}

char SerialDebug_ReadCharBlocking(void)
{
    while (SerialDebug_IsRxReady() != TRUE)
    {
    }

    return (char)IP_LPUART1->DATA;
}

boolean SerialDebug_IsTxReady(void)
{
    return (g_serialDebugInitialized == TRUE) ? TRUE : FALSE;
}

boolean SerialDebug_TryWriteChar(char ch)
{
    uint8 data = (uint8)ch;

    if (g_serialDebugInitialized != TRUE)
    {
        return FALSE;
    }

    return (Uart_SyncSend(SERIAL_DEBUG_UART_CHANNEL,
                          &data,
                          1U,
                          SERIAL_DEBUG_WRITE_TIMEOUT_US) == E_OK) ? TRUE : FALSE;
}

void SerialDebug_WriteChar(char ch)
{
    if (g_serialDebugInitialized != TRUE)
    {
        return;
    }

    while (SerialDebug_TryWriteChar(ch) != TRUE)
    {
    }
}

void SerialDebug_WriteString(const char *text)
{
    if (text == (const char *)0)
    {
        return;
    }

    while (*text != '\0')
    {
        SerialDebug_WriteChar(*text);
        text++;
    }
}

void SerialDebug_WriteLine(const char *text)
{
    SerialDebug_WriteString(text);
    SerialDebug_WriteString("\r\n");
}

void SerialDebug_ServiceTx(void)
{
    if (g_serialDebugInitialized != TRUE)
    {
        return;
    }

    while ((g_serialDebugTxHead != g_serialDebugTxTail) &&
           (SerialDebug_IsTxReady() == TRUE))
    {
        if (SerialDebug_TryWriteChar((char)g_serialDebugTxBuffer[g_serialDebugTxTail]) != TRUE)
        {
            break;
        }
        g_serialDebugTxTail = (uint16)((g_serialDebugTxTail + 1U) % SERIAL_DEBUG_TX_BUFFER_SIZE);
    }
}

void SerialDebug_ClearTxQueue(void)
{
    g_serialDebugTxHead = 0U;
    g_serialDebugTxTail = 0U;
}

uint16 SerialDebug_GetTxFree(void)
{
    uint16 used;

    if (g_serialDebugTxHead >= g_serialDebugTxTail)
    {
        used = (uint16)(g_serialDebugTxHead - g_serialDebugTxTail);
    }
    else
    {
        used = (uint16)(SERIAL_DEBUG_TX_BUFFER_SIZE - g_serialDebugTxTail + g_serialDebugTxHead);
    }

    return (uint16)((SERIAL_DEBUG_TX_BUFFER_SIZE - 1U) - used);
}

boolean SerialDebug_EnqueueBytes(const uint8 *data, uint16 length)
{
    uint16 i;

    if ((g_serialDebugInitialized != TRUE) || (data == (const uint8 *)0))
    {
        return FALSE;
    }

    if (length > SerialDebug_GetTxFree())
    {
        return FALSE;
    }

    for (i = 0U; i < length; i++)
    {
        g_serialDebugTxBuffer[g_serialDebugTxHead] = data[i];
        g_serialDebugTxHead = (uint16)((g_serialDebugTxHead + 1U) % SERIAL_DEBUG_TX_BUFFER_SIZE);
    }

    SerialDebug_ServiceTx();
    return TRUE;
}

uint32 SerialDebug_ReadLineBlocking(char *buffer, uint32 bufferSize, boolean echo)
{
    uint32 length = 0U;

    if ((buffer == (char *)0) || (bufferSize == 0U))
    {
        return 0U;
    }

    for (;;)
    {
        char ch = SerialDebug_ReadCharBlocking();

        if ((ch == '\r') || (ch == '\n'))
        {
            if (echo == TRUE)
            {
                SerialDebug_WriteString("\r\n");
            }
            break;
        }

        if ((ch == '\b') || ((uint8)ch == 0x7FU))
        {
            if (length > 0U)
            {
                length--;
                if (echo == TRUE)
                {
                    SerialDebug_WriteString("\b \b");
                }
            }
            continue;
        }

        if ((length + 1U) < bufferSize)
        {
            buffer[length] = ch;
            length++;

            if (echo == TRUE)
            {
                SerialDebug_WriteChar(ch);
            }
        }
    }

    buffer[length] = '\0';
    return length;
}
