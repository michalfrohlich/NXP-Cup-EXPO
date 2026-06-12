#ifndef TEENSY_LINK_PROTOCOL_H
#define TEENSY_LINK_PROTOCOL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TEENSY_LINK_SYNC0                       (0xA5u)
#define TEENSY_LINK_SYNC1                       (0x5Au)
#define TEENSY_LINK_VERSION                     (2u)

#define TEENSY_LINK_FRAME_TYPE_S32K             (0x53u)
#define TEENSY_LINK_FRAME_TYPE_TEENSY           (0x54u)

#define TEENSY_LINK_FRAME_BYTES                 (84u)
#define TEENSY_LINK_HEADER_BYTES                (16u)
#define TEENSY_LINK_PAYLOAD_BYTES               (66u)
#define TEENSY_LINK_CRC_OFFSET                  (82u)

#define TEENSY_LINK_CAMERA_COUNT                (2u)
#define TEENSY_LINK_CAMERA_WIRE_BYTES           (10u)

#define TEENSY_LINK_STALE_MS                    (100u)
#define TEENSY_LINK_SERVICE_PERIOD_MS           (5u)

#define TEENSY_LINK_FLAG_READY_HIGH             (1u << 0)
#define TEENSY_LINK_FLAG_PAYLOAD_VALID          (1u << 1)
#define TEENSY_LINK_FLAG_SENSOR_STALE           (1u << 2)
#define TEENSY_LINK_FLAG_LOGGER_READY           (1u << 3)
#define TEENSY_LINK_FLAG_LOGGER_ERROR           (1u << 4)

#define TEENSY_LINK_CAMERA_FLAG_VALID           (1u << 0)
#define TEENSY_LINK_CAMERA_FLAG_STALE           (1u << 1)
#define TEENSY_LINK_CAMERA_FLAG_SOURCE_TEENSY   (1u << 2)
#define TEENSY_LINK_CAMERA_FLAG_SOURCE_S32K     (1u << 3)

#define TEENSY_LINK_LOGGER_FLAG_READY           (1u << 0)
#define TEENSY_LINK_LOGGER_FLAG_WRITING         (1u << 1)
#define TEENSY_LINK_LOGGER_FLAG_ERROR           (1u << 2)

#define TEENSY_LINK_S32K_DIAG_SPI_SLOT_MISS         (1u << 0)
#define TEENSY_LINK_S32K_DIAG_CONTROL_DEADLINE_MISS (1u << 1)
#define TEENSY_LINK_S32K_DIAG_VISION_NO_SAMPLE      (1u << 2)
#define TEENSY_LINK_S32K_DIAG_VISION_OUTLIER        (1u << 3)
#define TEENSY_LINK_S32K_DIAG_SERVO_COMMIT_MISS     (1u << 4)

#define TEENSY_LINK_S32K_ACK_TEENSY_SEQ_OFF     (0u)
#define TEENSY_LINK_S32K_CONTROL_SEQ_OFF        (2u)
#define TEENSY_LINK_S32K_CONTROL_DT_US_OFF      (4u)
#define TEENSY_LINK_S32K_APP_MODE_OFF           (6u)
#define TEENSY_LINK_S32K_APP_STATE_OFF          (7u)
#define TEENSY_LINK_S32K_SAFETY_FLAGS_OFF       (8u)
#define TEENSY_LINK_S32K_CAMERA0_OFF            (10u)
#define TEENSY_LINK_S32K_CAMERA1_OFF            (20u)
#define TEENSY_LINK_S32K_STEER_RAW_OFF          (30u)
#define TEENSY_LINK_S32K_STEER_FILT_OFF         (32u)
#define TEENSY_LINK_S32K_STEER_OUT_OFF          (34u)
#define TEENSY_LINK_S32K_TARGET_SPEED_OFF       (36u)
#define TEENSY_LINK_S32K_CURRENT_SPEED_OFF      (38u)
#define TEENSY_LINK_S32K_ESC_PRIMARY_OFF        (40u)
#define TEENSY_LINK_S32K_ESC_SECONDARY_OFF      (42u)
#define TEENSY_LINK_S32K_SERVO_CMD_OFF          (44u)
#define TEENSY_LINK_S32K_ACTUATOR_FLAGS_OFF     (46u)
#define TEENSY_LINK_S32K_ULTRA_DIST_CM10_OFF    (48u)
#define TEENSY_LINK_S32K_ULTRA_AGE_MS_OFF       (50u)
#define TEENSY_LINK_S32K_ULTRA_FLAGS_OFF        (52u)
#define TEENSY_LINK_S32K_DIAG_FLAGS_OFF         (54u)

#define TEENSY_LINK_TEENSY_ACK_S32K_SEQ_OFF     (0u)
#define TEENSY_LINK_TEENSY_SENSOR_SEQ_OFF       (2u)
#define TEENSY_LINK_TEENSY_SENSOR_DT_US_OFF     (4u)
#define TEENSY_LINK_TEENSY_SENSOR_AGE_MS_OFF    (6u)
#define TEENSY_LINK_TEENSY_STATUS_FLAGS_OFF     (8u)
#define TEENSY_LINK_TEENSY_COMPONENT_MASK_OFF   (10u)
#define TEENSY_LINK_TEENSY_CAMERA0_OFF          (12u)
#define TEENSY_LINK_TEENSY_CAMERA1_OFF          (22u)
#define TEENSY_LINK_TEENSY_AX_MG_OFF            (32u)
#define TEENSY_LINK_TEENSY_AY_MG_OFF            (34u)
#define TEENSY_LINK_TEENSY_AZ_MG_OFF            (36u)
#define TEENSY_LINK_TEENSY_GX_DPS10_OFF         (38u)
#define TEENSY_LINK_TEENSY_GY_DPS10_OFF         (40u)
#define TEENSY_LINK_TEENSY_GZ_DPS10_OFF         (42u)
#define TEENSY_LINK_TEENSY_ROLL_CDEG_OFF        (44u)
#define TEENSY_LINK_TEENSY_PITCH_CDEG_OFF       (46u)
#define TEENSY_LINK_TEENSY_YAW_CDEG_OFF         (48u)
#define TEENSY_LINK_TEENSY_ACCEL_NORM_MG_OFF    (50u)
#define TEENSY_LINK_TEENSY_LATERAL_MG_OFF       (52u)
#define TEENSY_LINK_TEENSY_TEMP_C10_OFF         (54u)
#define TEENSY_LINK_TEENSY_LOGGER_FLAGS_OFF     (56u)
#define TEENSY_LINK_TEENSY_LOGGER_DROPS_OFF     (58u)
#define TEENSY_LINK_TEENSY_RX_FRAMES_OFF        (60u)
#define TEENSY_LINK_TEENSY_RX_ERRORS_OFF        (62u)

#if ((TEENSY_LINK_HEADER_BYTES + TEENSY_LINK_PAYLOAD_BYTES + 2u) != TEENSY_LINK_FRAME_BYTES)
#error "Teensy link frame size does not match header, payload, and CRC sizes"
#endif

#if ((TEENSY_LINK_S32K_DIAG_FLAGS_OFF + 2u) > TEENSY_LINK_PAYLOAD_BYTES)
#error "S32K payload fields exceed the Teensy link payload"
#endif

#if ((TEENSY_LINK_TEENSY_RX_ERRORS_OFF + 2u) > TEENSY_LINK_PAYLOAD_BYTES)
#error "Teensy payload fields exceed the Teensy link payload"
#endif

typedef struct
{
    int8_t errorPct;
    uint8_t status;
    uint8_t feature;
    uint8_t confidence;
    uint8_t leftLineIdx;
    uint8_t rightLineIdx;
    uint8_t ageMs;
    uint8_t flags;
    uint16_t sequence;
} TeensyLinkCameraWire_t;

#ifdef __cplusplus
}
#endif

#endif /* TEENSY_LINK_PROTOCOL_H */
