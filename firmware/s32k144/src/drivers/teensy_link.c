#include "drivers/teensy_link.h"

#include <string.h>

#include "Dio.h"
#include "Spi.h"
#include "Spi_Cfg.h"
#include "Dio_Cfg.h"
#include "../../../../shared/protocol/teensy_link_crc.h"

#if (TEENSY_LINK_USE_DMA == STD_ON)
/* Raw register access for the DMA fast path. The AUTOSAR Spi driver in this
   project is generated sync-only (SPI_LEVEL0, SPI_IPW_DMA_USED off) and the
   generated eDMA config has no logic channels, so the link drives two free
   eDMA channels directly. Spi_Init() still owns the LPSPI module setup. */
#include "S32K144_DMA.h"
#include "S32K144_DMAMUX.h"
#include "S32K144_LPSPI.h"
#include "Lpspi_Ip_Types.h"

/* Generated bus settings of the Teensy SPI device (instance 1, mode 0,
   PCS3, continuous CS). Declared in generate/src/Lpspi_Ip_VS_0_PBcfg.c. */
extern const Lpspi_Ip_ExternalDeviceType Lpspi_Ip_DeviceAttributes_TeensySpiDevice_Instance_1_VS_0;
#endif

#define TEENSY_LINK_DEG_TO_RAD_F  (0.0174532925f)
#define TEENSY_LINK_GRAVITY_MPS2  (9.80665f)

static Spi_DataBufferType g_teensyLinkTx[TEENSY_LINK_FRAME_BYTES];
static Spi_DataBufferType g_teensyLinkRx[TEENSY_LINK_FRAME_BYTES];
static TeensyLinkSnapshot_t g_teensyLinkSnapshot;
static TeensyLinkDiagnostics_t g_teensyLinkDiag;
static boolean g_teensyLinkStaleCounted;
static uint32 g_teensyLinkLastTransferMs;
static boolean g_teensyLinkHaveTransferred;

#if (TEENSY_LINK_USE_DMA == STD_ON)
static boolean g_teensyLinkDmaPrimed;
static uint32 g_teensyLinkDmaTcrCommand;
static uint8 g_teensyLinkDmaPollTicks;
#endif

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

    crc = TeensyLink_Crc16CcittFalse(tx, (uint16)TEENSY_LINK_CRC_OFFSET);
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

    expectedCrc = TeensyLink_Crc16CcittFalse(rx, (uint16)TEENSY_LINK_CRC_OFFSET);
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

/* The original blocking exchange: CPU sits in Spi_SyncTransmit for the
   whole ~512 us wire time. Used for every transfer when DMA is off, and
   for the first transfer after init when DMA is on (it lets the proven
   Spi driver program the LPSPI clock registers before DMA reuses them). */
static Std_ReturnType teensy_link_transfer_blocking(uint32 nowMs)
{
    Std_ReturnType ret;

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

#if (TEENSY_LINK_USE_DMA == STD_ON)

#define TEENSY_LINK_LPSPI_SR_W1C_MASK \
    (LPSPI_SR_WCF_MASK | LPSPI_SR_FCF_MASK | LPSPI_SR_TCF_MASK | \
     LPSPI_SR_TEF_MASK | LPSPI_SR_REF_MASK | LPSPI_SR_DMF_MASK)

/* One-time wiring of the two eDMA channels. Static TCD fields stay valid
   for every frame because SLAST/DLASTSGA rewind the buffer addresses by
   -128 each time a major loop (one full frame) completes. */
static void teensy_link_dma_setup(void)
{
    DMA_Type *dma = IP_DMA;
    DMAMUX_Type *mux = IP_DMAMUX;
    LPSPI_Type *spi = IP_LPSPI1;

    /* Keep the mux routing disabled while the descriptors change. */
    mux->CHCFG[TEENSY_LINK_DMA_RX_CHANNEL] = 0U;
    mux->CHCFG[TEENSY_LINK_DMA_TX_CHANNEL] = 0U;

    /* RX: one byte per request from the LPSPI receive register into the
       RX frame buffer, 128 times. */
    dma->TCD[TEENSY_LINK_DMA_RX_CHANNEL].SADDR = (uint32)&spi->RDR;
    dma->TCD[TEENSY_LINK_DMA_RX_CHANNEL].SOFF = 0;
    dma->TCD[TEENSY_LINK_DMA_RX_CHANNEL].ATTR = 0U; /* 8-bit reads/writes */
    dma->TCD[TEENSY_LINK_DMA_RX_CHANNEL].NBYTES.MLNO = 1U;
    dma->TCD[TEENSY_LINK_DMA_RX_CHANNEL].SLAST = 0U;
    dma->TCD[TEENSY_LINK_DMA_RX_CHANNEL].DADDR = (uint32)&g_teensyLinkRx[0];
    dma->TCD[TEENSY_LINK_DMA_RX_CHANNEL].DOFF = 1;
    dma->TCD[TEENSY_LINK_DMA_RX_CHANNEL].CITER.ELINKNO = (uint16)TEENSY_LINK_FRAME_BYTES;
    dma->TCD[TEENSY_LINK_DMA_RX_CHANNEL].DLASTSGA = (uint32)(-(sint32)TEENSY_LINK_FRAME_BYTES);
    dma->TCD[TEENSY_LINK_DMA_RX_CHANNEL].BITER.ELINKNO = (uint16)TEENSY_LINK_FRAME_BYTES;
    dma->TCD[TEENSY_LINK_DMA_RX_CHANNEL].CSR = DMA_TCD_CSR_DREQ_MASK;

    /* TX: one byte per request from the TX frame buffer into the LPSPI
       transmit register, 128 times. */
    dma->TCD[TEENSY_LINK_DMA_TX_CHANNEL].SADDR = (uint32)&g_teensyLinkTx[0];
    dma->TCD[TEENSY_LINK_DMA_TX_CHANNEL].SOFF = 1;
    dma->TCD[TEENSY_LINK_DMA_TX_CHANNEL].ATTR = 0U;
    dma->TCD[TEENSY_LINK_DMA_TX_CHANNEL].NBYTES.MLNO = 1U;
    dma->TCD[TEENSY_LINK_DMA_TX_CHANNEL].SLAST = (uint32)(-(sint32)TEENSY_LINK_FRAME_BYTES);
    dma->TCD[TEENSY_LINK_DMA_TX_CHANNEL].DADDR = (uint32)&spi->TDR;
    dma->TCD[TEENSY_LINK_DMA_TX_CHANNEL].DOFF = 0;
    dma->TCD[TEENSY_LINK_DMA_TX_CHANNEL].CITER.ELINKNO = (uint16)TEENSY_LINK_FRAME_BYTES;
    dma->TCD[TEENSY_LINK_DMA_TX_CHANNEL].DLASTSGA = 0U;
    dma->TCD[TEENSY_LINK_DMA_TX_CHANNEL].BITER.ELINKNO = (uint16)TEENSY_LINK_FRAME_BYTES;
    dma->TCD[TEENSY_LINK_DMA_TX_CHANNEL].CSR = DMA_TCD_CSR_DREQ_MASK;

    mux->CHCFG[TEENSY_LINK_DMA_RX_CHANNEL] =
        (uint8)(DMAMUX_CHCFG_ENBL_MASK | DMAMUX_CHCFG_SOURCE(TEENSY_LINK_DMAMUX_SRC_LPSPI1_RX));
    mux->CHCFG[TEENSY_LINK_DMA_TX_CHANNEL] =
        (uint8)(DMAMUX_CHCFG_ENBL_MASK | DMAMUX_CHCFG_SOURCE(TEENSY_LINK_DMAMUX_SRC_LPSPI1_TX));

    /* Same bus command the blocking driver sends: mode 0, PCS3, CS held
       for the whole frame (CONT=1), plus 8-bit frames, MSB first. */
    g_teensyLinkDmaTcrCommand =
        Lpspi_Ip_DeviceAttributes_TeensySpiDevice_Instance_1_VS_0.Tcr |
        LPSPI_TCR_FRAMESZ(7U);
}

/* Drop chip-select the same way the blocking driver ends a continuous
   transfer, and stop the LPSPI from raising further DMA requests. */
static void teensy_link_dma_release_bus(void)
{
    LPSPI_Type *spi = IP_LPSPI1;

    spi->DER = 0U;
    spi->TCR &= ~(LPSPI_TCR_CONT_MASK | LPSPI_TCR_CONTC_MASK);
}

static boolean teensy_link_dma_done(void)
{
    return ((IP_DMA->TCD[TEENSY_LINK_DMA_RX_CHANNEL].CSR & DMA_TCD_CSR_DONE_MASK) != 0U)
               ? TRUE : FALSE;
}

/* Hard stop: kill both channel requests, flush the FIFOs, release CS.
   Used on timeout and when (re)entering the test. */
static void teensy_link_dma_abort(void)
{
    DMA_Type *dma = IP_DMA;
    LPSPI_Type *spi = IP_LPSPI1;

    dma->CERQ = (uint8)TEENSY_LINK_DMA_RX_CHANNEL;
    dma->CERQ = (uint8)TEENSY_LINK_DMA_TX_CHANNEL;
    dma->CDNE = (uint8)TEENSY_LINK_DMA_RX_CHANNEL;
    dma->CDNE = (uint8)TEENSY_LINK_DMA_TX_CHANNEL;

    teensy_link_dma_release_bus();
    spi->CR |= (LPSPI_CR_RTF_MASK | LPSPI_CR_RRF_MASK);
    spi->SR = TEENSY_LINK_LPSPI_SR_W1C_MASK;

    g_teensyLinkDiag.dmaBusy = FALSE;
}

/* Kick one 128-byte full-duplex frame and return immediately. The CPU is
   free for the whole ~512 us wire time; completion is picked up by the
   next service tick via teensy_link_dma_done(). */
static void teensy_link_dma_start(void)
{
    DMA_Type *dma = IP_DMA;
    LPSPI_Type *spi = IP_LPSPI1;

    spi->SR = TEENSY_LINK_LPSPI_SR_W1C_MASK;
    spi->CR |= (LPSPI_CR_RTF_MASK | LPSPI_CR_RRF_MASK);
    /* Ask for TX service while the FIFO is nearly empty and RX service as
       soon as one byte arrives. */
    spi->FCR = LPSPI_FCR_TXWATER(1U) | LPSPI_FCR_RXWATER(0U);

    dma->CDNE = (uint8)TEENSY_LINK_DMA_RX_CHANNEL;
    dma->CDNE = (uint8)TEENSY_LINK_DMA_TX_CHANNEL;

    /* Queue the bus command first, then arm RX before TX so the receive
       side can never fall behind the transmit side. */
    spi->TCR = g_teensyLinkDmaTcrCommand;
    dma->SERQ = (uint8)TEENSY_LINK_DMA_RX_CHANNEL;
    dma->SERQ = (uint8)TEENSY_LINK_DMA_TX_CHANNEL;
    spi->DER = (LPSPI_DER_RDDE_MASK | LPSPI_DER_TDDE_MASK);

    g_teensyLinkDmaPollTicks = 0U;
    g_teensyLinkDiag.dmaBusy = TRUE;
    g_teensyLinkDiag.dmaStartCount++;
}

#endif /* TEENSY_LINK_USE_DMA */

void TeensyLink_Init(void)
{
    (void)memset(g_teensyLinkTx, 0, sizeof(g_teensyLinkTx));
    (void)memset(g_teensyLinkRx, 0, sizeof(g_teensyLinkRx));
    (void)memset(&g_teensyLinkSnapshot, 0, sizeof(g_teensyLinkSnapshot));
    (void)memset(&g_teensyLinkDiag, 0, sizeof(g_teensyLinkDiag));
    g_teensyLinkDiag.lastSpiResult = E_NOT_OK;
    g_teensyLinkStaleCounted = FALSE;
    g_teensyLinkLastTransferMs = 0U;
    g_teensyLinkHaveTransferred = FALSE;

#if (TEENSY_LINK_USE_DMA == STD_ON)
    /* Kill any leftover in-flight transfer from a previous test run, then
       (re)build the channel descriptors. Safe to repeat at every entry. */
    teensy_link_dma_abort();
    teensy_link_dma_setup();
    g_teensyLinkDmaPrimed = FALSE;
    g_teensyLinkDmaPollTicks = 0U;
#endif
}

Std_ReturnType TeensyLink_Service5ms(uint32 nowMs, const TeensyLinkS32kInputs_t *in)
{
    uint32 sinceLastMs;

    g_teensyLinkDiag.readyHigh =
        (Dio_ReadChannel(DioConf_DioChannel_TeensyReady) == STD_HIGH) ? TRUE : FALSE;

#if (TEENSY_LINK_USE_DMA == STD_ON)
    if (g_teensyLinkDiag.dmaBusy == TRUE)
    {
        if (teensy_link_dma_done() == TRUE)
        {
            /* Frame finished in the background. Release CS, then run the
               exact same validation/decoding as the blocking path. */
            teensy_link_dma_release_bus();
            g_teensyLinkDiag.dmaBusy = FALSE;
            g_teensyLinkDiag.lastSpiResult = E_OK;
            g_teensyLinkDiag.txCount++;

            if (validate_teensy_frame((const uint8 *)g_teensyLinkRx) == TRUE)
            {
                decode_teensy_frame((const uint8 *)g_teensyLinkRx, nowMs);
            }
            else
            {
                update_stale_state(nowMs);
            }
            /* Fall through: the scheduler below may start the next one. */
        }
        else
        {
            g_teensyLinkDmaPollTicks++;
            if (g_teensyLinkDmaPollTicks >= (uint8)TEENSY_LINK_DMA_TIMEOUT_TICKS)
            {
                /* The wire time is ~512 us; if the frame is not done after
                   several 5 ms ticks, the request chain is stuck. Abort so
                   the test can never hang on DMA. */
                teensy_link_dma_abort();
                g_teensyLinkDiag.dmaTimeoutCount++;
                g_teensyLinkDiag.spiErrorCount++;
                g_teensyLinkDiag.lastSpiResult = E_NOT_OK;
                g_teensyLinkDiag.txCount++;
            }
            update_stale_state(nowMs);
            return E_OK;
        }
    }
#endif

    /* The very first service call always transfers, so a fresh test
       starts probing immediately. After that the scheduler decides. */
    if (g_teensyLinkHaveTransferred == TRUE)
    {
        sinceLastMs = nowMs - g_teensyLinkLastTransferMs;

        /* The Teensy refreshes its sensor frame at 100 Hz. Clocking the
           bus faster than that only re-reads the same data. */
        if (sinceLastMs < (uint32)TEENSY_LINK_TRANSFER_PERIOD_MS)
        {
            update_stale_state(nowMs);
            return E_OK;
        }

        if (g_teensyLinkDiag.readyHigh != TRUE)
        {
            /* Teensy says it is not ready. Leave the bus alone, unless
               it has been quiet so long that we probe anyway to find
               out whether only the READY wire is broken. */
            if (sinceLastMs < (uint32)TEENSY_LINK_HEARTBEAT_MS)
            {
                g_teensyLinkDiag.notReadySkipCount++;
                update_stale_state(nowMs);
                return E_OK;
            }
            g_teensyLinkDiag.heartbeatProbeCount++;
        }
    }

    g_teensyLinkLastTransferMs = nowMs;
    g_teensyLinkHaveTransferred = TRUE;

    g_teensyLinkDiag.txSeq++;
    build_s32k_frame(nowMs, in);

#if (TEENSY_LINK_USE_DMA == STD_ON)
    if (g_teensyLinkDmaPrimed != TRUE)
    {
        /* Transfer #1 goes through the proven blocking path so the Spi
           driver programs the LPSPI clock/mode registers. Every later
           frame reuses that setup with DMA. */
        g_teensyLinkDmaPrimed = TRUE;
        return teensy_link_transfer_blocking(nowMs);
    }

    teensy_link_dma_start();
    return E_OK;
#else
    return teensy_link_transfer_blocking(nowMs);
#endif
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

boolean TeensyLink_ImuToMotion(const TeensyLinkImuData_t *imu, TeensyLinkImuMotion_t *outMotion)
{
    if ((imu == NULL_PTR) || (outMotion == NULL_PTR))
    {
        return FALSE;
    }

    outMotion->axG = (float)imu->axMg * 0.001f;
    outMotion->ayG = (float)imu->ayMg * 0.001f;
    outMotion->azG = (float)imu->azMg * 0.001f;
    outMotion->gxDps = (float)imu->gxDps10 * 0.1f;
    outMotion->gyDps = (float)imu->gyDps10 * 0.1f;
    outMotion->gzDps = (float)imu->gzDps10 * 0.1f;
    outMotion->rollDeg = (float)imu->rollCdeg * 0.01f;
    outMotion->pitchDeg = (float)imu->pitchCdeg * 0.01f;
    outMotion->yawDeg = (float)imu->yawCdeg * 0.01f;
    outMotion->accelNormG = (float)imu->accelNormMg * 0.001f;
    outMotion->lateralG = (float)imu->lateralMg * 0.001f;
    outMotion->tempC = (float)imu->tempC10 * 0.1f;
    return TRUE;
}

float TeensyLink_EstimateSlipG(const TeensyLinkImuMotion_t *motion, float speedMps)
{
    float yawRateRadS;
    float expectedLatMps2;
    float measuredLatMps2;

    if (motion == NULL_PTR)
    {
        return 0.0f;
    }

    yawRateRadS = motion->gzDps * TEENSY_LINK_DEG_TO_RAD_F;
    expectedLatMps2 = speedMps * yawRateRadS;
    measuredLatMps2 = motion->lateralG * TEENSY_LINK_GRAVITY_MPS2;

    return (measuredLatMps2 - expectedLatMps2) / TEENSY_LINK_GRAVITY_MPS2;
}
