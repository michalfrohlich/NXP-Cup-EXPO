#ifndef TEENSY_LINK_H
#define TEENSY_LINK_H

#include "Platform_Types.h"
#include "Std_Types.h"

#include "../../../../shared/protocol/teensy_link_protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

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
    uint16 sequence;
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
    uint16 diagnosticFlags;
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
    uint8 rawRxHeader[6];
    boolean readyHigh;
    Std_ReturnType lastSpiResult;
} TeensyLinkDiagnostics_t;

void TeensyLink_Init(void);
Std_ReturnType TeensyLink_Service(uint32 nowMs, const TeensyLinkS32kInputs_t *in);
boolean TeensyLink_GetSnapshot(TeensyLinkSnapshot_t *outSnapshot);
boolean TeensyLink_GetDiagnostics(TeensyLinkDiagnostics_t *outDiagnostics);
boolean TeensyLink_ImuToMotion(const TeensyLinkImuData_t *imu, TeensyLinkImuMotion_t *outMotion);
float TeensyLink_EstimateSlipG(const TeensyLinkImuMotion_t *motion, float speedMps);

#ifdef __cplusplus
}
#endif

#endif /* TEENSY_LINK_H */
