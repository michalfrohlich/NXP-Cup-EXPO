#include "drivers/teensy_imu.h"

#include <string.h>

static TeensyImuSnapshot_t g_teensyImuSnapshot;

static uint16 read_u16_le(const uint8 *bytes, uint16 offset)
{
    return (uint16)(((uint16)bytes[offset]) |
                    ((uint16)bytes[offset + 1U] << 8U));
}

static sint16 read_s16_le(const uint8 *bytes, uint16 offset)
{
    return (sint16)read_u16_le(bytes, offset);
}

static uint32 read_u32_le(const uint8 *bytes, uint16 offset)
{
    return (uint32)(((uint32)bytes[offset]) |
                    ((uint32)bytes[offset + 1U] << 8U) |
                    ((uint32)bytes[offset + 2U] << 16U) |
                    ((uint32)bytes[offset + 3U] << 24U));
}

static void write_u16_le(uint8 *bytes, uint16 offset, uint16 value)
{
    bytes[offset] = (uint8)(value & 0xFFU);
    bytes[offset + 1U] = (uint8)((value >> 8U) & 0xFFU);
}

static void write_s16_le(uint8 *bytes, uint16 offset, sint16 value)
{
    write_u16_le(bytes, offset, (uint16)value);
}

static void write_u32_le(uint8 *bytes, uint16 offset, uint32 value)
{
    bytes[offset] = (uint8)(value & 0xFFU);
    bytes[offset + 1U] = (uint8)((value >> 8U) & 0xFFU);
    bytes[offset + 2U] = (uint8)((value >> 16U) & 0xFFU);
    bytes[offset + 3U] = (uint8)((value >> 24U) & 0xFFU);
}

static uint8 checksum_packet(const uint8 *bytes, uint16 length)
{
    uint8 checksum = 0U;

    for (uint16 i = 2U; i < (uint16)(length - 1U); i++)
    {
        checksum ^= bytes[i];
    }

    return checksum;
}

void TeensyImu_Init(void)
{
    (void)memset(&g_teensyImuSnapshot, 0, sizeof(g_teensyImuSnapshot));
}

boolean TeensyImu_DecodePacket(const uint8 *bytes, uint16 length, TeensyImuPacket_t *outPacket)
{
    TeensyImuPacket_t packet;

    if ((bytes == NULL_PTR) || (outPacket == NULL_PTR))
    {
        return FALSE;
    }

    if (length != (uint16)TEENSY_IMU_PACKET_BYTES)
    {
        return FALSE;
    }

    if ((bytes[0] != (uint8)TEENSY_IMU_SYNC0) ||
        (bytes[1] != (uint8)TEENSY_IMU_SYNC1))
    {
        return FALSE;
    }

    if (checksum_packet(bytes, length) != bytes[TEENSY_IMU_PACKET_BYTES - 1U])
    {
        return FALSE;
    }

    packet.sync0 = bytes[0];
    packet.sync1 = bytes[1];
    packet.version = bytes[2];
    packet.type = bytes[3];
    packet.packetBytes = read_u16_le(bytes, 4U);
    packet.sequence = read_u16_le(bytes, 6U);
    packet.timeMs = read_u32_le(bytes, 8U);
    packet.statusFlags = read_u16_le(bytes, 12U);
    packet.componentMask = read_u16_le(bytes, 14U);
    packet.camera1Status = bytes[16];
    packet.camera2Status = bytes[17];
    packet.sampleDtUs = read_u16_le(bytes, 18U);
    packet.axMg = read_s16_le(bytes, 20U);
    packet.ayMg = read_s16_le(bytes, 22U);
    packet.azMg = read_s16_le(bytes, 24U);
    packet.gxDps10 = read_s16_le(bytes, 26U);
    packet.gyDps10 = read_s16_le(bytes, 28U);
    packet.gzDps10 = read_s16_le(bytes, 30U);
    packet.rollCdeg = read_s16_le(bytes, 32U);
    packet.pitchCdeg = read_s16_le(bytes, 34U);
    packet.yawCdeg = read_s16_le(bytes, 36U);
    packet.accelNormMg = read_s16_le(bytes, 38U);
    packet.lateralMg = read_s16_le(bytes, 40U);
    packet.tempC10 = read_s16_le(bytes, 42U);
    packet.checksum = bytes[44];

    if ((packet.version != (uint8)TEENSY_IMU_PACKET_VERSION) ||
        (packet.type != (uint8)TEENSY_IMU_PACKET_TYPE) ||
        (packet.packetBytes != (uint16)TEENSY_IMU_PACKET_BYTES))
    {
        return FALSE;
    }

    *outPacket = packet;
    return TRUE;
}

boolean TeensyImu_SubmitRxBytes(const uint8 *bytes, uint16 length, uint32 nowMs)
{
    TeensyImuPacket_t packet;

    if (TeensyImu_DecodePacket(bytes, length, &packet) != TRUE)
    {
        g_teensyImuSnapshot.errorCount++;
        return FALSE;
    }

    g_teensyImuSnapshot.packet = packet;
    g_teensyImuSnapshot.lastRxMs = nowMs;
    g_teensyImuSnapshot.goodPacketCount++;
    g_teensyImuSnapshot.havePacket = TRUE;
    return TRUE;
}

boolean TeensyImu_GetSnapshot(TeensyImuSnapshot_t *outSnapshot)
{
    if (outSnapshot == NULL_PTR)
    {
        return FALSE;
    }

    *outSnapshot = g_teensyImuSnapshot;
    return g_teensyImuSnapshot.havePacket;
}

void TeensyImu_InjectDemoSample(uint32 nowMs)
{
    static uint16 seq = 0U;
    uint8 bytes[TEENSY_IMU_PACKET_BYTES];
    sint16 yawCdeg;

    (void)memset(bytes, 0, sizeof(bytes));

    yawCdeg = (sint16)(((sint32)(seq % 360U) - 180) * 100);

    bytes[0] = (uint8)TEENSY_IMU_SYNC0;
    bytes[1] = (uint8)TEENSY_IMU_SYNC1;
    bytes[2] = (uint8)TEENSY_IMU_PACKET_VERSION;
    bytes[3] = (uint8)TEENSY_IMU_PACKET_TYPE;
    write_u16_le(bytes, 4U, (uint16)TEENSY_IMU_PACKET_BYTES);
    write_u16_le(bytes, 6U, seq);
    write_u32_le(bytes, 8U, nowMs);
    write_u16_le(bytes, 12U,
                 (uint16)(TEENSY_IMU_STATUS_PRESENT |
                          TEENSY_IMU_STATUS_CALIBRATED |
                          TEENSY_IMU_STATUS_DATA_VALID |
                          TEENSY_IMU_STATUS_ACCEL_TRUSTED |
                          TEENSY_IMU_STATUS_YAW_RELATIVE));
    write_u16_le(bytes, 14U, 1U);
    bytes[16] = (uint8)TEENSY_IMU_CAMERA_ON_S32K;
    bytes[17] = (uint8)TEENSY_IMU_CAMERA_NOT_CONNECTED;
    write_u16_le(bytes, 18U, 5000U);
    write_s16_le(bytes, 20U, 20);
    write_s16_le(bytes, 22U, -35);
    write_s16_le(bytes, 24U, 995);
    write_s16_le(bytes, 26U, 1);
    write_s16_le(bytes, 28U, -2);
    write_s16_le(bytes, 30U, 75);
    write_s16_le(bytes, 32U, 120);
    write_s16_le(bytes, 34U, -210);
    write_s16_le(bytes, 36U, yawCdeg);
    write_s16_le(bytes, 38U, 1000);
    write_s16_le(bytes, 40U, -35);
    write_s16_le(bytes, 42U, 260);
    bytes[44] = checksum_packet(bytes, (uint16)TEENSY_IMU_PACKET_BYTES);

    (void)TeensyImu_SubmitRxBytes(bytes, (uint16)TEENSY_IMU_PACKET_BYTES, nowMs);
    seq++;
}

const char *TeensyImu_CameraStatusText(uint8 status)
{
    switch (status)
    {
        case TEENSY_IMU_CAMERA_ON_S32K:
            return "S32";
        case TEENSY_IMU_CAMERA_ON_TEENSY:
            return "TEN";
        case TEENSY_IMU_CAMERA_ERROR:
            return "ERR";
        case TEENSY_IMU_CAMERA_NOT_CONNECTED:
        default:
            return "NC ";
    }
}
