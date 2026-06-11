#ifndef TEENSY_LINK_H
#define TEENSY_LINK_H

#include "Platform_Types.h"
#include "Std_Types.h"

#include "../../../../shared/protocol/teensy_link_protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Master-side transfer scheduling. The wire format does not change.

   - The service keeps being called every TEENSY_LINK_SERVICE_PERIOD_MS (5 ms).
   - A real SPI transfer happens at most every TEENSY_LINK_TRANSFER_PERIOD_MS,
     because the Teensy only produces new sensor data at 100 Hz. Clocking
     faster just reads the same frame twice and burns CPU.
   - A transfer only starts when the Teensy READY pin is high. The Teensy
     lowers READY during its blocking SD chunk writes and display I2C
     updates, so READY low usually means "busy for a few tens of ms".
   - If READY stays low much longer than any normal busy window, one probe
     transfer still goes out every TEENSY_LINK_HEARTBEAT_MS so a broken
     READY wire, an unpowered Teensy, or a boards-powered-in-any-order
     start cannot silently kill the link. The link recovers by itself as
     soon as the Teensy answers a probe. */
#define TEENSY_LINK_TRANSFER_PERIOD_MS    (10u)
#define TEENSY_LINK_HEARTBEAT_MS          (500u)

/* Transfer engine selection.

   STD_ON  = eDMA moves the 128-byte frame while the CPU keeps running.
             The service call only starts the transfer and picks up the
             finished frame on a later tick. The very first transfer after
             init still uses the blocking path, because that path programs
             the LPSPI clock registers exactly like the proven setup.
   STD_OFF = the original blocking Spi_SyncTransmit path (~512 us of CPU
             per frame). Keep this as the fallback if DMA misbehaves. */
#define TEENSY_LINK_USE_DMA               (STD_ON)

/* eDMA channels used by the link. The project's generated DMA config has
   ZERO logic channels (Dma_Ip_VS_0_PBcfg.c is empty), so no other driver
   uses eDMA today. If ConfigTools ever adds DMA users, keep these unique. */
#define TEENSY_LINK_DMA_RX_CHANNEL        (0u)
#define TEENSY_LINK_DMA_TX_CHANNEL        (1u)

/* DMAMUX request sources, S32K144 reference manual "DMA MUX request sources":
   LPSPI1 RX = 16, LPSPI1 TX = 17. Must match the LPSPI instance in the
   generated TeensySpiDevice config (instance 1). */
#define TEENSY_LINK_DMAMUX_SRC_LPSPI1_RX  (16u)
#define TEENSY_LINK_DMAMUX_SRC_LPSPI1_TX  (17u)

/* A DMA frame takes ~512 us on the wire. If it is not done after this many
   5 ms service ticks, something is stuck and the transfer is aborted. */
#define TEENSY_LINK_DMA_TIMEOUT_TICKS     (3u)

typedef struct
{
    sint8 errorPct;
    uint8 status;
    uint8 feature;
    uint8 confidence;
    uint8 leftLineIdx;
    uint8 rightLineIdx;
    uint8 ageMs;
    uint8 flags;
} TeensyLinkCameraResult_t;

typedef struct
{
    sint16 axMg;
    sint16 ayMg;
    sint16 azMg;
    sint16 gxDps10;
    sint16 gyDps10;
    sint16 gzDps10;
    sint16 rollCdeg;
    sint16 pitchCdeg;
    sint16 yawCdeg;
    sint16 accelNormMg;
    sint16 lateralMg;
    sint16 tempC10;
} TeensyLinkImuData_t;

typedef struct
{
    float axG;
    float ayG;
    float azG;
    float gxDps;
    float gyDps;
    float gzDps;
    float rollDeg;
    float pitchDeg;
    float yawDeg;
    float accelNormG;
    float lateralG;
    float tempC;
} TeensyLinkImuMotion_t;

typedef struct
{
    uint16 controlLoopSeq;
    uint16 controlDtUs;
    uint8 appMode;
    uint8 appState;
    uint16 safetyFlags;
    TeensyLinkCameraResult_t camera[TEENSY_LINK_CAMERA_COUNT];
    sint16 steerRaw;
    sint16 steerFilt;
    sint16 steerOut;
    sint16 targetSpeedPct;
    sint16 currentSpeedPct;
    sint16 escPrimaryLogical;
    sint16 escSecondaryLogical;
    sint16 servoCmd;
    uint16 actuatorFlags;
    uint16 ultrasonicDistanceCm10;
    uint16 ultrasonicAgeMs;
    uint16 ultrasonicFlags;
} TeensyLinkS32kInputs_t;

typedef struct
{
    boolean haveFrame;
    boolean live;
    uint32 lastRxMs;
    uint16 s32kSeq;
    uint16 teensySeq;
    uint16 ackS32kSeq;
    uint16 flags;
    uint16 sensorDtUs;
    uint16 sensorAgeMs;
    uint16 statusFlags;
    uint16 componentMask;
    TeensyLinkCameraResult_t camera[TEENSY_LINK_CAMERA_COUNT];
    TeensyLinkImuData_t imu;
    uint16 loggerFlags;
    uint16 loggerDropCount;
    uint16 teensyRxFrameCount;
    uint16 teensyRxErrorCount;
} TeensyLinkSnapshot_t;

typedef struct
{
    uint16 txSeq;
    uint16 lastTeensySeq;
    uint32 lastRxMs;
    uint32 txCount;
    uint32 goodFrameCount;
    uint32 spiErrorCount;
    uint32 crcErrorCount;
    uint32 protocolErrorCount;
    uint32 staleCount;
    uint32 notReadySkipCount;   /* service ticks skipped: READY low */
    uint32 heartbeatProbeCount; /* transfers forced while READY was low */
    uint32 dmaStartCount;       /* transfers handed to eDMA */
    uint32 dmaTimeoutCount;     /* DMA transfers aborted by the tick timeout */
    boolean dmaBusy;            /* a DMA transfer is in flight right now */
    boolean readyHigh;
    Std_ReturnType lastSpiResult;
} TeensyLinkDiagnostics_t;

void TeensyLink_Init(void);
Std_ReturnType TeensyLink_Service5ms(uint32 nowMs, const TeensyLinkS32kInputs_t *in);
boolean TeensyLink_GetSnapshot(TeensyLinkSnapshot_t *outSnapshot);
boolean TeensyLink_GetDiagnostics(TeensyLinkDiagnostics_t *outDiagnostics);
boolean TeensyLink_ImuToMotion(const TeensyLinkImuData_t *imu, TeensyLinkImuMotion_t *outMotion);
float TeensyLink_EstimateSlipG(const TeensyLinkImuMotion_t *motion, float speedMps);

#ifdef __cplusplus
}
#endif

#endif /* TEENSY_LINK_H */
