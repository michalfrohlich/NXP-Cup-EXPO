#include "debug/uart_host_link.h"
#include "config/board_config.h"

/* Temporary S32K UART transport.
 * This module still reaches below the generated UART API for RX polling and
 * should be treated as board-specific bring-up/debug code, not portable logic.
 */

#include "CDD_Uart.h"
#include "S32K144.h"

#define UART_HOST_WRITE_TIMEOUT_US 10000U

static boolean g_uartHostInitialized = FALSE;
static uint8 g_uartHostTxBuffer[UART_HOST_TX_BUFFER_SIZE];
static volatile uint16 g_uartHostTxHead = 0U;
static volatile uint16 g_uartHostTxTail = 0U;

void UartHost_Init(void)
{
    Uart_Init(NULL_PTR);

    /* The generated UART config is interrupt-mode only. Keep RX enabled here so
       the existing tuning shell can still poll single characters without an ISR. */
    /* TODO(serial): Replace this proof-of-concept RX polling with generated
       UART async receive plus an LPUART1 IRQ/callback ring buffer. */
    IP_LPUART1->CTRL |= LPUART_CTRL_RE_MASK;

    g_uartHostTxHead = 0U;
    g_uartHostTxTail = 0U;

    g_uartHostInitialized = TRUE;
}

boolean UartHost_IsRxReady(void)
{
    if (g_uartHostInitialized != TRUE)
    {
        return FALSE;
    }

    return ((IP_LPUART1->STAT & LPUART_STAT_RDRF_MASK) != 0U) ? TRUE : FALSE;
}

boolean UartHost_TryReadChar(char *ch)
{
    if ((ch == (char *)0) || (UartHost_IsRxReady() != TRUE))
    {
        return FALSE;
    }

    *ch = (char)IP_LPUART1->DATA;
    return TRUE;
}

char UartHost_ReadCharBlocking(void)
{
    while (UartHost_IsRxReady() != TRUE)
    {
    }

    return (char)IP_LPUART1->DATA;
}

boolean UartHost_IsTxReady(void)
{
    return (g_uartHostInitialized == TRUE) ? TRUE : FALSE;
}

boolean UartHost_TryWriteChar(char ch)
{
    uint8 data = (uint8)ch;

    if (g_uartHostInitialized != TRUE)
    {
        return FALSE;
    }

    return (Uart_SyncSend(UART_HOST_UART_CHANNEL,
                          &data,
                          1U,
                          UART_HOST_WRITE_TIMEOUT_US) == E_OK) ? TRUE : FALSE;
}

void UartHost_WriteChar(char ch)
{
    if (g_uartHostInitialized != TRUE)
    {
        return;
    }

    while (UartHost_TryWriteChar(ch) != TRUE)
    {
    }
}

void UartHost_WriteString(const char *text)
{
    if (text == (const char *)0)
    {
        return;
    }

    while (*text != '\0')
    {
        UartHost_WriteChar(*text);
        text++;
    }
}

void UartHost_WriteLine(const char *text)
{
    UartHost_WriteString(text);
    UartHost_WriteString("\r\n");
}

void UartHost_ServiceTx(void)
{
    if (g_uartHostInitialized != TRUE)
    {
        return;
    }

    while ((g_uartHostTxHead != g_uartHostTxTail) &&
           (UartHost_IsTxReady() == TRUE))
    {
        if (UartHost_TryWriteChar((char)g_uartHostTxBuffer[g_uartHostTxTail]) != TRUE)
        {
            break;
        }
        g_uartHostTxTail = (uint16)((g_uartHostTxTail + 1U) % UART_HOST_TX_BUFFER_SIZE);
    }
}

void UartHost_ClearTxQueue(void)
{
    g_uartHostTxHead = 0U;
    g_uartHostTxTail = 0U;
}

uint16 UartHost_GetTxFree(void)
{
    uint16 used;

    if (g_uartHostTxHead >= g_uartHostTxTail)
    {
        used = (uint16)(g_uartHostTxHead - g_uartHostTxTail);
    }
    else
    {
        used = (uint16)(UART_HOST_TX_BUFFER_SIZE - g_uartHostTxTail + g_uartHostTxHead);
    }

    return (uint16)((UART_HOST_TX_BUFFER_SIZE - 1U) - used);
}

boolean UartHost_EnqueueBytes(const uint8 *data, uint16 length)
{
    uint16 i;

    if ((g_uartHostInitialized != TRUE) || (data == (const uint8 *)0))
    {
        return FALSE;
    }

    if (length > UartHost_GetTxFree())
    {
        return FALSE;
    }

    for (i = 0U; i < length; i++)
    {
        g_uartHostTxBuffer[g_uartHostTxHead] = data[i];
        g_uartHostTxHead = (uint16)((g_uartHostTxHead + 1U) % UART_HOST_TX_BUFFER_SIZE);
    }

    UartHost_ServiceTx();
    return TRUE;
}

uint32 UartHost_ReadLineBlocking(char *buffer, uint32 bufferSize, boolean echo)
{
    uint32 length = 0U;

    if ((buffer == (char *)0) || (bufferSize == 0U))
    {
        return 0U;
    }

    for (;;)
    {
        char ch = UartHost_ReadCharBlocking();

        if ((ch == '\r') || (ch == '\n'))
        {
            if (echo == TRUE)
            {
                UartHost_WriteString("\r\n");
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
                    UartHost_WriteString("\b \b");
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
                UartHost_WriteChar(ch);
            }
        }
    }

    buffer[length] = '\0';
    return length;
}
