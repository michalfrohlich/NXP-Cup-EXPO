#ifndef SERIAL_DEBUG_H
#define SERIAL_DEBUG_H

#include "Platform_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Temporary handwritten UART debug service.
   This bypasses generated .mex/RTD UART config on purpose for the current
   proof-of-concept and uses the teammate demo wiring and baud rate. */

#define SERIAL_DEBUG_BAUD_RATE 115200U
#define SERIAL_DEBUG_TX_BUFFER_SIZE 512U

void SerialDebug_Init(void);

boolean SerialDebug_IsRxReady(void);
boolean SerialDebug_TryReadChar(char *ch);
char SerialDebug_ReadCharBlocking(void);

boolean SerialDebug_IsTxReady(void);
boolean SerialDebug_TryWriteChar(char ch);
void SerialDebug_WriteChar(char ch);
void SerialDebug_WriteString(const char *text);
void SerialDebug_WriteLine(const char *text);
void SerialDebug_ServiceTx(void);
void SerialDebug_ClearTxQueue(void);
uint16 SerialDebug_GetTxFree(void);
boolean SerialDebug_EnqueueBytes(const uint8 *data, uint16 length);

uint32 SerialDebug_ReadLineBlocking(char *buffer, uint32 bufferSize, boolean echo);

#ifdef __cplusplus
}
#endif

#endif /* SERIAL_DEBUG_H */
