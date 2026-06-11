#include "../app_internal.h"

typedef struct
{
    uint32 nextServiceMs;
    uint32 lastValidCameraMs;
    uint32 lastAcceptedRxMs;
    uint16 controlSeq;
    TeensyLinkSnapshot_t snapshot;
    boolean haveAcceptedFrame;
    boolean hardFault;
} RaceTeensyCam0LinkState_t;

static RaceTeensyCam0LinkState_t g_raceTeensyCam0Link;
static boolean g_raceUsesTeensyCam0;

static void race_mode_enter(uint32 nowMs, boolean useTeensyCam0)
{
    (void)memset(&g_raceMode, 0, sizeof(g_raceMode));
    (void)memset(&g_raceTeensyCam0Link, 0, sizeof(g_raceTeensyCam0Link));
    g_raceUsesTeensyCam0 = useTeensyCam0;

    /* Race mode owns all actuators directly. Initialize them once, then keep
       the runtime loop strictly ordered: vision -> ultrasonic -> control. */
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
    SteerStraight();

    if (useTeensyCam0 == TRUE)
    {
        TeensyLink_Init();
        g_raceTeensyCam0Link.nextServiceMs = nowMs;
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

    (void)memcpy(st->processedFrame.Values,
                 &latestFrame->Values[CAM_TRIM_LEFT_PX],
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
    camera->flags =
        (uint8)(TEENSY_LINK_CAMERA_FLAG_SOURCE_S32K |
                TEENSY_LINK_CAMERA_FLAG_STALE);
}

static void race_mode_fill_teensy_link_input(const RaceModeState_t *st,
                                             TeensyLinkS32kInputs_t *input)
{
    if ((st == NULL_PTR) || (input == NULL_PTR))
    {
        return;
    }

    (void)memset(input, 0, sizeof(*input));
    input->controlLoopSeq = g_raceTeensyCam0Link.controlSeq;
    input->controlDtUs = (uint16)((uint16)TEENSY_LINK_SERVICE_PERIOD_MS * 1000U);
    input->appMode = (uint8)APP_BUILD_MODE_TEENSY_CAM0_RACE;
    input->appState = (uint8)st->phase;
    input->safetyFlags = (g_raceTeensyCam0Link.hardFault == TRUE) ? 1U : 0U;
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

static void race_mode_update_teensy_cam0(RaceModeState_t *st, uint32 nowMs)
{
    TeensyLinkS32kInputs_t input;
    VisionOutput_t vision;

    if (st == NULL_PTR)
    {
        return;
    }

    if (time_reached(nowMs, g_raceTeensyCam0Link.nextServiceMs) == TRUE)
    {
        g_raceTeensyCam0Link.controlSeq++;
        race_mode_fill_teensy_link_input(st, &input);
        (void)TeensyLink_Service(nowMs, &input);
        g_raceTeensyCam0Link.nextServiceMs += (uint32)TEENSY_LINK_SERVICE_PERIOD_MS;

        if (time_reached(nowMs, g_raceTeensyCam0Link.nextServiceMs) == TRUE)
        {
            g_raceTeensyCam0Link.nextServiceMs =
                nowMs + (uint32)TEENSY_LINK_SERVICE_PERIOD_MS;
        }
    }

    (void)TeensyLink_GetSnapshot(&g_raceTeensyCam0Link.snapshot);

    if (TeensyCameraSource_GetCamera0Vision(&g_raceTeensyCam0Link.snapshot,
                                            nowMs,
                                            (uint32)TEENSY_CAM0_CONTROL_MAX_AGE_MS,
                                            &vision) == TRUE)
    {
        st->result = vision;
        st->haveValidVision = TRUE;

        if ((g_raceTeensyCam0Link.haveAcceptedFrame != TRUE) ||
            (g_raceTeensyCam0Link.snapshot.lastRxMs !=
             g_raceTeensyCam0Link.lastAcceptedRxMs))
        {
            g_raceTeensyCam0Link.lastAcceptedRxMs =
                g_raceTeensyCam0Link.snapshot.lastRxMs;
            g_raceTeensyCam0Link.lastValidCameraMs = nowMs;
            g_raceTeensyCam0Link.haveAcceptedFrame = TRUE;
        }
    }
    else if ((uint32)(nowMs - g_raceTeensyCam0Link.lastValidCameraMs) >
             (uint32)TEENSY_CAM0_CONTROL_MAX_AGE_MS)
    {
        st->haveValidVision = FALSE;
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

    if ((st == NULL_PTR) || (time_reached(nowMs, st->nextControlMs) != TRUE))
    {
        return;
    }

    st->nextControlMs += STEER_UPDATE_MS;
    if (time_reached(nowMs, st->nextControlMs) == TRUE)
    {
        st->nextControlMs = nowMs + STEER_UPDATE_MS;
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
            ((st->haveValidVision == TRUE) && (st->result.status != VISION_TRACK_LOST)) ? 0U : nowMs;
    }

    if ((st->phase == RACE_PHASE_SPEED_RUN) &&
        (st->haveValidVision == TRUE) &&
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
        (boolean)((st->phase == RACE_PHASE_HONOR_RUN) &&
                  (st->haveValidDistance == TRUE) &&
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
            float dt = ((float)STEER_UPDATE_MS) * 0.001f;
            VehicleControlOutput_t out;

            controllerBaseSpeed = (st->currentSpeedPct > 0) ? (uint8)st->currentSpeedPct : FULL_AUTO_SPEED_PCT;
            out = SteeringController_Update(&st->ctrl, &st->result, dt, controllerBaseSpeed);
            st->steerRaw = (sint16)out.steer_cmd;

            if (((st->steerRaw < 0) ? (sint16)(-st->steerRaw) : st->steerRaw) <= 2)
            {
                st->steerRaw = 0;
            }

            st->steerFilt = SteeringSmooth_IirS16(st->steerFilt, st->steerRaw, 0.45f);

            {
                const sint16 steerRateMax = 8;
                sint16 delta = (sint16)(st->steerFilt - st->steerOut);

                delta = SteeringSmooth_ClampS16(delta, (sint16)(-steerRateMax), (sint16)(+steerRateMax));
                st->steerOut = (sint16)(st->steerOut + delta);
            }

            st->steerOut = SteeringSmooth_ClampS16(st->steerOut,
                                                   (sint16)(-STEER_CMD_CLAMP),
                                                   (sint16)(+STEER_CMD_CLAMP));
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
        sint32 steerSlowPct = ((sint32)((st->steerOut < 0) ? (sint16)(-st->steerOut) : st->steerOut) * (sint32)SPEED_SLOW_PER_STEER) /
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
            Esc_SetLogicalSpeed((int)(-st->currentSpeedPct),
                                (int)(-st->currentSpeedPct));
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
    if ((st == NULL_PTR) ||
        (g_raceUsesTeensyCam0 != TRUE) ||
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

    DisplayTextPadded(0U,
                      (g_raceUsesTeensyCam0 == TRUE) ? "TEENSY CAM RACE" : "RACE MODE");
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
    race_mode_enter(Timebase_GetMs(), useTeensyCam0);
    nextButtonsMs = Timebase_GetMs();

    for (;;)
    {
        uint32 nowMs = Timebase_GetMs();
        boolean stopPressed;
        boolean displaySwitchOn;

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
        }
        else
        {
            race_mode_update_vision(&g_raceMode, nowMs);
        }
        race_mode_update_ultrasonic(&g_raceMode, nowMs);
        race_mode_update_control(&g_raceMode, nowMs, stopPressed);
        race_mode_apply_teensy_camera_hard_fault(&g_raceMode);

        if ((displaySwitchOn == TRUE) &&
            (g_raceMode.displayInitialized != TRUE) &&
            (g_raceMode.phase == RACE_PHASE_ESC_ARM))
        {
            race_mode_ensure_display_ready(&g_raceMode);
            nowMs = Timebase_GetMs();
            g_raceMode.nextDisplayMs = nowMs;
        }

        if ((displaySwitchOn == TRUE) &&
            (g_raceMode.displayInitialized == TRUE) &&
            ((g_raceMode.displayWasOn != TRUE) || (time_reached(nowMs, g_raceMode.nextDisplayMs) == TRUE)))
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
