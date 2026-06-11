#include "logging/teensy_sd_logger.h"

/* Sync (save file size + directory entry) every 2 s. Power loss costs
   at most the last 2 s of rows. Syncing more often would hit the card
   with slow directory writes while the car is driving. */
static constexpr uint32_t SD_SYNC_INTERVAL_MS = 2000UL;

/* Move data to the card in single 512-byte sectors. One sector is the
   cheapest possible SD write, so the loop() pause stays tiny. */
static constexpr size_t SD_WRITE_CHUNK = 512U;

/* Worst-case formatted CSV row, with margin. Used to decide "row fits
   in the ring buffer or gets dropped". */
static constexpr size_t SD_MAX_ROW_BYTES = 512U;

/* Reserve the file space up front so writing never has to stop and
   allocate clusters mid-run. 64 MiB is hours of CSV at 100 Hz. */
static constexpr uint64_t SD_PREALLOCATE_BYTES = 64ULL * 1024ULL * 1024ULL;

bool TeensySdLogger::begin()
{
    ready_ = false;
    error_ = false;
    dropCount_ = 0;
    logSeq_ = 0;
    bytesWritten_ = 0;

    if (!sd_.begin(SdioConfig(FIFO_SDIO)))
    {
        /* No card (or card not readable). Stay inactive, keep driving. */
        return false;
    }

    if (!openNextFile())
    {
        error_ = true;
        return false;
    }

    /* Best effort: a fragmented card can refuse this and logging still
       works, just with occasional slower writes. */
    (void)file_.preAllocate(SD_PREALLOCATE_BYTES);

    ringBuf_.begin(&file_);
    writeHeader();
    if (error_)
    {
        /* Header did not reach the card (write-protected or failing
           card). Treat it like a missing card: stay inactive. */
        (void)file_.close();
        return false;
    }

    nextSyncMs_ = millis() + SD_SYNC_INTERVAL_MS;
    ready_ = true;
    return true;
}

bool TeensySdLogger::openNextFile()
{
    for (uint16_t i = 0; i < 1000U; i++)
    {
        snprintf(fileName_, sizeof(fileName_), "LOG%03u.CSV", i);
        if (!sd_.exists(fileName_))
        {
            return file_.open(fileName_, O_WRONLY | O_CREAT | O_EXCL);
        }
    }
    return false;
}

void TeensySdLogger::writeHeader()
{
    /* Header goes straight to the file. begin() runs inside setup(),
       before the SPI link matters, so blocking here is fine. */
    file_.print(F(
        "time_ms,log_seq,tx_seq,sensor_seq,sensor_dt_us,sensor_age_ms,"
        "s32k_valid,s32k_seq,rx_frames,rx_errors,timeouts,"
        "s32k_app_mode,s32k_app_state,s32k_control_seq,s32k_control_dt_us,s32k_safety_flags,"
        "ax_g,ay_g,az_g,gx_dps,gy_dps,gz_dps,"
        "roll_deg,pitch_deg,yaw_deg,accel_norm_g,lateral_g,imu_temp_c,"
        "cam0_err_pct,cam0_status,cam0_conf,cam0_left,cam0_right,cam0_age_ms,cam0_flags,"
        "cam1_err_pct,cam1_status,cam1_conf,cam1_left,cam1_right,cam1_age_ms,cam1_flags,"
        "ultra_dist_cm10,ultra_age_ms,ultra_flags,"
        "steer_raw,steer_filt,steer_out,target_speed_pct,current_speed_pct,"
        "esc_primary,esc_secondary,servo_cmd,actuator_flags,"
        "logger_flags,logger_drops\n"));
    if (!file_.sync())
    {
        error_ = true;
    }
}

static void printCamera(Print &out, const TeensyLinkCameraResult &cam)
{
    out.print(cam.errorPct);
    out.print(',');
    out.print(cam.status);
    out.print(',');
    out.print(cam.confidence);
    out.print(',');
    out.print(cam.leftLineIdx);
    out.print(',');
    out.print(cam.rightLineIdx);
    out.print(',');
    out.print(cam.ageMs);
    out.print(',');
    out.print(cam.flags);
}

void TeensySdLogger::logRow(uint32_t nowMs,
                            uint16_t txSeq,
                            const TeensyLinkTelemetryInputs &telemetry,
                            const TeensyLinkS32kSnapshot &s32k,
                            uint16_t rxFrames,
                            uint16_t rxErrors,
                            uint16_t timeouts)
{
    if (!ready_ || error_)
    {
        /* A broken card counts its lost rows. A missing card was never
           logging, so nothing is "lost". */
        if (error_)
        {
            dropCount_++;
        }
        return;
    }

    /* Every row gets a sequence number even if it is dropped below,
       so gaps in the CSV log_seq column mark exactly where rows were
       lost. */
    logSeq_++;

    if (ringBuf_.bytesFree() < SD_MAX_ROW_BYTES)
    {
        /* SD is falling behind. Drop this row instead of stalling the
           loop (and with it the bit-banged SPI slave). */
        dropCount_++;
        return;
    }

    RingBuf<FsFile, 32768> &rb = ringBuf_;
    rb.print(nowMs);
    rb.print(',');
    rb.print(logSeq_);
    rb.print(',');
    rb.print(txSeq);
    rb.print(',');
    rb.print(telemetry.sensorSeq);
    rb.print(',');
    rb.print(telemetry.sensorDtUs);
    rb.print(',');
    rb.print(telemetry.sensorAgeMs);
    rb.print(',');

    /* s32k_valid tells MATLAB whether the S32K columns mean anything.
       When 0 (no valid frame yet), they are all logged as 0. */
    rb.print(s32k.valid ? 1 : 0);
    rb.print(',');
    rb.print(s32k.valid ? s32k.frameSeq : 0);
    rb.print(',');
    rb.print(rxFrames);
    rb.print(',');
    rb.print(rxErrors);
    rb.print(',');
    rb.print(timeouts);
    rb.print(',');
    rb.print(s32k.valid ? s32k.appMode : 0);
    rb.print(',');
    rb.print(s32k.valid ? s32k.appState : 0);
    rb.print(',');
    rb.print(s32k.valid ? s32k.controlLoopSeq : 0);
    rb.print(',');
    rb.print(s32k.valid ? s32k.controlDtUs : 0);
    rb.print(',');
    rb.print(s32k.valid ? s32k.safetyFlags : 0);
    rb.print(',');

    rb.print(telemetry.imu.axG, 4);
    rb.print(',');
    rb.print(telemetry.imu.ayG, 4);
    rb.print(',');
    rb.print(telemetry.imu.azG, 4);
    rb.print(',');
    rb.print(telemetry.imu.gxDps, 2);
    rb.print(',');
    rb.print(telemetry.imu.gyDps, 2);
    rb.print(',');
    rb.print(telemetry.imu.gzDps, 2);
    rb.print(',');
    rb.print(telemetry.imu.rollDeg, 2);
    rb.print(',');
    rb.print(telemetry.imu.pitchDeg, 2);
    rb.print(',');
    rb.print(telemetry.imu.yawDeg, 2);
    rb.print(',');
    rb.print(telemetry.imu.accelNormG, 4);
    rb.print(',');
    rb.print(telemetry.imu.lateralG, 4);
    rb.print(',');
    rb.print(telemetry.imu.tempC, 1);
    rb.print(',');

    printCamera(rb, telemetry.camera[0]);
    rb.print(',');
    printCamera(rb, telemetry.camera[1]);
    rb.print(',');

    rb.print(s32k.valid ? s32k.ultrasonicDistanceCm10 : 0);
    rb.print(',');
    rb.print(s32k.valid ? s32k.ultrasonicAgeMs : 0);
    rb.print(',');
    rb.print(s32k.valid ? s32k.ultrasonicFlags : 0);
    rb.print(',');
    rb.print(s32k.valid ? s32k.steerRaw : 0);
    rb.print(',');
    rb.print(s32k.valid ? s32k.steerFilt : 0);
    rb.print(',');
    rb.print(s32k.valid ? s32k.steerOut : 0);
    rb.print(',');
    rb.print(s32k.valid ? s32k.targetSpeedPct : 0);
    rb.print(',');
    rb.print(s32k.valid ? s32k.currentSpeedPct : 0);
    rb.print(',');
    rb.print(s32k.valid ? s32k.escPrimaryLogical : 0);
    rb.print(',');
    rb.print(s32k.valid ? s32k.escSecondaryLogical : 0);
    rb.print(',');
    rb.print(s32k.valid ? s32k.servoCmd : 0);
    rb.print(',');
    rb.print(s32k.valid ? s32k.actuatorFlags : 0);
    rb.print(',');

    rb.print(linkLoggerFlags());
    rb.print(',');
    rb.print(dropCount_);
    rb.print('\n');

    if (rb.getWriteError())
    {
        /* Should not happen because of the bytesFree() guard, but if a
           row ever overflows, count it and clear the error. */
        rb.clearWriteError();
        dropCount_++;
    }
}

void TeensySdLogger::service(uint32_t nowMs)
{
    if (!ready_ || error_)
    {
        return;
    }

    /* Card still chewing on the last write: come back next loop. */
    if (file_.isBusy())
    {
        return;
    }

    if (ringBuf_.bytesUsed() >= SD_WRITE_CHUNK)
    {
        if (ringBuf_.writeOut(SD_WRITE_CHUNK) != SD_WRITE_CHUNK)
        {
            error_ = true;
            return;
        }
        bytesWritten_ += SD_WRITE_CHUNK;
        return;
    }

    if ((int32_t)(nowMs - nextSyncMs_) >= 0)
    {
        nextSyncMs_ = nowMs + SD_SYNC_INTERVAL_MS;

        /* Push the partial tail out, then save the file size so a
           power cut keeps everything up to this moment. */
        size_t tail = ringBuf_.bytesUsed();
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
    if (!ready_)
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

uint16_t TeensySdLogger::linkLoggerFlags() const
{
    uint16_t flags = 0U;

    if (error_)
    {
        flags |= TEENSY_LINK_LOGGER_FLAG_ERROR;
    }
    else if (ready_)
    {
        flags |= TEENSY_LINK_LOGGER_FLAG_READY;
        if (bytesWritten_ > 0U)
        {
            flags |= TEENSY_LINK_LOGGER_FLAG_WRITING;
        }
    }

    return flags;
}
