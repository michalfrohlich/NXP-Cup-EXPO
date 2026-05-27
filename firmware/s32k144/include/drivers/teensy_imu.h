#ifndef TEENSY_IMU_H
#define TEENSY_IMU_H

#include "Std_Types.h"

/* Teensy -> S32K packet framing.
   The S32K receives these bytes from the generated SPI callback. */
#define TEENSY_IMU_SYNC0                 (0xA5U)
#define TEENSY_IMU_SYNC1                 (0x5AU)
#define TEENSY_IMU_PACKET_VERSION        (1U)
#define TEENSY_IMU_PACKET_TYPE           (0x49U)
#define TEENSY_IMU_PACKET_BYTES          (45U)

/* Camera ownership reported by the Teensy packet. */
typedef enum
{
    TEENSY_IMU_CAMERA_NOT_CONNECTED = 0U,
    TEENSY_IMU_CAMERA_ON_S32K       = 1U,
    TEENSY_IMU_CAMERA_ON_TEENSY     = 2U,
    TEENSY_IMU_CAMERA_ERROR         = 3U
} TeensyImuCameraStatus_t;

/* Runtime health flags from the Teensy IMU bridge. */
typedef enum
{
    TEENSY_IMU_STATUS_PRESENT       = 1U << 0,
    TEENSY_IMU_STATUS_CALIBRATED    = 1U << 1,
    TEENSY_IMU_STATUS_DATA_VALID    = 1U << 2,
    TEENSY_IMU_STATUS_ACCEL_TRUSTED = 1U << 3,
    TEENSY_IMU_STATUS_SD_READY      = 1U << 4,
    TEENSY_IMU_STATUS_YAW_RELATIVE  = 1U << 5
} TeensyImuStatusFlag_t;

/* Decoded 45-byte packet. Units stay integer-friendly for the display. */
typedef struct
{
    uint8 sync0;
    uint8 sync1;
    uint8 version;
    uint8 type;
    uint16 packetBytes;
    uint16 sequence;
    uint32 timeMs;
    uint16 statusFlags;
    uint16 componentMask;
    uint8 camera1Status;
    uint8 camera2Status;
    uint16 sampleDtUs;
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
    uint8 checksum;
} TeensyImuPacket_t;

/* Latest accepted packet plus counters for the display test. */
typedef struct
{
    TeensyImuPacket_t packet;
    uint32 lastRxMs;
    uint16 goodPacketCount;
    uint16 errorCount;
    boolean havePacket;
} TeensyImuSnapshot_t;

void TeensyImu_Init(void);
boolean TeensyImu_DecodePacket(const uint8 *bytes, uint16 length, TeensyImuPacket_t *outPacket);

/* Call this from the future S32K SPI receive callback. */
boolean TeensyImu_SubmitRxBytes(const uint8 *bytes, uint16 length, uint32 nowMs);
boolean TeensyImu_GetSnapshot(TeensyImuSnapshot_t *outSnapshot);

/* Local display/parser test until real SPI bytes are wired in. */
void TeensyImu_InjectDemoSample(uint32 nowMs);
const char *TeensyImu_CameraStatusText(uint8 status);

#endif /* TEENSY_IMU_H */
