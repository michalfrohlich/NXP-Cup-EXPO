#include "../app_internal.h"

#define RACE_TEENSY_CAMERA_SAMPLE_COUNT (4U)
#define RACE_TEENSY_SPI_SLOT_COUNT (4U)
#define RACE_TEENSY_SPI_SLOT_PERIOD_MS (5U)
#define RACE_TEENSY_CONTROL_PHASE_MS (17U)
#define RACE_TEENSY_CONTROL_DEADLINE_MS (19U)
#define RACE_TEENSY_CAMERA_OUTLIER_THRESHOLD (0.20f)

typedef struct
{
    VisionOutput_t vision;
    uint32 rxMs;
    uint16 sequence;
    uint8 sourceAgeMs;
} RaceTeensyCameraSample_t;

typedef struct
{
    uint32 lastValidCameraMs;
    uint32 pwmPeriodSequence;
    uint32 spiSlotMissCount;
    uint32 controlDeadlineMissCount;
    uint32 cameraReceivedCount;
    uint32 cameraAcceptedCount;
    uint32 cameraDuplicateCount;
    uint32 cameraRejectedCount;
    uint32 lastCollectedRxMs;
    uint16 controlSeq;
    uint16 lastCameraSequence;
    uint8 nextSpiSlot;
    uint8 sampleCount;
    uint8 lastSamplesUsed;
    uint8 lastMaxSampleAgeMs;
    float lastMedianError;
    float lastAverageError;
    TeensyLinkSnapshot_t snapshot;
    RaceTeensyCameraSample_t samples[RACE_TEENSY_CAMERA_SAMPLE_COUNT];
    boolean haveCameraSequence;
    boolean haveCollectedRx;
    boolean outlierRejected;
    boolean controlDue;
    boolean controlExecuted;
    boolean hardFault;
} RaceTeensyCam0LinkState_t;

static RaceTeensyCam0LinkState_t g_raceTeensyCam0Link;
static boolean g_raceUsesTeensyCam0;
static boolean g_raceTuneDriveMode;

static void race_mode_apply_runtime_tune(RaceModeState_t *st)
{
    if (st == NULL_PTR)
    {
        return;
    }

    SteeringController_SetTunings(&st->ctrl, g_runtimeTune.profile.kp, g_runtimeTune.profile.kd,
                                  1.0f);
    st->ctrl.ki = g_runtimeTune.profile.ki;
    st->ctrl.iTermClamp = g_runtimeTune.profile.iTermClamp;
}

static void race_mode_enter(uint32 nowMs, boolean useTeensyCam0, boolean tuneDriveMode)
{
    (void)memset(&g_raceMode, 0, sizeof(g_raceMode));
    (void)memset(&g_raceTeensyCam0Link, 0, sizeof(g_raceTeensyCam0Link));
    g_raceUsesTeensyCam0 = useTeensyCam0;
    g_raceTuneDriveMode = tuneDriveMode;

    RuntimeTune_InitDefaults();

    /* Runtime loop order: vision -> ultrasonic -> control. */
    g_raceMode.phase = RACE_PHASE_ESC_ARM;
    g_raceMode.nextControlMs = nowMs;
    g_raceMode.nextSpeedRampMs = nowMs;
    g_raceMode.nextUltraTrigMs = nowMs;
    g_raceMode.nextDisplayMs = nowMs;
    g_raceMode.armDoneMs = nowMs + ESC_ARM_TIME_MS;
    g_raceMode.lastDistanceMs = nowMs;

    Esc_Init(ESC_PWM_CH, ESC_SECOND_PWM_CH, ESC_DUTY_MIN, ESC_DUTY_MED, ESC_DUTY_MAX);
    Esc_StopNeutral();

    Servo_Init(SERVO_PWM_CH, SERVO_DUTY_MAX, SERVO_DUTY_MIN, SERVO_DUTY_MED);
    if (useTeensyCam0 == TRUE)
    {
        Servo_SetUpdatePolicy(SERVO_UPDATE_PHASED_FOREGROUND);
    }
    SteerStraight();

    if (useTeensyCam0 == TRUE)
    {
        TeensyLink_Init();
        g_raceTeensyCam0Link.lastValidCameraMs = nowMs;
    }
    else
    {
        if (LinearCameraIsBusy() == TRUE)
        {
            LinearCameraStopStream();
        }

        LinearCameraInit(CAM_CLK_PWM_CH, CAM_SHUTTER_GPT_CH, CAM_ADC_GROUP, CAM_SHUTTER_PCR);
        LinearCameraSetFrameIntervalTicks(CAM_FRAME_INTERVAL_TICKS);
        LineDetector_Init();
    }

    SteeringController_Init(&g_raceMode.ctrl);
    SteeringController_Reset(&g_raceMode.ctrl);
    if (g_raceTuneDriveMode == TRUE)
    {
        race_mode_apply_runtime_tune(&g_raceMode);
    }

    Ultrasonic_Init();

    if (useTeensyCam0 != TRUE)
    {
        (void)LinearCameraStartStream();
    }
    StatusLed_Blue();
}

static void race_mode_update_vision(RaceModeState_t *st, uint32 nowMs)
{
    const LinearCameraFrame *latestFrame = NULL_PTR;

    (void)nowMs;

    if (st == NULL_PTR)
    {
        return;
    }

    if (LinearCameraGetLatestFrame(&latestFrame) != TRUE)
    {
        return;
    }

    (void)memcpy(st->processedFrame.Values, &latestFrame->Values[CAM_TRIM_LEFT_PX],
                 ((size_t)VISION_LINEAR_BUFFER_SIZE * sizeof(st->processedFrame.Values[0])));
    LineDetector_Process(st->processedFrame.Values, &st->result);
    st->haveValidVision = TRUE;
}

static void race_mode_default_link_camera(TeensyLinkCameraResult_t *camera)
{
    if (camera == NULL_PTR)
    {
        return;
    }

    camera->errorPct = 0;
    camera->status = (uint8)VISION_TRACK_LOST;
    camera->feature = (uint8)VISION_FEATURE_NONE;
    camera->confidence = 0U;
    camera->leftLineIdx = (uint8)VISION_LINEAR_INVALID_IDX;
    camera->rightLineIdx = (uint8)VISION_LINEAR_INVALID_IDX;
    camera->ageMs = 255U;
    camera->flags = (uint8)(TEENSY_LINK_CAMERA_FLAG_SOURCE_S32K | TEENSY_LINK_CAMERA_FLAG_STALE);
    camera->sequence = 0U;
}

static void race_mode_fill_teensy_link_input(const RaceModeState_t *st,
                                             TeensyLinkS32kInputs_t *input)
{
    ServoDebugSnapshot servoSnapshot;

    if ((st == NULL_PTR) || (input == NULL_PTR))
    {
        return;
    }

    (void)memset(input, 0, sizeof(*input));
    input->controlLoopSeq = g_raceTeensyCam0Link.controlSeq;
    input->controlDtUs = 20000U;
    input->appMode = (uint8)APP_BUILD_MODE_TEENSY_CAM0_RACE;
    input->appState = (uint8)st->phase;
    input->safetyFlags = (g_raceTeensyCam0Link.hardFault == TRUE) ? 1U : 0U;
    if (g_raceTeensyCam0Link.spiSlotMissCount != 0U)
    {
        input->diagnosticFlags |= (uint16)TEENSY_LINK_S32K_DIAG_SPI_SLOT_MISS;
    }
    if (g_raceTeensyCam0Link.controlDeadlineMissCount != 0U)
    {
        input->diagnosticFlags |= (uint16)TEENSY_LINK_S32K_DIAG_CONTROL_DEADLINE_MISS;
    }
    if (g_raceTeensyCam0Link.lastSamplesUsed == 0U)
    {
        input->diagnosticFlags |= (uint16)TEENSY_LINK_S32K_DIAG_VISION_NO_SAMPLE;
    }
    if (g_raceTeensyCam0Link.outlierRejected == TRUE)
    {
        input->diagnosticFlags |= (uint16)TEENSY_LINK_S32K_DIAG_VISION_OUTLIER;
    }
    Servo_GetDebugSnapshot(&servoSnapshot);
    if (servoSnapshot.MissedCommitCount != 0U)
    {
        input->diagnosticFlags |= (uint16)TEENSY_LINK_S32K_DIAG_SERVO_COMMIT_MISS;
    }
    race_mode_default_link_camera(&input->camera[0]);
    race_mode_default_link_camera(&input->camera[1]);
    input->steerRaw = st->steerRaw;
    input->steerFilt = st->steerFilt;
    input->steerOut = st->steerOut;
    input->targetSpeedPct = (sint16)st->targetSpeedPct;
    input->currentSpeedPct = (sint16)st->currentSpeedPct;
    input->escPrimaryLogical = (sint16)(-st->currentSpeedPct);
    input->escSecondaryLogical = (sint16)(-st->currentSpeedPct);
    input->servoCmd = st->steerOut;

    if (st->haveValidDistance == TRUE)
    {
        float distanceCm10 = st->lastDistanceCm * 10.0f;
        if (distanceCm10 > 65535.0f)
        {
            distanceCm10 = 65535.0f;
        }
        input->ultrasonicDistanceCm10 = (uint16)distanceCm10;
        input->ultrasonicFlags = 1U;
    }
}

static float race_mode_abs_f(float value)
{
    return (value < 0.0f) ? -value : value;
}

static void race_mode_sort_errors(float *errors, uint8 count)
{
    uint8 i;

    for (i = 1U; i < count; i++)
    {
        float value = errors[i];
        uint8 j = i;

        while ((j > 0U) && (errors[j - 1U] > value))
        {
            errors[j] = errors[j - 1U];
            j--;
        }
        errors[j] = value;
    }
}

static uint8 race_mode_sample_age_ms(const RaceTeensyCameraSample_t *sample, uint32 nowMs)
{
    uint32 ageMs;

    if (sample == NULL_PTR)
    {
        return 255U;
    }

    ageMs = (uint32)sample->sourceAgeMs + (uint32)(nowMs - sample->rxMs);
    return (ageMs > 255U) ? 255U : (uint8)ageMs;
}

static boolean race_mode_average_teensy_camera(RaceModeState_t *st, uint32 nowMs)
{
    float errors[RACE_TEENSY_CAMERA_SAMPLE_COUNT];
    boolean usable[RACE_TEENSY_CAMERA_SAMPLE_COUNT] = {FALSE, FALSE, FALSE, FALSE};
    uint8 usableIndices[RACE_TEENSY_CAMERA_SAMPLE_COUNT];
    uint8 usableCount = 0U;
    uint8 rejectIndex = 255U;
    uint8 i;
    float median;
    float weightedError = 0.0f;
    float acceptedErrors[RACE_TEENSY_CAMERA_SAMPLE_COUNT];
    float acceptedMedian;
    uint32 weightSum = 0U;
    uint32 confidenceSum = 0U;
    uint8 acceptedCount = 0U;
    uint8 newestIndex = 0U;
    uint8 maxAgeMs = 0U;

    if (st == NULL_PTR)
    {
        return FALSE;
    }

    for (i = 0U; i < g_raceTeensyCam0Link.sampleCount; i++)
    {
        uint8 ageMs = race_mode_sample_age_ms(&g_raceTeensyCam0Link.samples[i], nowMs);
        const VisionOutput_t *vision = &g_raceTeensyCam0Link.samples[i].vision;

        if ((ageMs <= (uint8)TEENSY_CAM0_CONTROL_MAX_AGE_MS) &&
            (vision->status != VISION_TRACK_LOST))
        {
            usable[i] = TRUE;
            usableIndices[usableCount] = i;
            errors[usableCount] = vision->error;
            usableCount++;
        }
        else
        {
            g_raceTeensyCam0Link.cameraRejectedCount++;
        }
    }

    if (usableCount == 0U)
    {
        st->haveValidVision = FALSE;
        st->result.status = VISION_TRACK_LOST;
        st->result.feature = VISION_FEATURE_NONE;
        st->result.confidence = 0U;
        st->result.error = 0.0f;
        st->result.leftLineIdx = (uint8)VISION_LINEAR_INVALID_IDX;
        st->result.rightLineIdx = (uint8)VISION_LINEAR_INVALID_IDX;
        g_raceTeensyCam0Link.lastSamplesUsed = 0U;
        g_raceTeensyCam0Link.lastMaxSampleAgeMs = 0U;
        g_raceTeensyCam0Link.lastMedianError = 0.0f;
        g_raceTeensyCam0Link.lastAverageError = 0.0f;
        g_raceTeensyCam0Link.outlierRejected = FALSE;
        return FALSE;
    }

    race_mode_sort_errors(errors, usableCount);
    if ((usableCount & 1U) != 0U)
    {
        median = errors[usableCount / 2U];
    }
    else
    {
        median = (errors[(usableCount / 2U) - 1U] + errors[usableCount / 2U]) * 0.5f;
    }

    if (usableCount >= 3U)
    {
        float largestDeviation = 0.0f;

        for (i = 0U; i < usableCount; i++)
        {
            uint8 sampleIndex = usableIndices[i];
            float deviation =
                race_mode_abs_f(g_raceTeensyCam0Link.samples[sampleIndex].vision.error - median);

            if (deviation > largestDeviation)
            {
                largestDeviation = deviation;
                rejectIndex = sampleIndex;
            }
        }

        if (largestDeviation <= RACE_TEENSY_CAMERA_OUTLIER_THRESHOLD)
        {
            rejectIndex = 255U;
        }
        else
        {
            g_raceTeensyCam0Link.cameraRejectedCount++;
        }
    }

    g_raceTeensyCam0Link.outlierRejected = (rejectIndex != 255U) ? TRUE : FALSE;
    maxAgeMs = 0U;
    for (i = 0U; i < g_raceTeensyCam0Link.sampleCount; i++)
    {
        const VisionOutput_t *vision;
        uint32 weight;
        uint8 ageMs;

        if ((usable[i] != TRUE) || (i == rejectIndex))
        {
            continue;
        }

        vision = &g_raceTeensyCam0Link.samples[i].vision;
        weight = (uint32)vision->confidence;
        weightedError += vision->error * (float)weight;
        acceptedErrors[acceptedCount] = vision->error;
        weightSum += weight;
        confidenceSum += (uint32)vision->confidence;
        acceptedCount++;
        newestIndex = i;
        ageMs = race_mode_sample_age_ms(&g_raceTeensyCam0Link.samples[i], nowMs);
        if (ageMs > maxAgeMs)
        {
            maxAgeMs = ageMs;
        }
    }

    if (acceptedCount == 0U)
    {
        return FALSE;
    }

    race_mode_sort_errors(acceptedErrors, acceptedCount);
    if ((acceptedCount & 1U) != 0U)
    {
        acceptedMedian = acceptedErrors[acceptedCount / 2U];
    }
    else
    {
        acceptedMedian =
            (acceptedErrors[(acceptedCount / 2U) - 1U] + acceptedErrors[acceptedCount / 2U]) * 0.5f;
    }

    st->result = g_raceTeensyCam0Link.samples[newestIndex].vision;
    st->result.error = (weightSum != 0U) ? (weightedError / (float)weightSum) : acceptedMedian;
    st->result.confidence = (uint8)(confidenceSum / (uint32)acceptedCount);
    st->haveValidVision = TRUE;

    g_raceTeensyCam0Link.lastSamplesUsed = acceptedCount;
    g_raceTeensyCam0Link.lastMaxSampleAgeMs = maxAgeMs;
    g_raceTeensyCam0Link.lastMedianError = median;
    g_raceTeensyCam0Link.lastAverageError = st->result.error;
    return TRUE;
}

static void race_mode_collect_teensy_camera(RaceModeState_t *st, uint32 nowMs)
{
    VisionOutput_t vision;
    uint16 cameraSequence;

    if (st == NULL_PTR)
    {
        return;
    }

    if (TeensyLink_GetSnapshot(&g_raceTeensyCam0Link.snapshot) != TRUE)
    {
        return;
    }

    if ((g_raceTeensyCam0Link.haveCollectedRx == TRUE) &&
        (g_raceTeensyCam0Link.snapshot.lastRxMs == g_raceTeensyCam0Link.lastCollectedRxMs))
    {
        return;
    }

    g_raceTeensyCam0Link.haveCollectedRx = TRUE;
    g_raceTeensyCam0Link.lastCollectedRxMs = g_raceTeensyCam0Link.snapshot.lastRxMs;
    g_raceTeensyCam0Link.cameraReceivedCount++;

    if (TeensyCameraSource_GetCamera0Vision(&g_raceTeensyCam0Link.snapshot, nowMs,
                                            (uint32)TEENSY_CAM0_CONTROL_MAX_AGE_MS,
                                            &vision) != TRUE)
    {
        g_raceTeensyCam0Link.cameraRejectedCount++;
        return;
    }

    cameraSequence = g_raceTeensyCam0Link.snapshot.camera[0].sequence;
    if ((g_raceTeensyCam0Link.haveCameraSequence == TRUE) &&
        (cameraSequence == g_raceTeensyCam0Link.lastCameraSequence))
    {
        g_raceTeensyCam0Link.cameraDuplicateCount++;
        return;
    }

    g_raceTeensyCam0Link.haveCameraSequence = TRUE;
    g_raceTeensyCam0Link.lastCameraSequence = cameraSequence;
    g_raceTeensyCam0Link.lastValidCameraMs = nowMs;
    g_raceTeensyCam0Link.cameraAcceptedCount++;

    if (g_raceTeensyCam0Link.sampleCount < RACE_TEENSY_CAMERA_SAMPLE_COUNT)
    {
        RaceTeensyCameraSample_t *sample =
            &g_raceTeensyCam0Link.samples[g_raceTeensyCam0Link.sampleCount];

        sample->vision = vision;
        sample->rxMs = nowMs;
        sample->sequence = cameraSequence;
        sample->sourceAgeMs = g_raceTeensyCam0Link.snapshot.camera[0].ageMs;
        g_raceTeensyCam0Link.sampleCount++;
    }
}

static void race_mode_update_teensy_cam0(RaceModeState_t *st, uint32 nowMs)
{
    TeensyLinkS32kInputs_t input;
    ServoDebugSnapshot servoSnapshot;
    uint32 phaseMs;

    if (st == NULL_PTR)
    {
        return;
    }

    Servo_GetDebugSnapshot(&servoSnapshot);
    if ((servoSnapshot.Initialized != TRUE) || (servoSnapshot.PeriodSequence == 0U))
    {
        return;
    }

    if (servoSnapshot.PeriodSequence != g_raceTeensyCam0Link.pwmPeriodSequence)
    {
        if ((g_raceTeensyCam0Link.pwmPeriodSequence != 0U) &&
            (g_raceTeensyCam0Link.controlExecuted != TRUE))
        {
            g_raceTeensyCam0Link.controlDeadlineMissCount++;
        }

        g_raceTeensyCam0Link.pwmPeriodSequence = servoSnapshot.PeriodSequence;
        g_raceTeensyCam0Link.nextSpiSlot = 0U;
        g_raceTeensyCam0Link.sampleCount = 0U;
        g_raceTeensyCam0Link.controlDue = FALSE;
        g_raceTeensyCam0Link.controlExecuted = FALSE;
    }

    phaseMs = (uint32)(nowMs - servoSnapshot.PeriodStartMs);

    while (g_raceTeensyCam0Link.nextSpiSlot < RACE_TEENSY_SPI_SLOT_COUNT)
    {
        uint32 slotDueMs =
            (uint32)g_raceTeensyCam0Link.nextSpiSlot * RACE_TEENSY_SPI_SLOT_PERIOD_MS;

        if (phaseMs < slotDueMs)
        {
            break;
        }

        if (phaseMs >= (slotDueMs + RACE_TEENSY_SPI_SLOT_PERIOD_MS))
        {
            g_raceTeensyCam0Link.spiSlotMissCount++;
            g_raceTeensyCam0Link.nextSpiSlot++;
            continue;
        }

        race_mode_fill_teensy_link_input(st, &input);
        (void)TeensyLink_Service(nowMs, &input);
        race_mode_collect_teensy_camera(st, nowMs);
        g_raceTeensyCam0Link.nextSpiSlot++;
        break;
    }

    nowMs = Timebase_GetMs();
    phaseMs = (uint32)(nowMs - servoSnapshot.PeriodStartMs);
    if ((g_raceTeensyCam0Link.controlExecuted != TRUE) &&
        (g_raceTeensyCam0Link.controlDue != TRUE) && (phaseMs >= RACE_TEENSY_CONTROL_PHASE_MS) &&
        (phaseMs < RACE_TEENSY_CONTROL_DEADLINE_MS))
    {
        (void)race_mode_average_teensy_camera(st, nowMs);
        g_raceTeensyCam0Link.controlDue = TRUE;
    }
    else if ((g_raceTeensyCam0Link.controlExecuted != TRUE) &&
             (phaseMs >= RACE_TEENSY_CONTROL_DEADLINE_MS))
    {
        g_raceTeensyCam0Link.controlExecuted = TRUE;
        g_raceTeensyCam0Link.controlDeadlineMissCount++;
    }

    if ((g_raceTeensyCam0Link.hardFault != TRUE) &&
        ((uint32)(nowMs - g_raceTeensyCam0Link.lastValidCameraMs) >=
         (uint32)TEENSY_CAM0_HARD_FAULT_MS))
    {
        g_raceTeensyCam0Link.hardFault = TRUE;
        st->phase = RACE_PHASE_FAULT;
    }
}

static void race_mode_update_ultrasonic(RaceModeState_t *st, uint32 nowMs)
{
    const uint32 ultraStaleMs =
        ((uint32)HONOR_ULTRA_TRIGGER_PERIOD_MS * 2U) + (uint32)ULTRA_TIMEOUT_MS;
    float distanceCm = 0.0f;
    Ultrasonic_StatusType ultraStatus;

    if (st == NULL_PTR)
    {
        return;
    }

    /* Keep ultrasonic service independent from the main control phase. */
    Ultrasonic_Task();

    if (time_reached(nowMs, st->nextUltraTrigMs) == TRUE)
    {
        st->nextUltraTrigMs = nowMs + HONOR_ULTRA_TRIGGER_PERIOD_MS;

        if (Ultrasonic_GetStatus() != ULTRA_STATUS_BUSY)
        {
            Ultrasonic_StartMeasurement();
        }
    }

    ultraStatus = Ultrasonic_GetStatus();

    if (Ultrasonic_GetDistanceCm(&distanceCm) == TRUE)
    {
        st->lastDistanceCm = distanceCm;
        st->lastDistanceMs = nowMs;
        st->haveValidDistance = TRUE;
        st->ultraError = FALSE;
    }
    else if (ultraStatus == ULTRA_STATUS_ERROR)
    {
        st->haveValidDistance = FALSE;
        st->ultraError = TRUE;
    }
    else if ((st->haveValidDistance == TRUE) &&
             ((uint32)(nowMs - st->lastDistanceMs) > ultraStaleMs))
    {
        st->haveValidDistance = FALSE;
        st->ultraError = FALSE;
    }
    else if (ultraStatus != ULTRA_STATUS_ERROR)
    {
        st->ultraError = FALSE;
    }
}

static void race_mode_update_control(RaceModeState_t *st, uint32 nowMs, boolean stopPressed)
{
    uint8 controllerBaseSpeed;
    boolean holdStraightForObstacle = FALSE;
    float controllerDt;
    float outputFilterAlpha;
    sint16 steerRateMax;

    if (st == NULL_PTR)
    {
        return;
    }

    if (g_raceUsesTeensyCam0 == TRUE)
    {
        ServoDebugSnapshot servoSnapshot;
        uint32 phaseMs;

        if (g_raceTeensyCam0Link.controlDue != TRUE)
        {
            return;
        }

        nowMs = Timebase_GetMs();
        Servo_GetDebugSnapshot(&servoSnapshot);
        phaseMs = (uint32)(nowMs - servoSnapshot.PeriodStartMs);
        g_raceTeensyCam0Link.controlDue = FALSE;
        if ((servoSnapshot.PeriodSequence != g_raceTeensyCam0Link.pwmPeriodSequence) ||
            (phaseMs >= RACE_TEENSY_CONTROL_DEADLINE_MS))
        {
            g_raceTeensyCam0Link.controlExecuted = TRUE;
            g_raceTeensyCam0Link.controlDeadlineMissCount++;
            return;
        }

        g_raceTeensyCam0Link.controlExecuted = TRUE;
        g_raceTeensyCam0Link.controlSeq++;
        controllerDt = 0.020f;
        outputFilterAlpha =
            (g_raceTuneDriveMode == TRUE) ? g_runtimeTune.profile.steerLpfAlpha : 0.70f;
        steerRateMax =
            (g_raceTuneDriveMode == TRUE) ? g_runtimeTune.profile.steerRateMax : (sint16)16;
    }
    else
    {
        if (time_reached(nowMs, st->nextControlMs) != TRUE)
        {
            return;
        }

        st->nextControlMs += STEER_UPDATE_MS;
        if (time_reached(nowMs, st->nextControlMs) == TRUE)
        {
            st->nextControlMs = nowMs + STEER_UPDATE_MS;
        }

        controllerDt = ((float)STEER_UPDATE_MS) * 0.001f;
        outputFilterAlpha = 0.45f;
        steerRateMax = 8;
    }

    if (stopPressed == TRUE)
    {
        st->phase = RACE_PHASE_STOPPED;
    }

    if ((st->haveValidVision == TRUE) && (st->result.status != VISION_TRACK_LOST))
    {
        st->lostSinceMs = 0U;
    }
    else if (st->lostSinceMs == 0U)
    {
        st->lostSinceMs = nowMs;
    }
    else if ((st->phase == RACE_PHASE_SPEED_RUN) || (st->phase == RACE_PHASE_HONOR_RUN))
    {
        if ((uint32)(nowMs - st->lostSinceMs) >= LINE_LOST_COAST_MS)
        {
            st->phase = RACE_PHASE_FAULT;
        }
    }

    if ((st->phase == RACE_PHASE_ESC_ARM) && (time_reached(nowMs, st->armDoneMs) == TRUE))
    {
        st->phase = RACE_PHASE_SPEED_RUN;
        st->finishStreak = 0U;
        st->lostSinceMs =
            ((st->haveValidVision == TRUE) && (st->result.status != VISION_TRACK_LOST)) ? 0U
                                                                                        : nowMs;
    }

    if ((st->phase == RACE_PHASE_SPEED_RUN) && (st->haveValidVision == TRUE) &&
        (st->result.feature == VISION_FEATURE_FINISH_LINE) &&
        (st->result.confidence >= (uint8)RACE_FINISH_MIN_CONFIDENCE))
    {
        if (st->finishStreak < 255U)
        {
            st->finishStreak++;
        }

        st->phase = RACE_PHASE_HONOR_RUN;
    }
    else if (st->phase == RACE_PHASE_SPEED_RUN)
    {
        st->finishStreak = 0U;
    }

    holdStraightForObstacle =
        (boolean)((st->phase == RACE_PHASE_HONOR_RUN) && (st->haveValidDistance == TRUE) &&
                  (st->lastDistanceCm <= (float)HONOR_SLOW1_DISTANCE_CM));

    /* Steering is updated from the latest accepted vision result only inside
       the active race phases. Other phases stay neutral and centered. */
    if ((st->phase == RACE_PHASE_SPEED_RUN) || (st->phase == RACE_PHASE_HONOR_RUN))
    {
        if (holdStraightForObstacle == TRUE)
        {
            st->steerRaw = 0;
            st->steerFilt = 0;
            st->steerOut = 0;
            SteerStraight();
        }
        else if ((st->haveValidVision == TRUE) && (st->result.status != VISION_TRACK_LOST))
        {
            VehicleControlOutput_t out;

            controllerBaseSpeed =
                (st->currentSpeedPct > 0) ? (uint8)st->currentSpeedPct : (uint8)FULL_AUTO_SPEED_PCT;
            out = SteeringController_Update(&st->ctrl, &st->result, controllerDt,
                                            controllerBaseSpeed);
            st->steerRaw = (sint16)out.steer_cmd;

            if (((st->steerRaw < 0) ? (sint16)(-st->steerRaw) : st->steerRaw) <= 2)
            {
                st->steerRaw = 0;
            }

            st->steerFilt = SteeringSmooth_IirS16(st->steerFilt, st->steerRaw, outputFilterAlpha);

            {
                sint16 delta = (sint16)(st->steerFilt - st->steerOut);

                delta = SteeringSmooth_ClampS16(delta, (sint16)(-steerRateMax),
                                                (sint16)(+steerRateMax));
                st->steerOut = (sint16)(st->steerOut + delta);
            }

            {
                sint16 steerClamp = (g_raceTuneDriveMode == TRUE) ? g_runtimeTune.profile.steerClamp
                                                                  : (sint16)STEER_CMD_CLAMP;

                st->steerOut = SteeringSmooth_ClampS16(st->steerOut, (sint16)(-steerClamp),
                                                       (sint16)(+steerClamp));
            }
            Servo_SetSteer((int)st->steerOut);
        }
        else
        {
            st->steerRaw = 0;
            st->steerFilt = 0;
            st->steerOut = 0;
            SteerStraight();
        }
    }
    else
    {
        st->steerRaw = 0;
        st->steerFilt = 0;
        st->steerOut = 0;
        SteerStraight();
    }

    if (st->phase == RACE_PHASE_SPEED_RUN)
    {
        sint32 steerSlowPct =
            ((sint32)((st->steerOut < 0) ? (sint16)(-st->steerOut) : st->steerOut) *
             (sint32)SPEED_SLOW_PER_STEER) /
            (sint32)STEER_CMD_CLAMP;

        st->targetSpeedPct = (sint32)FULL_AUTO_SPEED_PCT - steerSlowPct;

        if ((st->haveValidVision == TRUE) && (st->result.status != VISION_TRACK_BOTH))
        {
            if (st->targetSpeedPct > (sint32)FULLMODE_SLOW_SPEED_PCT)
            {
                st->targetSpeedPct = (sint32)FULLMODE_SLOW_SPEED_PCT;
            }
        }

        if ((st->haveValidVision != TRUE) || (st->result.status == VISION_TRACK_LOST))
        {
            st->targetSpeedPct = (sint32)SPEED_LOST_LINE;
        }
    }
    else if (st->phase == RACE_PHASE_HONOR_RUN)
    {
        st->targetSpeedPct = honor_speed_from_distance(st->haveValidDistance, st->lastDistanceCm);

        if ((st->haveValidVision != TRUE) || (st->result.status == VISION_TRACK_LOST))
        {
            st->targetSpeedPct = 0;
        }
    }
    else
    {
        st->targetSpeedPct = 0;
    }

    if (st->targetSpeedPct < 0)
    {
        st->targetSpeedPct = 0;
    }
    if (st->targetSpeedPct > 100)
    {
        st->targetSpeedPct = 100;
    }

    /* Speed commands are rate-limited to keep ESC updates deterministic. */
    if (time_reached(nowMs, st->nextSpeedRampMs) == TRUE)
    {
        st->nextSpeedRampMs += FULL_AUTO_RAMP_PERIOD_MS;
        if (time_reached(nowMs, st->nextSpeedRampMs) == TRUE)
        {
            st->nextSpeedRampMs = nowMs + FULL_AUTO_RAMP_PERIOD_MS;
        }

        if (st->currentSpeedPct < st->targetSpeedPct)
        {
            st->currentSpeedPct += (sint32)FULL_AUTO_RAMP_STEP_PCT;
            if (st->currentSpeedPct > st->targetSpeedPct)
            {
                st->currentSpeedPct = st->targetSpeedPct;
            }
        }
        else if (st->currentSpeedPct > st->targetSpeedPct)
        {
            st->currentSpeedPct -= (sint32)FULL_AUTO_RAMP_STEP_PCT;
            if (st->currentSpeedPct < st->targetSpeedPct)
            {
                st->currentSpeedPct = st->targetSpeedPct;
            }
        }

        if (st->currentSpeedPct <= 0)
        {
            st->currentSpeedPct = 0;
            Esc_StopNeutral();
        }
        else
        {
            Esc_SetBrake(0U, 0U);
            Esc_SetLogicalSpeed((int)(-st->currentSpeedPct), (int)(-st->currentSpeedPct));
        }
    }

    if (st->phase == RACE_PHASE_SPEED_RUN)
    {
        StatusLed_Green();
    }
    else if (st->phase == RACE_PHASE_HONOR_RUN)
    {
        StatusLed_Cyan();
    }
    else if ((st->phase == RACE_PHASE_FAULT) || (st->ultraError == TRUE))
    {
        StatusLed_Red();
    }
    else
    {
        StatusLed_Blue();
    }
}

static void race_mode_apply_teensy_camera_hard_fault(RaceModeState_t *st)
{
    if ((st == NULL_PTR) || (g_raceUsesTeensyCam0 != TRUE) ||
        (g_raceTeensyCam0Link.hardFault != TRUE))
    {
        return;
    }

    st->phase = RACE_PHASE_FAULT;
    st->targetSpeedPct = 0;
    st->currentSpeedPct = 0;
    st->steerRaw = 0;
    st->steerFilt = 0;
    st->steerOut = 0;
    Esc_StopNeutral();
    SteerStraight();
    StatusLed_Red();
}

static void race_mode_ensure_display_ready(RaceModeState_t *st)
{
    if ((st == NULL_PTR) || (st->displayInitialized == TRUE))
    {
        return;
    }

    /* The OLED path is blocking, so race mode keeps it completely out of the
       hot path until the dedicated debug switch explicitly requests it. */
    display_power_stabilize_delay();
    DisplayInit(0U, STD_ON);
    DisplayClear();
    DisplayRefresh();
    st->displayInitialized = TRUE;
}

static void race_mode_draw(const RaceModeState_t *st)
{
    const char *phaseText = "UNKNOWN";

    if (st == NULL_PTR)
    {
        return;
    }

    switch (st->phase)
    {
        case RACE_PHASE_ESC_ARM:
            phaseText = "ARM";
            break;

        case RACE_PHASE_SPEED_RUN:
            phaseText = "RUN";
            break;

        case RACE_PHASE_HONOR_RUN:
            phaseText = "HONOR";
            break;

        case RACE_PHASE_STOPPED:
            phaseText = "STOP";
            break;

        case RACE_PHASE_FAULT:
            phaseText = "FAULT";
            break;

        default:
            break;
    }

    DisplayTextPadded(0U, (g_raceUsesTeensyCam0 == TRUE) ? "TEENSY CAM RACE" : "RACE MODE");
    DisplayTextPadded(1U, "State:");
    DisplayText(1U, phaseText, (uint16)strlen(phaseText), 7U);
    DisplayTextPadded(2U, "Spd:    D:");
    DisplayValue(2U, (int)st->currentSpeedPct, 3U, 4U);

    if (st->haveValidDistance == TRUE)
    {
        DisplayValue(2U, (int)(st->lastDistanceCm + 0.5f), 3U, 11U);
    }
    else
    {
        DisplayText(2U, "---", 3U, 11U);
    }

    DisplayTextPadded(3U, "V:      F:");
    if ((st->haveValidVision == TRUE) && (st->result.status != VISION_TRACK_LOST))
    {
        DisplayValue(3U, (int)st->result.confidence, 3U, 2U);
    }
    else
    {
        DisplayText(3U, "LST", 3U, 2U);
    }
    DisplayValue(3U, (int)st->finishStreak, 2U, 12U);
    DisplayRefresh();
}

static void race_mode_run(boolean useTeensyCam0)
{
    uint32 nextButtonsMs;

    App_InitRuntimeCore();
    race_mode_enter(Timebase_GetMs(), useTeensyCam0, FALSE);
    nextButtonsMs = Timebase_GetMs();

    for (;;)
    {
        uint32 nowMs = Timebase_GetMs();
        boolean stopPressed;
        boolean displaySwitchOn;

        App_ServiceRuntimeCore(nowMs);

        while (time_reached(nowMs, nextButtonsMs) == TRUE)
        {
            Buttons_Update();
            nextButtonsMs += BUTTONS_PERIOD_MS;
        }

        /* SW2 events are drained even though production race mode autostarts
           after ESC arm. This keeps button latches from accumulating. */
        (void)Buttons_WasPressed(BUTTON_ID_SW2);
        stopPressed = Buttons_WasPressed(BUTTON_ID_SW3);
        displaySwitchOn = Buttons_IsOn(BUTTON_ID_SWPCB);

        if (useTeensyCam0 == TRUE)
        {
            race_mode_update_teensy_cam0(&g_raceMode, nowMs);
            nowMs = Timebase_GetMs();
            race_mode_apply_teensy_camera_hard_fault(&g_raceMode);
            race_mode_update_control(&g_raceMode, nowMs, stopPressed);
            nowMs = Timebase_GetMs();
            App_ServiceRuntimeCore(nowMs);
            race_mode_update_ultrasonic(&g_raceMode, nowMs);
        }
        else
        {
            race_mode_update_vision(&g_raceMode, nowMs);
            race_mode_update_ultrasonic(&g_raceMode, nowMs);
            race_mode_update_control(&g_raceMode, nowMs, stopPressed);
        }
        race_mode_apply_teensy_camera_hard_fault(&g_raceMode);
        App_ServiceRuntimeCore(nowMs);

        if ((displaySwitchOn == TRUE) && (g_raceMode.displayInitialized != TRUE) &&
            (g_raceMode.phase == RACE_PHASE_ESC_ARM))
        {
            race_mode_ensure_display_ready(&g_raceMode);
            nowMs = Timebase_GetMs();
            g_raceMode.nextDisplayMs = nowMs;
        }

        if ((displaySwitchOn == TRUE) && (g_raceMode.displayInitialized == TRUE) &&
            ((useTeensyCam0 != TRUE) || (g_raceMode.phase == RACE_PHASE_ESC_ARM) ||
             (g_raceMode.phase == RACE_PHASE_STOPPED) || (g_raceMode.phase == RACE_PHASE_FAULT)) &&
            ((g_raceMode.displayWasOn != TRUE) ||
             (time_reached(nowMs, g_raceMode.nextDisplayMs) == TRUE)))
        {
            g_raceMode.nextDisplayMs = nowMs + RACE_DISPLAY_PERIOD_MS;
            race_mode_draw(&g_raceMode);
        }

        g_raceMode.displayWasOn = displaySwitchOn;
    }
}

void mode_race_mode(void)
{
    race_mode_run(FALSE);
}

void mode_teensy_cam0_race(void)
{
    race_mode_run(TRUE);
}

void tune_drive_test_enter(uint32 nowMs)
{
    EspUartLink_Init();
    race_mode_enter(nowMs, TRUE, TRUE);

    DisplayClear();
    DisplayTextPadded(0U, "TUNE DRIVE");
    DisplayTextPadded(1U, "ESP LIVE TUNE");
    DisplayTextPadded(2U, "SW3 STOP");
    DisplayTextPadded(3U, "SWPCB EXIT");
    DisplayRefresh();
}

void tune_drive_test_update(uint32 nowMs, boolean stopPressed)
{
    EspUartLink_TuneFrame_t tune;
    (void)tune;

    if (esp_link_service_tune_frames(nowMs, &tune) == TRUE)
    {
        race_mode_apply_runtime_tune(&g_raceMode);
    }

    App_ServiceRuntimeCore(nowMs);
    race_mode_update_teensy_cam0(&g_raceMode, nowMs);
    nowMs = Timebase_GetMs();
    race_mode_apply_teensy_camera_hard_fault(&g_raceMode);
    race_mode_update_control(&g_raceMode, nowMs, stopPressed);
    nowMs = Timebase_GetMs();
    App_ServiceRuntimeCore(nowMs);
    race_mode_update_ultrasonic(&g_raceMode, nowMs);
    race_mode_apply_teensy_camera_hard_fault(&g_raceMode);
    App_ServiceRuntimeCore(nowMs);
}

void tune_drive_test_exit(void)
{
    g_raceTuneDriveMode = FALSE;
    Esc_StopNeutral();
    SteerStraight();
    StatusLed_Off();
}
