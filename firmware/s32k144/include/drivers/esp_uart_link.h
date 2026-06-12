#ifndef ESP_UART_LINK_H
#define ESP_UART_LINK_H

#include "Platform_Types.h"
#include "Std_Types.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define ESP_UART_LINK_TX_PERIOD_MS (50U)

    typedef struct
    {
        boolean sw2Pressed;
        boolean sw3Pressed;
        boolean swPcbOn;
        uint8 sequence;
        uint16 timestampMs;
    } EspUartLink_ButtonFrame_t;

    typedef EspUartLink_ButtonFrame_t EspUartLink_AckFrame_t;

    typedef struct
    {
        uint8 sequence;
        uint32 proportionalMilli;
        uint32 integralMilli;
        uint32 derivativeMilli;
        uint8 steerClamp;
        uint16 steerLpfMilli;
        uint16 minContrast;
        uint8 edgeHighPct;
        uint8 edgeLowPct;
    } EspUartLink_TuneFrame_t;

    typedef struct
    {
        uint16 txFrames;
        uint16 txQueuedFrames;
        uint16 txOverwrites;
        uint16 txTuneResults;
        uint16 rxAckFrames;
        uint16 rxTuneFrames;
        uint16 rxTuneOverwrites;
        uint16 rxBytes;
        uint16 rxHardwareErrors;
        uint16 rxOverrunErrors;
        uint16 rxNoiseErrors;
        uint16 rxFramingErrors;
        uint16 rxParityErrors;
        uint16 rxProtocolErrors;
        uint16 txErrors;
        uint32 lastTxMs;
        uint32 lastAckMs;
        uint32 lastTuneMs;
        uint16 lastAckAgeMs;
        boolean initialized;
        boolean txBusy;
        boolean txPending;
        boolean ackValid;
        boolean tunePending;
    } EspUartLink_Diagnostics_t;

    void EspUartLink_Init(void);
    Std_ReturnType EspUartLink_QueueButtons(const EspUartLink_ButtonFrame_t *buttons);
    void EspUartLink_Service(uint32 nowMs);
    Std_ReturnType EspUartLink_SendButtons(const EspUartLink_ButtonFrame_t *buttons);
    void EspUartLink_Poll(uint32 nowMs);
    boolean EspUartLink_GetLastAck(EspUartLink_AckFrame_t *outAck);
    boolean EspUartLink_TakeTune(EspUartLink_TuneFrame_t *outTune);
    Std_ReturnType EspUartLink_QueueTuneResult(uint8 sequence, boolean accepted);
    void EspUartLink_GetDiagnostics(EspUartLink_Diagnostics_t *outDiagnostics);

#ifdef __cplusplus
}
#endif

#endif /* ESP_UART_LINK_H */
