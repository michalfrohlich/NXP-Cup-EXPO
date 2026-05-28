#include "drivers/teensy_link.h"

#include <string.h>

#include "Dio.h"
#include "Spi.h"
#include "Spi_Cfg.h"
#include "Dio_Cfg.h"

static Spi_DataBufferType g_teensyLinkTx[TEENSY_LINK_FRAME_BYTES];
static Spi_DataBufferType g_teensyLinkRx[TEENSY_LINK_FRAME_BYTES];
static TeensyLinkSnapshot_t g_teensyLinkSnapshot;
static TeensyLinkDiagnostics_t g_teensyLinkDiag;
static boolean g_teensyLinkStaleCounted;

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

static uint16 read_u16_le(const uint8 *bytes, uint16 offset)
{
    return (uint16)(((uint16)bytes[offset]) |
                    ((uint16)bytes[offset + 1U] << 8U));
}

static sint16 read_s16_le(const uint8 *bytes, uint16 offset)
{
    return (sint16)read_u16_le(bytes, offset);
}

static uint16 crc16_ccitt_false(const uint8 *bytes, uint16 length)
{
    uint16 crc = 0xFFFFU;

    for (uint16 i = 0U; i < length; i++)
    {
        crc ^= (uint16)((uint16)bytes[i] << 8U);

        for (uint8 bit = 0U; bit < 8U; bit++)
        {
            if ((crc & 0x8000U) != 0U)
            {
                crc = (uint16)((crc << 1U) ^ 0x1021U);
            }
            else
            {
                crc = (uint16)(crc << 1U);
            }
        }
    }

    return crc;
}

static sint8 clamp_s16_to_s8(sint16 value)
{
    if (value > 127)
    {
        return 127;
    }
    if (value < -127)
    {
        return -127;
    }
    return (sint8)value;
}

static void encode_camera(uint8 *payload, uint16 offset, const TeensyLinkCameraResult_t *camera)
{
    if (camera == NULL_PTR)
    {
        payload[offset + 0U] = (uint8)0U;
        payload[offset + 1U] = (uint8)0U;
        payload[offset + 2U] = (uint8)0U;
        payload[offset + 3U] = (uint8)0U;
        payload[offset + 4U] = (uint8)255U;
        payload[offset + 5U] = (uint8)255U;
        payload[offset + 6U] = (uint8)255U;
        payload[offset + 7U] = (uint8)TEENSY_LINK_CAMERA_FLAG_STALE;
        return;
    }

    payload[offset + 0U] = (uint8)clamp_s16_to_s8((sint16)camera->errorPct);
    payload[offset + 1U] = camera->status;
    payload[offset + 2U] = camera->feature;
    payload[offset + 3U] = camera->confidence;
    payload[offset + 4U] = camera->leftLineIdx;
    payload[offset + 5U] = camera->rightLineIdx;
    payload[offset + 6U] = camera->ageMs;
    payload[offset + 7U] = camera->flags;
}

static void decode_camera(const uint8 *payload, uint16 offset, TeensyLinkCameraResult_t *camera)
{
    if (camera == NULL_PTR)
    {
        return;
    }

    camera->errorPct = (sint8)payload[offset + 0U];
    camera->status = payload[offset + 1U];
    camera->feature = payload[offset + 2U];
    camera->confidence = payload[offset + 3U];
    camera->leftLineIdx = payload[offset + 4U];
    camera->rightLineIdx = payload[offset + 5U];
    camera->ageMs = payload[offset + 6U];
    camera->flags = payload[offset + 7U];
}

static void build_s32k_frame(uint32 nowMs, const TeensyLinkS32kInputs_t *in)
{
    uint8 *tx = (uint8 *)g_teensyLinkTx;
    uint8 *payload = &tx[TEENSY_LINK_HEADER_BYTES];
    uint16 flags = 0U;
    uint16 crc;

    (void)memset(tx, 0, TEENSY_LINK_FRAME_BYTES);

    if (g_teensyLinkDiag.readyHigh == TRUE)
    {
        flags |= (uint16)TEENSY_LINK_FLAG_READY_HIGH;
    }
    flags |= (uint16)TEENSY_LINK_FLAG_PAYLOAD_VALID;

    tx[0] = (uint8)TEENSY_LINK_SYNC0;
    tx[1] = (uint8)TEENSY_LINK_SYNC1;
    tx[2] = (uint8)TEENSY_LINK_VERSION;
    tx[3] = (uint8)TEENSY_LINK_FRAME_TYPE_S32K;
    write_u16_le(tx, 4U, (uint16)TEENSY_LINK_FRAME_BYTES);
    write_u16_le(tx, 6U, g_teensyLinkDiag.txSeq);
    write_u32_le(tx, 8U, nowMs);
    write_u16_le(tx, 12U, flags);
    write_u16_le(tx, 14U, (uint16)TEENSY_LINK_PAYLOAD_BYTES);

    write_u16_le(payload, TEENSY_LINK_S32K_ACK_TEENSY_SEQ_OFF, g_teensyLinkSnapshot.teensySeq);

    if (in != NULL_PTR)
    {
        write_u16_le(payload, TEENSY_LINK_S32K_CONTROL_SEQ_OFF, in->controlLoopSeq);
        write_u16_le(payload, TEENSY_LINK_S32K_CONTROL_DT_US_OFF, in->controlDtUs);
        payload[TEENSY_LINK_S32K_APP_MODE_OFF] = in->appMode;
        payload[TEENSY_LINK_S32K_APP_STATE_OFF] = in->appState;
        write_u16_le(payload, TEENSY_LINK_S32K_SAFETY_FLAGS_OFF, in->safetyFlags);
        encode_camera(payload, TEENSY_LINK_S32K_CAMERA0_OFF, &in->camera[0]);
        encode_camera(payload, TEENSY_LINK_S32K_CAMERA1_OFF, &in->camera[1]);
        write_s16_le(payload, TEENSY_LINK_S32K_STEER_RAW_OFF, in->steerRaw);
        write_s16_le(payload, TEENSY_LINK_S32K_STEER_FILT_OFF, in->steerFilt);
        write_s16_le(payload, TEENSY_LINK_S32K_STEER_OUT_OFF, in->steerOut);
        write_s16_le(payload, TEENSY_LINK_S32K_TARGET_SPEED_OFF, in->targetSpeedPct);
        write_s16_le(payload, TEENSY_LINK_S32K_CURRENT_SPEED_OFF, in->currentSpeedPct);
        write_s16_le(payload, TEENSY_LINK_S32K_ESC_PRIMARY_OFF, in->escPrimaryLogical);
        write_s16_le(payload, TEENSY_LINK_S32K_ESC_SECONDARY_OFF, in->escSecondaryLogical);
        write_s16_le(payload, TEENSY_LINK_S32K_SERVO_CMD_OFF, in->servoCmd);
        write_u16_le(payload, TEENSY_LINK_S32K_ACTUATOR_FLAGS_OFF, in->actuatorFlags);
        write_u16_le(payload, TEENSY_LINK_S32K_ULTRA_DIST_CM10_OFF, in->ultrasonicDistanceCm10);
        write_u16_le(payload, TEENSY_LINK_S32K_ULTRA_AGE_MS_OFF, in->ultrasonicAgeMs);
        write_u16_le(payload, TEENSY_LINK_S32K_ULTRA_FLAGS_OFF, in->ultrasonicFlags);
    }
    else
    {
        encode_camera(payload, TEENSY_LINK_S32K_CAMERA0_OFF, NULL_PTR);
        encode_camera(payload, TEENSY_LINK_S32K_CAMERA1_OFF, NULL_PTR);
    }

    write_u16_le(payload, TEENSY_LINK_S32K_DIAG_FLAGS_OFF, flags);

    crc = crc16_ccitt_false(tx, (uint16)TEENSY_LINK_CRC_OFFSET);
    write_u16_le(tx, (uint16)TEENSY_LINK_CRC_OFFSET, crc);
}

static boolean validate_teensy_frame(const uint8 *rx)
{
    uint16 expectedCrc;
    uint16 actualCrc;

    if ((rx[0] != (uint8)TEENSY_LINK_SYNC0) ||
        (rx[1] != (uint8)TEENSY_LINK_SYNC1) ||
        (rx[2] != (uint8)TEENSY_LINK_VERSION) ||
        (rx[3] != (uint8)TEENSY_LINK_FRAME_TYPE_TEENSY) ||
        (read_u16_le(rx, 4U) != (uint16)TEENSY_LINK_FRAME_BYTES) ||
        (read_u16_le(rx, 14U) != (uint16)TEENSY_LINK_PAYLOAD_BYTES))
    {
        g_teensyLinkDiag.protocolErrorCount++;
        return FALSE;
    }

    expectedCrc = crc16_ccitt_false(rx, (uint16)TEENSY_LINK_CRC_OFFSET);
    actualCrc = read_u16_le(rx, (uint16)TEENSY_LINK_CRC_OFFSET);

    if (expectedCrc != actualCrc)
    {
        g_teensyLinkDiag.crcErrorCount++;
        return FALSE;
    }

    return TRUE;
}

static void decode_teensy_frame(const uint8 *rx, uint32 nowMs)
{
    const uint8 *payload = &rx[TEENSY_LINK_HEADER_BYTES];
    uint16 previousTeensySeq = g_teensyLinkSnapshot.teensySeq;
    uint16 txSeqPrevious = (uint16)(g_teensyLinkDiag.txSeq - 1U);

    g_teensyLinkSnapshot.haveFrame = TRUE;
    g_teensyLinkSnapshot.lastRxMs = nowMs;
    g_teensyLinkSnapshot.s32kSeq = g_teensyLinkDiag.txSeq;
    g_teensyLinkSnapshot.flags = read_u16_le(rx, 12U);
    g_teensyLinkSnapshot.ackS32kSeq = read_u16_le(payload, TEENSY_LINK_TEENSY_ACK_S32K_SEQ_OFF);
    g_teensyLinkSnapshot.teensySeq = read_u16_le(payload, TEENSY_LINK_TEENSY_SENSOR_SEQ_OFF);
    g_teensyLinkSnapshot.sensorDtUs = read_u16_le(payload, TEENSY_LINK_TEENSY_SENSOR_DT_US_OFF);
    g_teensyLinkSnapshot.sensorAgeMs = read_u16_le(payload, TEENSY_LINK_TEENSY_SENSOR_AGE_MS_OFF);
    g_teensyLinkSnapshot.statusFlags = read_u16_le(payload, TEENSY_LINK_TEENSY_STATUS_FLAGS_OFF);
    g_teensyLinkSnapshot.componentMask = read_u16_le(payload, TEENSY_LINK_TEENSY_COMPONENT_MASK_OFF);

    decode_camera(payload, TEENSY_LINK_TEENSY_CAMERA0_OFF, &g_teensyLinkSnapshot.camera[0]);
    decode_camera(payload, TEENSY_LINK_TEENSY_CAMERA1_OFF, &g_teensyLinkSnapshot.camera[1]);

    g_teensyLinkSnapshot.imu.axMg = read_s16_le(payload, TEENSY_LINK_TEENSY_AX_MG_OFF);
    g_teensyLinkSnapshot.imu.ayMg = read_s16_le(payload, TEENSY_LINK_TEENSY_AY_MG_OFF);
    g_teensyLinkSnapshot.imu.azMg = read_s16_le(payload, TEENSY_LINK_TEENSY_AZ_MG_OFF);
    g_teensyLinkSnapshot.imu.gxDps10 = read_s16_le(payload, TEENSY_LINK_TEENSY_GX_DPS10_OFF);
    g_teensyLinkSnapshot.imu.gyDps10 = read_s16_le(payload, TEENSY_LINK_TEENSY_GY_DPS10_OFF);
    g_teensyLinkSnapshot.imu.gzDps10 = read_s16_le(payload, TEENSY_LINK_TEENSY_GZ_DPS10_OFF);
    g_teensyLinkSnapshot.imu.rollCdeg = read_s16_le(payload, TEENSY_LINK_TEENSY_ROLL_CDEG_OFF);
    g_teensyLinkSnapshot.imu.pitchCdeg = read_s16_le(payload, TEENSY_LINK_TEENSY_PITCH_CDEG_OFF);
    g_teensyLinkSnapshot.imu.yawCdeg = read_s16_le(payload, TEENSY_LINK_TEENSY_YAW_CDEG_OFF);
    g_teensyLinkSnapshot.imu.accelNormMg = read_s16_le(payload, TEENSY_LINK_TEENSY_ACCEL_NORM_MG_OFF);
    g_teensyLinkSnapshot.imu.lateralMg = read_s16_le(payload, TEENSY_LINK_TEENSY_LATERAL_MG_OFF);
    g_teensyLinkSnapshot.imu.tempC10 = read_s16_le(payload, TEENSY_LINK_TEENSY_TEMP_C10_OFF);

    g_teensyLinkSnapshot.loggerFlags = read_u16_le(payload, TEENSY_LINK_TEENSY_LOGGER_FLAGS_OFF);
    g_teensyLinkSnapshot.loggerDropCount = read_u16_le(payload, TEENSY_LINK_TEENSY_LOGGER_DROPS_OFF);
    g_teensyLinkSnapshot.teensyRxFrameCount = read_u16_le(payload, TEENSY_LINK_TEENSY_RX_FRAMES_OFF);
    g_teensyLinkSnapshot.teensyRxErrorCount = read_u16_le(payload, TEENSY_LINK_TEENSY_RX_ERRORS_OFF);

    g_teensyLinkSnapshot.live =
        ((g_teensyLinkSnapshot.ackS32kSeq == g_teensyLinkDiag.txSeq) ||
         (g_teensyLinkSnapshot.ackS32kSeq == txSeqPrevious) ||
         (g_teensyLinkSnapshot.teensySeq != previousTeensySeq)) ? TRUE : FALSE;

    g_teensyLinkDiag.lastTeensySeq = g_teensyLinkSnapshot.teensySeq;
    g_teensyLinkDiag.goodFrameCount++;
    g_teensyLinkDiag.lastRxMs = nowMs;
    g_teensyLinkStaleCounted = FALSE;
}

static void update_stale_state(uint32 nowMs)
{
    if (g_teensyLinkSnapshot.haveFrame != TRUE)
    {
        g_teensyLinkSnapshot.live = FALSE;
        return;
    }

    if ((uint32)(nowMs - g_teensyLinkSnapshot.lastRxMs) > (uint32)TEENSY_LINK_STALE_MS)
    {
        g_teensyLinkSnapshot.live = FALSE;
        if (g_teensyLinkStaleCounted != TRUE)
        {
            g_teensyLinkDiag.staleCount++;
            g_teensyLinkStaleCounted = TRUE;
        }
    }
}

void TeensyLink_Init(void)
{
    (void)memset(g_teensyLinkTx, 0, sizeof(g_teensyLinkTx));
    (void)memset(g_teensyLinkRx, 0, sizeof(g_teensyLinkRx));
    (void)memset(&g_teensyLinkSnapshot, 0, sizeof(g_teensyLinkSnapshot));
    (void)memset(&g_teensyLinkDiag, 0, sizeof(g_teensyLinkDiag));
    g_teensyLinkDiag.lastSpiResult = E_NOT_OK;
    g_teensyLinkStaleCounted = FALSE;
}

Std_ReturnType TeensyLink_Service5ms(uint32 nowMs, const TeensyLinkS32kInputs_t *in)
{
    Std_ReturnType ret;

    g_teensyLinkDiag.readyHigh =
        (Dio_ReadChannel(DioConf_DioChannel_TeensyReady) == STD_HIGH) ? TRUE : FALSE;

    g_teensyLinkDiag.txSeq++;
    build_s32k_frame(nowMs, in);

    ret = Spi_SetupEB(SpiConf_SpiChannel_TeensySpiFrameChannel,
                      g_teensyLinkTx,
                      g_teensyLinkRx,
                      (Spi_NumberOfDataType)TEENSY_LINK_FRAME_BYTES);

    if (ret == E_OK)
    {
        ret = Spi_SyncTransmit(SpiConf_SpiSequence_TeensySpiSequence);
    }

    g_teensyLinkDiag.lastSpiResult = ret;
    g_teensyLinkDiag.txCount++;

    if (ret != E_OK)
    {
        g_teensyLinkDiag.spiErrorCount++;
        update_stale_state(nowMs);
        return ret;
    }

    if (validate_teensy_frame((const uint8 *)g_teensyLinkRx) == TRUE)
    {
        decode_teensy_frame((const uint8 *)g_teensyLinkRx, nowMs);
    }
    else
    {
        update_stale_state(nowMs);
    }

    return ret;
}

boolean TeensyLink_GetSnapshot(TeensyLinkSnapshot_t *outSnapshot)
{
    if (outSnapshot == NULL_PTR)
    {
        return FALSE;
    }

    *outSnapshot = g_teensyLinkSnapshot;
    return g_teensyLinkSnapshot.haveFrame;
}

boolean TeensyLink_GetDiagnostics(TeensyLinkDiagnostics_t *outDiagnostics)
{
    if (outDiagnostics == NULL_PTR)
    {
        return FALSE;
    }

    *outDiagnostics = g_teensyLinkDiag;
    return TRUE;
}
