#ifndef UART_HOST_LINK_H
#define UART_HOST_LINK_H

#include "Platform_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Temporary UART debug service.
   Pin muxing and UART baud/frame setup are generated from .mex. The service
   keeps the existing byte-oriented tuning API on top of the generated driver. */

#define UART_HOST_BAUD_RATE 115200U
#define UART_HOST_TX_BUFFER_SIZE 512U

void UartHost_Init(void);

boolean UartHost_IsRxReady(void);
boolean UartHost_TryReadChar(char *ch);
char UartHost_ReadCharBlocking(void);

boolean UartHost_IsTxReady(void);
boolean UartHost_TryWriteChar(char ch);
void UartHost_WriteChar(char ch);
void UartHost_WriteString(const char *text);
void UartHost_WriteLine(const char *text);
void UartHost_ServiceTx(void);
void UartHost_ClearTxQueue(void);
uint16 UartHost_GetTxFree(void);
boolean UartHost_EnqueueBytes(const uint8 *data, uint16 length);

uint32 UartHost_ReadLineBlocking(char *buffer, uint32 bufferSize, boolean echo);

#ifdef __cplusplus
}
#endif

#endif /* UART_HOST_LINK_H */
