#include "serial_debug.h"

#include "S32K144.h"

#define SERIAL_DEBUG_UART_RX_PIN 6U
#define SERIAL_DEBUG_UART_TX_PIN 7U

static boolean g_serialDebugInitialized = FALSE;
static uint8 g_serialDebugTxBuffer[SERIAL_DEBUG_TX_BUFFER_SIZE];
static volatile uint16 g_serialDebugTxHead = 0U;
static volatile uint16 g_serialDebugTxTail = 0U;

static void SerialDebug_InitPins(void)
{
    IP_PCC->PCCn[PCC_PORTC_INDEX] |= PCC_PCCn_CGC_MASK;

    /* Temporary proof-of-concept muxing:
       PTC6 -> LPUART1_RX (ALT2)
       PTC7 -> LPUART1_TX (ALT2) */
    IP_PORTC->PCR[SERIAL_DEBUG_UART_RX_PIN] = PORT_PCR_MUX(2U);
    IP_PORTC->PCR[SERIAL_DEBUG_UART_TX_PIN] = PORT_PCR_MUX(2U);
}

static void SerialDebug_InitClocking(void)
{
    /* Re-enable SIRCDIV outputs so LPUART1 can use the same known 8 MHz source
       as the teammate demo, independent of .mex UART configuration. */
    IP_SCG->SIRCDIV = SCG_SIRCDIV_SIRCDIV1(1U) | SCG_SIRCDIV_SIRCDIV2(1U);

    IP_PCC->PCCn[PCC_LPUART1_INDEX] &= ~PCC_PCCn_CGC_MASK;
    IP_PCC->PCCn[PCC_LPUART1_INDEX] = PCC_PCCn_PCS(2U) | PCC_PCCn_CGC_MASK;
}

void SerialDebug_Init(void)
{
    SerialDebug_InitPins();
    SerialDebug_InitClocking();

    /* 8 MHz / (14 * 5) = 114285 baud, close enough for 115200 debug streaming. */
    IP_LPUART1->BAUD = LPUART_BAUD_SBR(5U) | LPUART_BAUD_OSR(13U);
    IP_LPUART1->CTRL = LPUART_CTRL_RE_MASK | LPUART_CTRL_TE_MASK;

    g_serialDebugTxHead = 0U;
    g_serialDebugTxTail = 0U;

    g_serialDebugInitialized = TRUE;
}

boolean SerialDebug_IsRxReady(void)
{
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
    if (g_serialDebugInitialized != TRUE)
    {
        return FALSE;
    }

    return ((IP_LPUART1->STAT & LPUART_STAT_TDRE_MASK) != 0U) ? TRUE : FALSE;
}

boolean SerialDebug_TryWriteChar(char ch)
{
    if (SerialDebug_IsTxReady() != TRUE)
    {
        return FALSE;
    }

    IP_LPUART1->DATA = (uint8)ch;
    return TRUE;
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
        IP_LPUART1->DATA = g_serialDebugTxBuffer[g_serialDebugTxTail];
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
