#include "telemetry/teensy_link_telemetry.h"

#include <string.h>

#include "../../../../shared/protocol/teensy_link_crc.h"
#include "teensy_config.h"

static void writeU16Le(uint8_t *bytes, uint16_t offset, uint16_t value)
{
    bytes[offset] = (uint8_t)(value & 0xFFU);
    bytes[offset + 1U] = (uint8_t)((value >> 8U) & 0xFFU);
}

static void writeS16Le(uint8_t *bytes, uint16_t offset, int16_t value)
{
    writeU16Le(bytes, offset, (uint16_t)value);
}

static void writeU32Le(uint8_t *bytes, uint16_t offset, uint32_t value)
{
    bytes[offset] = (uint8_t)(value & 0xFFU);
    bytes[offset + 1U] = (uint8_t)((value >> 8U) & 0xFFU);
    bytes[offset + 2U] = (uint8_t)((value >> 16U) & 0xFFU);
    bytes[offset + 3U] = (uint8_t)((value >> 24U) & 0xFFU);
}

static uint16_t readU16Le(const uint8_t *bytes, uint16_t offset)
{
    return (uint16_t)(((uint16_t)bytes[offset]) |
                      ((uint16_t)bytes[offset + 1U] << 8U));
}

static int16_t readS16Le(const uint8_t *bytes, uint16_t offset)
{
    return (int16_t)readU16Le(bytes, offset);
}

static uint32_t readU32Le(const uint8_t *bytes, uint16_t offset)
{
    return (uint32_t)(((uint32_t)bytes[offset]) |
                      ((uint32_t)bytes[offset + 1U] << 8U) |
                      ((uint32_t)bytes[offset + 2U] << 16U) |
                      ((uint32_t)bytes[offset + 3U] << 24U));
}

static int16_t clampFloatToI16(float value)
{
    if (value > 32767.0f)
    {
        return 32767;
    }
    if (value < -32768.0f)
    {
        return -32768;
    }
    return (int16_t)value;
}

static int8_t clampErrorPct(int8_t value)
{
    if (value < -127)
    {
        return -127;
    }
    return value;
}

static void encodeCamera(uint8_t *payload, uint16_t offset, const TeensyLinkCameraResult &camera)
{
    payload[offset + 0U] = (uint8_t)clampErrorPct(camera.errorPct);
    payload[offset + 1U] = camera.status;
    payload[offset + 2U] = camera.feature;
    payload[offset + 3U] = camera.confidence;
    payload[offset + 4U] = camera.leftLineIdx;
    payload[offset + 5U] = camera.rightLineIdx;
    payload[offset + 6U] = camera.ageMs;
    payload[offset + 7U] = camera.flags;
    writeU16Le(payload, offset + 8U, camera.sequence);
}

static void decodeCamera(const uint8_t *payload, uint16_t offset, TeensyLinkCameraResult &camera)
{
    camera.errorPct = (int8_t)payload[offset + 0U];
    camera.status = payload[offset + 1U];
    camera.feature = payload[offset + 2U];
    camera.confidence = payload[offset + 3U];
    camera.leftLineIdx = payload[offset + 4U];
    camera.rightLineIdx = payload[offset + 5U];
    camera.ageMs = payload[offset + 6U];
    camera.flags = payload[offset + 7U];
    camera.sequence = readU16Le(payload, offset + 8U);
}

void TeensyLinkTelemetry_DefaultCamera(TeensyLinkCameraResult &camera, uint8_t sourceFlag)
{
    camera.errorPct = 0;
    camera.status = TEENSY_LINK_CAMERA_STATUS_TRACK_LOST;
    camera.feature = 0U;
    camera.confidence = 0U;
    camera.leftLineIdx = 255U;
    camera.rightLineIdx = 255U;
    camera.ageMs = 255U;
    camera.flags = (uint8_t)(TEENSY_LINK_CAMERA_FLAG_STALE | sourceFlag);
    camera.sequence = 0U;
}

void TeensyLinkTelemetry_BuildTeensyFrame(uint8_t frame[TEENSY_LINK_FRAME_BYTES],
                                          uint16_t frameSeq,
                                          uint32_t nowMs,
                                          uint16_t ackS32kSeq,
                                          const TeensyLinkTelemetryInputs &inputs,
                                          uint16_t teensyRxFrames,
                                          uint16_t teensyRxErrors)
{
    uint8_t *payload = &frame[TEENSY_LINK_HEADER_BYTES];
    uint16_t flags = TEENSY_LINK_FLAG_PAYLOAD_VALID;

    if ((inputs.loggerFlags & TEENSY_LINK_LOGGER_FLAG_READY) != 0U)
    {
        flags |= TEENSY_LINK_FLAG_LOGGER_READY;
    }
    if ((inputs.loggerFlags & TEENSY_LINK_LOGGER_FLAG_ERROR) != 0U)
    {
        flags |= TEENSY_LINK_FLAG_LOGGER_ERROR;
    }
    if (inputs.sensorAgeMs > TEENSY_LINK_STALE_MS)
    {
        flags |= TEENSY_LINK_FLAG_SENSOR_STALE;
    }

    (void)memset(frame, 0, TEENSY_LINK_FRAME_BYTES);

    frame[0] = TEENSY_LINK_SYNC0;
    frame[1] = TEENSY_LINK_SYNC1;
    frame[2] = TEENSY_LINK_VERSION;
    frame[3] = TEENSY_LINK_FRAME_TYPE_TEENSY;
    writeU16Le(frame, 4U, TEENSY_LINK_FRAME_BYTES);
    writeU16Le(frame, 6U, frameSeq);
    writeU32Le(frame, 8U, nowMs);
    writeU16Le(frame, 12U, flags);
    writeU16Le(frame, 14U, TEENSY_LINK_PAYLOAD_BYTES);

    writeU16Le(payload, TEENSY_LINK_TEENSY_ACK_S32K_SEQ_OFF, ackS32kSeq);
    writeU16Le(payload, TEENSY_LINK_TEENSY_SENSOR_SEQ_OFF, inputs.sensorSeq);
    writeU16Le(payload, TEENSY_LINK_TEENSY_SENSOR_DT_US_OFF, inputs.sensorDtUs);
    writeU16Le(payload, TEENSY_LINK_TEENSY_SENSOR_AGE_MS_OFF, inputs.sensorAgeMs);
    writeU16Le(payload, TEENSY_LINK_TEENSY_STATUS_FLAGS_OFF, inputs.statusFlags);
    writeU16Le(payload, TEENSY_LINK_TEENSY_COMPONENT_MASK_OFF, inputs.componentMask);

    encodeCamera(payload, TEENSY_LINK_TEENSY_CAMERA0_OFF, inputs.camera[0]);
    encodeCamera(payload, TEENSY_LINK_TEENSY_CAMERA1_OFF, inputs.camera[1]);

    writeS16Le(payload, TEENSY_LINK_TEENSY_AX_MG_OFF, clampFloatToI16(inputs.imu.axG * 1000.0f));
    writeS16Le(payload, TEENSY_LINK_TEENSY_AY_MG_OFF, clampFloatToI16(inputs.imu.ayG * 1000.0f));
    writeS16Le(payload, TEENSY_LINK_TEENSY_AZ_MG_OFF, clampFloatToI16(inputs.imu.azG * 1000.0f));
    writeS16Le(payload, TEENSY_LINK_TEENSY_GX_DPS10_OFF, clampFloatToI16(inputs.imu.gxDps * 10.0f));
    writeS16Le(payload, TEENSY_LINK_TEENSY_GY_DPS10_OFF, clampFloatToI16(inputs.imu.gyDps * 10.0f));
    writeS16Le(payload, TEENSY_LINK_TEENSY_GZ_DPS10_OFF, clampFloatToI16(inputs.imu.gzDps * 10.0f));
    writeS16Le(payload, TEENSY_LINK_TEENSY_ROLL_CDEG_OFF, clampFloatToI16(inputs.imu.rollDeg * 100.0f));
    writeS16Le(payload, TEENSY_LINK_TEENSY_PITCH_CDEG_OFF, clampFloatToI16(inputs.imu.pitchDeg * 100.0f));
    writeS16Le(payload, TEENSY_LINK_TEENSY_YAW_CDEG_OFF, clampFloatToI16(inputs.imu.yawDeg * 100.0f));
    writeS16Le(payload, TEENSY_LINK_TEENSY_ACCEL_NORM_MG_OFF, clampFloatToI16(inputs.imu.accelNormG * 1000.0f));
    writeS16Le(payload, TEENSY_LINK_TEENSY_LATERAL_MG_OFF, clampFloatToI16(inputs.imu.lateralG * 1000.0f));
    writeS16Le(payload, TEENSY_LINK_TEENSY_TEMP_C10_OFF, clampFloatToI16(inputs.imu.tempC * 10.0f));

    writeU16Le(payload, TEENSY_LINK_TEENSY_LOGGER_FLAGS_OFF, inputs.loggerFlags);
    writeU16Le(payload, TEENSY_LINK_TEENSY_LOGGER_DROPS_OFF, inputs.loggerDropCount);
    writeU16Le(payload, TEENSY_LINK_TEENSY_RX_FRAMES_OFF, teensyRxFrames);
    writeU16Le(payload, TEENSY_LINK_TEENSY_RX_ERRORS_OFF, teensyRxErrors);

    writeU16Le(frame,
               TEENSY_LINK_CRC_OFFSET,
               TeensyLink_Crc16CcittFalse(frame, TEENSY_LINK_CRC_OFFSET));
}

bool TeensyLinkTelemetry_DecodeS32kFrame(const uint8_t frame[TEENSY_LINK_FRAME_BYTES],
                                         TeensyLinkS32kSnapshot &outSnapshot)
{
    const uint8_t *payload = &frame[TEENSY_LINK_HEADER_BYTES];
    uint16_t expectedCrc;
    uint16_t actualCrc;

    (void)memset(&outSnapshot, 0, sizeof(outSnapshot));

    if ((frame[0] != TEENSY_LINK_SYNC0) ||
        (frame[1] != TEENSY_LINK_SYNC1) ||
        (frame[2] != TEENSY_LINK_VERSION) ||
        (frame[3] != TEENSY_LINK_FRAME_TYPE_S32K) ||
        (readU16Le(frame, 4U) != TEENSY_LINK_FRAME_BYTES) ||
        (readU16Le(frame, 14U) != TEENSY_LINK_PAYLOAD_BYTES))
    {
        return false;
    }

    expectedCrc = TeensyLink_Crc16CcittFalse(frame, TEENSY_LINK_CRC_OFFSET);
    actualCrc = readU16Le(frame, TEENSY_LINK_CRC_OFFSET);
    if (expectedCrc != actualCrc)
    {
        return false;
    }

    outSnapshot.valid = true;
    outSnapshot.frameSeq = readU16Le(frame, 6U);
    outSnapshot.timeMs = readU32Le(frame, 8U);
    outSnapshot.flags = readU16Le(frame, 12U);
    outSnapshot.ackTeensySeq = readU16Le(payload, TEENSY_LINK_S32K_ACK_TEENSY_SEQ_OFF);
    outSnapshot.controlLoopSeq = readU16Le(payload, TEENSY_LINK_S32K_CONTROL_SEQ_OFF);
    outSnapshot.controlDtUs = readU16Le(payload, TEENSY_LINK_S32K_CONTROL_DT_US_OFF);
    outSnapshot.appMode = payload[TEENSY_LINK_S32K_APP_MODE_OFF];
    outSnapshot.appState = payload[TEENSY_LINK_S32K_APP_STATE_OFF];
    outSnapshot.safetyFlags = readU16Le(payload, TEENSY_LINK_S32K_SAFETY_FLAGS_OFF);
    decodeCamera(payload, TEENSY_LINK_S32K_CAMERA0_OFF, outSnapshot.camera[0]);
    decodeCamera(payload, TEENSY_LINK_S32K_CAMERA1_OFF, outSnapshot.camera[1]);
    outSnapshot.steerRaw = readS16Le(payload, TEENSY_LINK_S32K_STEER_RAW_OFF);
    outSnapshot.steerFilt = readS16Le(payload, TEENSY_LINK_S32K_STEER_FILT_OFF);
    outSnapshot.steerOut = readS16Le(payload, TEENSY_LINK_S32K_STEER_OUT_OFF);
    outSnapshot.targetSpeedPct = readS16Le(payload, TEENSY_LINK_S32K_TARGET_SPEED_OFF);
    outSnapshot.currentSpeedPct = readS16Le(payload, TEENSY_LINK_S32K_CURRENT_SPEED_OFF);
    outSnapshot.escPrimaryLogical = readS16Le(payload, TEENSY_LINK_S32K_ESC_PRIMARY_OFF);
    outSnapshot.escSecondaryLogical = readS16Le(payload, TEENSY_LINK_S32K_ESC_SECONDARY_OFF);
    outSnapshot.servoCmd = readS16Le(payload, TEENSY_LINK_S32K_SERVO_CMD_OFF);
    outSnapshot.actuatorFlags = readU16Le(payload, TEENSY_LINK_S32K_ACTUATOR_FLAGS_OFF);
    outSnapshot.ultrasonicDistanceCm10 = readU16Le(payload, TEENSY_LINK_S32K_ULTRA_DIST_CM10_OFF);
    outSnapshot.ultrasonicAgeMs = readU16Le(payload, TEENSY_LINK_S32K_ULTRA_AGE_MS_OFF);
    outSnapshot.ultrasonicFlags = readU16Le(payload, TEENSY_LINK_S32K_ULTRA_FLAGS_OFF);
    outSnapshot.diagFlags = readU16Le(payload, TEENSY_LINK_S32K_DIAG_FLAGS_OFF);
    return true;
}
