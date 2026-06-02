#ifndef ESP_UART_LINK_H
#define ESP_UART_LINK_H

#include "Platform_Types.h"
#include "Std_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    boolean sw2Pressed;
    boolean sw3Pressed;
    boolean swPcbOn;
    uint8 sequence;
} EspUartLink_ButtonFrame_t;

typedef EspUartLink_ButtonFrame_t EspUartLink_AckFrame_t;

typedef struct
{
    uint16 txFrames;
    uint16 rxAckFrames;
    uint16 rxProtocolErrors;
    uint16 txErrors;
    uint32 lastAckMs;
    boolean initialized;
    boolean ackValid;
} EspUartLink_Diagnostics_t;

void EspUartLink_Init(void);
Std_ReturnType EspUartLink_SendButtons(const EspUartLink_ButtonFrame_t *buttons);
void EspUartLink_Poll(uint32 nowMs);
boolean EspUartLink_GetLastAck(EspUartLink_AckFrame_t *outAck);
void EspUartLink_GetDiagnostics(EspUartLink_Diagnostics_t *outDiagnostics);

#ifdef __cplusplus
}
#endif

#endif /* ESP_UART_LINK_H */
