#include "logging/teensy_sd_logger.h"

static constexpr uint32_t SD_SYNC_INTERVAL_MS = 2000UL;
static constexpr size_t SD_WRITE_CHUNK_BYTES = 512U;
/* Current rows are about 470 bytes with pessimistic field widths. Keep a
   larger guard so a single unexpected value width does not create a partial
   CSV row in the ring buffer. */
static constexpr size_t SD_MAX_ROW_BYTES = 768U;
static constexpr uint64_t SD_PREALLOCATE_BYTES = 64ULL * 1024ULL * 1024ULL;

bool TeensySdLogger::begin()
{
    ready_ = false;
    error_ = false;
    dropCount_ = 0U;
    logSeq_ = 0U;
    bytesWritten_ = 0U;
    fileName_[0] = '\0';

    if (!sd_.begin(SdioConfig(FIFO_SDIO)))
    {
        return false;
    }

    if (!openNextFile())
    {
        error_ = true;
        return false;
    }

    (void)file_.preAllocate(SD_PREALLOCATE_BYTES);
    ringBuf_.begin(&file_);
    writeHeader();
    if (error_ == true)
    {
        (void)file_.close();
        return false;
    }

    nextSyncMs_ = millis() + SD_SYNC_INTERVAL_MS;
    ready_ = true;
    return true;
}

bool TeensySdLogger::openNextFile()
{
    for (uint16_t i = 0U; i < 1000U; i++)
    {
        (void)snprintf(fileName_, sizeof(fileName_), "LOG%03u.CSV", i);
        if (!sd_.exists(fileName_))
        {
            return file_.open(fileName_, O_WRONLY | O_CREAT | O_EXCL);
        }
    }
    return false;
}

void TeensySdLogger::writeHeader()
{
    file_.print(F("time_ms,log_seq,tx_seq,sensor_seq,sensor_dt_us,sensor_age_ms,"
                  "teensy_status_flags,teensy_component_mask,"
                  "s32k_valid,s32k_seq,rx_frames,rx_errors,timeouts,"
                  "s32k_app_mode,s32k_app_state,s32k_control_seq,s32k_control_dt_us,"
                  "s32k_safety_flags,"
                  "ax_g,ay_g,az_g,gx_dps,gy_dps,gz_dps,"
                  "roll_deg,pitch_deg,yaw_deg,accel_norm_g,lateral_g,imu_temp_c,"
                  "cam0_err_pct,cam0_status,cam0_feature,cam0_conf,cam0_left,"
                  "cam0_right,cam0_age_ms,cam0_flags,cam0_seq,"
                  "cam1_err_pct,cam1_status,cam1_feature,cam1_conf,cam1_left,"
                  "cam1_right,cam1_age_ms,cam1_flags,cam1_seq,"
                  "ultra_dist_cm10,ultra_age_ms,ultra_flags,"
                  "steer_raw,steer_filt,steer_out,target_speed_pct,current_speed_pct,"
                  "esc_primary,esc_secondary,servo_cmd,actuator_flags,"
                  "logger_flags,logger_drops\n"));
    if (!file_.sync())
    {
        error_ = true;
    }
}

static void printCamera(Print &out, const TeensyLinkCameraResult &camera)
{
    out.print(camera.errorPct);
    out.print(',');
    out.print(camera.status);
    out.print(',');
    out.print(camera.feature);
    out.print(',');
    out.print(camera.confidence);
    out.print(',');
    out.print(camera.leftLineIdx);
    out.print(',');
    out.print(camera.rightLineIdx);
    out.print(',');
    out.print(camera.ageMs);
    out.print(',');
    out.print(camera.flags);
    out.print(',');
    out.print(camera.sequence);
}

void TeensySdLogger::logRow(uint32_t nowMs, uint16_t txSeq,
                            const TeensyLinkTelemetryInputs &telemetry,
                            const TeensyLinkS32kSnapshot &s32k, uint16_t rxFrames,
                            uint16_t rxErrors, uint16_t timeouts)
{
    if ((ready_ != true) || (error_ == true))
    {
        if (error_ == true)
        {
            dropCount_++;
        }
        return;
    }

    logSeq_++;
    if (ringBuf_.bytesFree() < SD_MAX_ROW_BYTES)
    {
        dropCount_++;
        return;
    }

    RingBuf<FsFile, 32768> &out = ringBuf_;
    out.print(nowMs);
    out.print(',');
    out.print(logSeq_);
    out.print(',');
    out.print(txSeq);
    out.print(',');
    out.print(telemetry.sensorSeq);
    out.print(',');
    out.print(telemetry.sensorDtUs);
    out.print(',');
    out.print(telemetry.sensorAgeMs);
    out.print(',');
    out.print(telemetry.statusFlags);
    out.print(',');
    out.print(telemetry.componentMask);
    out.print(',');

    out.print(s32k.valid ? 1 : 0);
    out.print(',');
    out.print(s32k.valid ? s32k.frameSeq : 0U);
    out.print(',');
    out.print(rxFrames);
    out.print(',');
    out.print(rxErrors);
    out.print(',');
    out.print(timeouts);
    out.print(',');
    out.print(s32k.valid ? s32k.appMode : 0U);
    out.print(',');
    out.print(s32k.valid ? s32k.appState : 0U);
    out.print(',');
    out.print(s32k.valid ? s32k.controlLoopSeq : 0U);
    out.print(',');
    out.print(s32k.valid ? s32k.controlDtUs : 0U);
    out.print(',');
    out.print(s32k.valid ? s32k.safetyFlags : 0U);
    out.print(',');

    out.print(telemetry.imu.axG, 4);
    out.print(',');
    out.print(telemetry.imu.ayG, 4);
    out.print(',');
    out.print(telemetry.imu.azG, 4);
    out.print(',');
    out.print(telemetry.imu.gxDps, 2);
    out.print(',');
    out.print(telemetry.imu.gyDps, 2);
    out.print(',');
    out.print(telemetry.imu.gzDps, 2);
    out.print(',');
    out.print(telemetry.imu.rollDeg, 2);
    out.print(',');
    out.print(telemetry.imu.pitchDeg, 2);
    out.print(',');
    out.print(telemetry.imu.yawDeg, 2);
    out.print(',');
    out.print(telemetry.imu.accelNormG, 4);
    out.print(',');
    out.print(telemetry.imu.lateralG, 4);
    out.print(',');
    out.print(telemetry.imu.tempC, 1);
    out.print(',');

    printCamera(out, telemetry.camera[0]);
    out.print(',');
    printCamera(out, telemetry.camera[1]);
    out.print(',');

    out.print(s32k.valid ? s32k.ultrasonicDistanceCm10 : 0U);
    out.print(',');
    out.print(s32k.valid ? s32k.ultrasonicAgeMs : 0U);
    out.print(',');
    out.print(s32k.valid ? s32k.ultrasonicFlags : 0U);
    out.print(',');
    out.print(s32k.valid ? s32k.steerRaw : 0);
    out.print(',');
    out.print(s32k.valid ? s32k.steerFilt : 0);
    out.print(',');
    out.print(s32k.valid ? s32k.steerOut : 0);
    out.print(',');
    out.print(s32k.valid ? s32k.targetSpeedPct : 0);
    out.print(',');
    out.print(s32k.valid ? s32k.currentSpeedPct : 0);
    out.print(',');
    out.print(s32k.valid ? s32k.escPrimaryLogical : 0);
    out.print(',');
    out.print(s32k.valid ? s32k.escSecondaryLogical : 0);
    out.print(',');
    out.print(s32k.valid ? s32k.servoCmd : 0);
    out.print(',');
    out.print(s32k.valid ? s32k.actuatorFlags : 0U);
    out.print(',');
    out.print(linkLoggerFlags());
    out.print(',');
    out.print(dropCount_);
    out.print('\n');

    if (out.getWriteError())
    {
        out.clearWriteError();
        dropCount_++;
    }
}

bool TeensySdLogger::serviceDue(uint32_t nowMs)
{
    if ((ready_ != true) || (error_ == true) || file_.isBusy())
    {
        return false;
    }

    return (ringBuf_.bytesUsed() >= SD_WRITE_CHUNK_BYTES) || ((int32_t)(nowMs - nextSyncMs_) >= 0);
}

void TeensySdLogger::service(uint32_t nowMs)
{
    if ((ready_ != true) || (error_ == true) || file_.isBusy())
    {
        return;
    }

    if (ringBuf_.bytesUsed() >= SD_WRITE_CHUNK_BYTES)
    {
        if (ringBuf_.writeOut(SD_WRITE_CHUNK_BYTES) != SD_WRITE_CHUNK_BYTES)
        {
            error_ = true;
            return;
        }
        bytesWritten_ += SD_WRITE_CHUNK_BYTES;
        return;
    }

    if ((int32_t)(nowMs - nextSyncMs_) >= 0)
    {
        size_t tail = ringBuf_.bytesUsed();

        nextSyncMs_ = nowMs + SD_SYNC_INTERVAL_MS;
        if (tail > 0U)
        {
            if (ringBuf_.writeOut(tail) != tail)
            {
                error_ = true;
                return;
            }
            bytesWritten_ += tail;
        }

        if (!file_.sync())
        {
            error_ = true;
        }
    }
}

void TeensySdLogger::end()
{
    if (ready_ != true)
    {
        return;
    }

    size_t tail = ringBuf_.bytesUsed();
    if (tail > 0U)
    {
        (void)ringBuf_.writeOut(tail);
    }
    (void)file_.truncate();
    (void)file_.close();
    ready_ = false;
}

bool TeensySdLogger::isReady() const
{
    return ready_;
}

bool TeensySdLogger::hasError() const
{
    return error_;
}

uint16_t TeensySdLogger::dropCount() const
{
    return dropCount_;
}

const char *TeensySdLogger::fileName() const
{
    return fileName_;
}

uint32_t TeensySdLogger::bytesWritten() const
{
    return bytesWritten_;
}

uint16_t TeensySdLogger::linkLoggerFlags() const
{
    uint16_t flags = 0U;

    if (error_ == true)
    {
        flags |= TEENSY_LINK_LOGGER_FLAG_ERROR;
    }
    else if (ready_ == true)
    {
        flags |= TEENSY_LINK_LOGGER_FLAG_READY;
        if (bytesWritten_ > 0U)
        {
            flags |= TEENSY_LINK_LOGGER_FLAG_WRITING;
        }
    }

    return flags;
}
