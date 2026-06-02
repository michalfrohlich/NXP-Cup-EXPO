#include "../app_internal.h"

static void esc_test_start_arming(EscManualTestState_t *st, uint32 nowMs);
static void esc_test_disarm(EscManualTestState_t *st);
static void esc_test_estop(EscManualTestState_t *st);
static sint16 esc_test_command_from_pot(uint8 pot);
static void esc_test_apply_outputs(EscManualTestState_t *st);
static void esc_test_update_led(const EscManualTestState_t *st, uint32 nowMs);
static void esc_test_draw(const EscManualTestState_t *st);
static void linear_camera_test_draw_waiting(const LinearCameraTestState_t *st);

void receiver_test_enter(uint32 nowMs)
{
    (void)memset(&g_receiverTest, 0, sizeof(g_receiverTest));

    ReceiverInit(0U, 0U, 11700U, 17700U, 23700U, 26000U);
    StatusLed_Blue();

    DisplayClear();
    DisplayTextPadded(0U, "Ch0:    Ch1:");
    DisplayTextPadded(1U, "Ch2:    Ch3:");
    DisplayTextPadded(2U, "Ch4:    Ch5:");
    DisplayTextPadded(3U, "Ch6:    Ch7:");
    DisplayRefresh();

    g_receiverTest.nextRefreshMs = nowMs;
}

void receiver_test_update(uint32 nowMs)
{
    uint8 displayValueOffset;
    int receiverChannels[8];

    if (time_reached(nowMs, g_receiverTest.nextRefreshMs) != TRUE)
    {
        return;
    }

    g_receiverTest.nextRefreshMs = nowMs + RECEIVER_REFRESH_MS;

    for (uint8 i = 0U; i < 8U; i++)
    {
        receiverChannels[i] = GetReceiverChannel(i);
        displayValueOffset = (uint8)(4U + 8U * (i % 2U));
        DisplayValue(i / 2U, receiverChannels[i], 4U, displayValueOffset);
    }

    DisplayRefresh();
}

void receiver_test_exit(void)
{
}

void ultrasonic_test_enter(uint32 nowMs)
{
    (void)memset(&g_ultrasonicTest, 0, sizeof(g_ultrasonicTest));

    Ultrasonic_Init();
    g_ultrasonicTest.nextTriggerMs = nowMs;
    g_ultrasonicTest.nextDisplayMs = nowMs;
    g_ultrasonicTest.lastDistanceCm = 0.0f;
    g_ultrasonicTest.haveValidDistance = FALSE;
    g_ultrasonicTest.ultraEnableMs = nowMs + NXP_ULTRA_ENABLE_AFTER_RUN_MS;
    g_ultrasonicTest.mode = ULTRA_TEST_MODE_CLEAR;
    g_ultrasonicTest.modeUntilMs = 0U;
    g_ultrasonicTest.slowRangeSinceMs = 0U;
    g_ultrasonicTest.stopRangeSinceMs = 0U;
    StatusLed_Blue();
}

void ultrasonic_test_update(uint32 nowMs, boolean sw2Pressed)
{
    Ultrasonic_StatusType st;
    boolean active;
    boolean inStopRange = FALSE;
    boolean inSlowRange = FALSE;

    if (sw2Pressed == TRUE)
    {
        Ultrasonic_Init();
        g_ultrasonicTest.nextTriggerMs = nowMs;
        g_ultrasonicTest.ultraEnableMs = nowMs + NXP_ULTRA_ENABLE_AFTER_RUN_MS;
        g_ultrasonicTest.haveValidDistance = FALSE;
        g_ultrasonicTest.lastDistanceCm = 0.0f;
        g_ultrasonicTest.mode = ULTRA_TEST_MODE_CLEAR;
        g_ultrasonicTest.modeUntilMs = 0U;
        g_ultrasonicTest.slowRangeSinceMs = 0U;
        g_ultrasonicTest.stopRangeSinceMs = 0U;
    }

    Ultrasonic_Task();
    st = Ultrasonic_GetStatus();

    if (Ultrasonic_GetDistanceCm(&g_ultrasonicTest.lastDistanceCm) == TRUE)
    {
        if (g_ultrasonicTest.lastDistanceCm > 0.0f)
        {
            g_ultrasonicTest.haveValidDistance = TRUE;
        }
    }

    if (time_reached(nowMs, g_ultrasonicTest.nextTriggerMs) == TRUE)
    {
        if (Ultrasonic_GetStatus() != ULTRA_STATUS_BUSY)
        {
            Ultrasonic_StartMeasurement();
            g_ultrasonicTest.nextTriggerMs = nowMs + (uint32)NXP_CUP_ULTRA_TRIGGER_PERIOD_MS;
        }
    }

    active = (time_reached(nowMs, g_ultrasonicTest.ultraEnableMs) == TRUE) ? TRUE : FALSE;

    if ((active == TRUE) && (g_ultrasonicTest.haveValidDistance == TRUE))
    {
        inStopRange = (g_ultrasonicTest.lastDistanceCm <= (float)NXP_CUP_ULTRA_CRAWL_STOP_CM) ? TRUE : FALSE;
        inSlowRange = ((g_ultrasonicTest.lastDistanceCm <= (float)NXP_CUP_ULTRA_STOP_CM) &&
                       (inStopRange != TRUE)) ? TRUE : FALSE;

        if (inStopRange == TRUE)
        {
            g_ultrasonicTest.mode = ULTRA_TEST_MODE_CUTOFF;
            StatusLed_Red();
        }
        else if (inSlowRange == TRUE)
        {
            g_ultrasonicTest.mode = ULTRA_TEST_MODE_CRAWL;
            StatusLed_Yellow();
        }
        else
        {
            g_ultrasonicTest.mode = ULTRA_TEST_MODE_CLEAR;
            StatusLed_Green();
        }
    }
    else
    {
        g_ultrasonicTest.mode = ULTRA_TEST_MODE_CLEAR;

        if (st == ULTRA_STATUS_ERROR)
        {
            StatusLed_Red();
        }
        else if (active == TRUE)
        {
            StatusLed_Green();
        }
        else
        {
            StatusLed_Blue();
        }
    }

    if (time_reached(nowMs, g_ultrasonicTest.nextDisplayMs) != TRUE)
    {
        return;
    }

    g_ultrasonicTest.nextDisplayMs = nowMs + DISPLAY_PERIOD_MS;

    DisplayClear();
    DisplayText(0U, "ULTRA TEST", 10U, 0U);

    if (g_ultrasonicTest.haveValidDistance == TRUE)
    {
        DisplayText(1U, "CM:", 3U, 0U);
        DisplayValue(1U, (int)(g_ultrasonicTest.lastDistanceCm + 0.5f), 4U, 4U);
    }
    else
    {
        DisplayText(1U, "CM: ----", 8U, 0U);
    }

    if (active != TRUE)
    {
        DisplayText(2U, "MODE: WAIT", 10U, 0U);
    }
    else if (g_ultrasonicTest.haveValidDistance != TRUE)
    {
        DisplayText(2U, "MODE: SCAN", 10U, 0U);
    }
    else if (g_ultrasonicTest.mode == ULTRA_TEST_MODE_CUTOFF)
    {
        DisplayText(2U, "MODE: STOP", 10U, 0U);
    }
    else if (g_ultrasonicTest.mode == ULTRA_TEST_MODE_CRAWL)
    {
        DisplayText(2U, "MODE: SLOW", 10U, 0U);
    }
    else
    {
        DisplayText(2U, "MODE: CLEAR", 11U, 0U);
    }

    switch (st)
    {
        case ULTRA_STATUS_BUSY:
            DisplayText(3U, "DRV: BUSY", 9U, 0U);
            break;
        case ULTRA_STATUS_NEW_DATA:
            DisplayText(3U, "DRV: NEW", 8U, 0U);
            break;
        case ULTRA_STATUS_ERROR:
            DisplayText(3U, "DRV: ERR", 8U, 0U);
            break;
        case ULTRA_STATUS_IDLE:
        default:
            DisplayText(3U, "DRV: IDLE", 9U, 0U);
            break;
    }

    DisplayRefresh();
}

void ultrasonic_test_exit(void)
{
}

void ultrasonic_esc_test_enter(uint32 nowMs)
{
    (void)memset(&g_ultrasonicEscTest, 0, sizeof(g_ultrasonicEscTest));

    Esc_Init(ESC_PWM_CH, ESC_SECOND_PWM_CH, ESC_DUTY_MIN, ESC_DUTY_MED, ESC_DUTY_MAX);
    Esc_StopNeutral();

    Servo_Init(SERVO_PWM_CH, SERVO_DUTY_MAX, SERVO_DUTY_MIN, SERVO_DUTY_MED);
    SteerStraight();

    Ultrasonic_Init();
    g_ultrasonicEscTest.armDoneMs = nowMs + ESC_ARM_TIME_MS;
    g_ultrasonicEscTest.nextUltraTrigMs = nowMs;
    g_ultrasonicEscTest.nextDisplayMs = nowMs;
    g_ultrasonicEscTest.lastDistanceMs = nowMs;
    g_ultrasonicEscTest.lastDistanceCm = 0.0f;
    g_ultrasonicEscTest.commandedSpeedPct = 0;
    g_ultrasonicEscTest.mode = ULTRA_TEST_MODE_CLEAR;

    DisplayTextPadded(0U, "ULTRA+ESC TEST");
    DisplayTextPadded(1U, "ARM  D: ---cm");
    DisplayTextPadded(2U, "SPD:   0 T: 60");
    DisplayTextPadded(3U, "45 LOW 8 STOP");
    DisplayRefresh();
    StatusLed_Blue();
}

void ultrasonic_esc_test_update(uint32 nowMs)
{
    const uint32 ultraStaleMs =
        ((uint32)HONOR_ULTRA_TRIGGER_PERIOD_MS * 2U) + (uint32)ULTRA_TIMEOUT_MS;
    Ultrasonic_StatusType st;
    float distanceCm = 0.0f;
    const char *modeText = "WAIT";

    Ultrasonic_Task();

    if (time_reached(nowMs, g_ultrasonicEscTest.nextUltraTrigMs) == TRUE)
    {
        g_ultrasonicEscTest.nextUltraTrigMs = nowMs + HONOR_ULTRA_TRIGGER_PERIOD_MS;

        if (Ultrasonic_GetStatus() != ULTRA_STATUS_BUSY)
        {
            Ultrasonic_StartMeasurement();
        }
    }

    st = Ultrasonic_GetStatus();

    if (Ultrasonic_GetDistanceCm(&distanceCm) == TRUE)
    {
        g_ultrasonicEscTest.lastDistanceCm = distanceCm;
        g_ultrasonicEscTest.lastDistanceMs = nowMs;
        g_ultrasonicEscTest.hasValidDistance = TRUE;
        st = ULTRA_STATUS_NEW_DATA;
    }
    else if (st == ULTRA_STATUS_ERROR)
    {
        g_ultrasonicEscTest.hasValidDistance = FALSE;
    }
    else if ((g_ultrasonicEscTest.hasValidDistance == TRUE) &&
             ((uint32)(nowMs - g_ultrasonicEscTest.lastDistanceMs) > ultraStaleMs))
    {
        g_ultrasonicEscTest.hasValidDistance = FALSE;
    }

    if (time_reached(nowMs, g_ultrasonicEscTest.armDoneMs) != TRUE)
    {
        g_ultrasonicEscTest.commandedSpeedPct = 0;
        g_ultrasonicEscTest.mode = ULTRA_TEST_MODE_CLEAR;
        Esc_StopNeutral();
        SteerStraight();
        modeText = "ARM ";
        StatusLed_Blue();
    }
    else if (st == ULTRA_STATUS_ERROR)
    {
        g_ultrasonicEscTest.commandedSpeedPct = 0;
        g_ultrasonicEscTest.mode = ULTRA_TEST_MODE_CUTOFF;
        Esc_StopNeutral();
        SteerStraight();
        modeText = "ERR ";
        StatusLed_Red();
    }
    else if (g_ultrasonicEscTest.hasValidDistance != TRUE)
    {
        g_ultrasonicEscTest.commandedSpeedPct = 0;
        g_ultrasonicEscTest.mode = ULTRA_TEST_MODE_CLEAR;
        Esc_StopNeutral();
        SteerStraight();
        modeText = "WAIT";
        StatusLed_Yellow();
    }
    else
    {
        if (g_ultrasonicEscTest.lastDistanceCm <= (float)ULTRA_ESC_TEST_STOP_CM)
        {
            g_ultrasonicEscTest.commandedSpeedPct = 0;
            g_ultrasonicEscTest.mode = ULTRA_TEST_MODE_CUTOFF;
            Esc_StopNeutral();
            SteerStraight();
            modeText = "STOP";
            StatusLed_Red();
        }
        else if (g_ultrasonicEscTest.lastDistanceCm <= (float)ULTRA_ESC_TEST_SLOW_CM)
        {
            g_ultrasonicEscTest.commandedSpeedPct = (sint32)ULTRA_ESC_TEST_LOW_SPEED_PCT;
            g_ultrasonicEscTest.mode = ULTRA_TEST_MODE_CRAWL;
            Esc_SetBrake(0U, 0U);
            Esc_SetLogicalSpeed((int)(-g_ultrasonicEscTest.commandedSpeedPct),
                                (int)(-g_ultrasonicEscTest.commandedSpeedPct));
            SteerStraight();
            modeText = "LOW ";
            StatusLed_Yellow();
        }
        else
        {
            g_ultrasonicEscTest.commandedSpeedPct = (sint32)ULTRA_ESC_TEST_FAST_SPEED_PCT;
            g_ultrasonicEscTest.mode = ULTRA_TEST_MODE_CLEAR;
            Esc_SetBrake(0U, 0U);
            Esc_SetLogicalSpeed((int)(-g_ultrasonicEscTest.commandedSpeedPct),
                                (int)(-g_ultrasonicEscTest.commandedSpeedPct));
            SteerStraight();
            modeText = "FAST";
            StatusLed_Green();
        }
    }

    if (time_reached(nowMs, g_ultrasonicEscTest.nextDisplayMs) != TRUE)
    {
        return;
    }

    g_ultrasonicEscTest.nextDisplayMs = nowMs + DISPLAY_PERIOD_MS;

    DisplayTextPadded(0U, "ULTRA+ESC TEST");
    DisplayTextPadded(1U, "    D: ---cm");
    DisplayText(1U, modeText, 4U, 0U);
    if (g_ultrasonicEscTest.hasValidDistance == TRUE)
    {
        DisplayValue(1U, (int)(g_ultrasonicEscTest.lastDistanceCm + 0.5f), 3U, 8U);
        DisplayText(1U, "cm", 2U, 11U);
    }
    else if (st == ULTRA_STATUS_ERROR)
    {
        DisplayText(1U, "ERR", 3U, 8U);
    }

    DisplayTextPadded(2U, "SPD:   0 T: 60");
    DisplayValue(2U, (int)g_ultrasonicEscTest.commandedSpeedPct, 3U, 4U);
    DisplayText(2U, "%", 1U, 7U);
    DisplayValue(2U, (int)HONOR_ULTRA_TRIGGER_PERIOD_MS, 2U, 14U);

    DisplayTextPadded(3U, "45 LOW 8 STOP");
    DisplayRefresh();
}

void ultrasonic_esc_test_exit(void)
{
    Esc_StopNeutral();
    SteerStraight();
    StatusLed_Off();
}

void servo_test_enter(uint32 nowMs)
{
    (void)memset(&g_servoTest, 0, sizeof(g_servoTest));

    Servo_Init(SERVO_PWM_CH, SERVO_DUTY_MAX, SERVO_DUTY_MIN, SERVO_DUTY_MED);
    SteerStraight();

    g_servoTest.configured = FALSE;
    g_servoTest.useSmoothing = TRUE;
    g_servoTest.nextDisplayMs = nowMs;

    StatusLed_Blue();
}

void servo_test_update(uint32 nowMs, boolean sw2Pressed, uint8 potLevel)
{
    if (g_servoTest.configured != TRUE)
    {
        g_servoTest.useSmoothing = (potLevel >= 128U) ? TRUE : FALSE;

        DisplayText(0U, "SERVO MODE", 10U, 0U);
        if (g_servoTest.useSmoothing == TRUE)
        {
            DisplayText(1U, " RAW", 4U, 0U);
            DisplayText(2U, ">SMOOTH", 7U, 0U);
        }
        else
        {
            DisplayText(1U, ">RAW", 4U, 0U);
            DisplayText(2U, " SMOOTH", 7U, 0U);
        }
        DisplayText(3U, "SW2 ENTER", 9U, 0U);
        DisplayRefresh();

        if (sw2Pressed == TRUE)
        {
            g_servoTest.configured = TRUE;
            g_servoTest.nextDisplayMs = nowMs;
        }
        return;
    }

    if ((uint32)(nowMs - g_servoTest.nextDisplayMs) < SERVO_TEST_UPDATE_MS)
    {
        return;
    }
    g_servoTest.nextDisplayMs += SERVO_TEST_UPDATE_MS;

    g_servoTest.steerRaw = (sint16)((((sint32)potLevel * 200) / 255) - 100);
    if (g_servoTest.steerRaw > (sint16)SERVO_TEST_CMD_CLAMP)
    {
        g_servoTest.steerRaw = (sint16)SERVO_TEST_CMD_CLAMP;
    }
    if (g_servoTest.steerRaw < (sint16)(-SERVO_TEST_CMD_CLAMP))
    {
        g_servoTest.steerRaw = (sint16)(-SERVO_TEST_CMD_CLAMP);
    }

    if (sw2Pressed == TRUE)
    {
        g_servoTest.steerRaw = 0;
        g_servoTest.steerFilt = 0;
        g_servoTest.steerOut = 0;
        SteerStraight();
    }
    else
    {
        if (g_servoTest.useSmoothing == TRUE)
        {
            sint16 rateMax;
            sint16 absFilt;

            g_servoTest.steerRaw = SteeringSmooth_DeadzoneS16(g_servoTest.steerRaw,
                                                              (sint16)SERVO_TEST_DEADBAND,
                                                              (sint16)SERVO_TEST_CMD_CLAMP);
            g_servoTest.steerFilt = SteeringSmooth_IirS16(g_servoTest.steerFilt,
                                                          g_servoTest.steerRaw,
                                                          (float)SERVO_TEST_LPF_ALPHA);

            absFilt = (g_servoTest.steerFilt < 0) ? (sint16)(-g_servoTest.steerFilt) : g_servoTest.steerFilt;
            rateMax = (sint16)SERVO_TEST_RATE_MAX;
            rateMax = (sint16)(rateMax + (absFilt / 6));
            if (rateMax > (sint16)SERVO_TEST_CMD_CLAMP)
            {
                rateMax = (sint16)SERVO_TEST_CMD_CLAMP;
            }

            g_servoTest.steerOut = SteeringSmooth_RateLimitS16(g_servoTest.steerOut,
                                                               g_servoTest.steerFilt,
                                                               rateMax,
                                                               (sint16)SERVO_TEST_CMD_CLAMP,
                                                               0,
                                                               0);
            g_servoTest.steerOut = SteeringSmooth_ClampS16(g_servoTest.steerOut,
                                                           (sint16)(-SERVO_TEST_CMD_CLAMP),
                                                           (sint16)SERVO_TEST_CMD_CLAMP);
        }
        else
        {
            g_servoTest.steerFilt = g_servoTest.steerRaw;
            g_servoTest.steerOut = g_servoTest.steerRaw;
        }

        Servo_SetSteer((int)g_servoTest.steerOut);
    }

    if (g_servoTest.useSmoothing == TRUE)
    {
        DisplayText(0U, "SERVO SMOOTH", 12U, 0U);
    }
    else
    {
        DisplayText(0U, "SERVO RAW", 9U, 0U);
    }
    DisplayText(1U, "RAW:", 4U, 0U);
    DisplayValue(1U, (int)g_servoTest.steerRaw, 4U, 5U);
    DisplayText(2U, "OUT:", 4U, 0U);
    DisplayValue(2U, (int)g_servoTest.steerOut, 4U, 5U);
    DisplayText(3U, "LIM:", 4U, 0U);
    DisplayValue(3U, (int)SERVO_TEST_CMD_CLAMP, 4U, 5U);
    DisplayRefresh();

    StatusLed_Blue();
}

void servo_test_exit(void)
{
    SteerStraight();
}
void esc_test_enter(uint32 nowMs)
{
    (void)memset(&g_escTest, 0, sizeof(g_escTest));

    g_escTest.mode = ESC_TEST_MODE_BOTH;
    g_escTest.nextDisplayMs = nowMs;
    Esc_Init(ESC_PWM_CH, ESC_SECOND_PWM_CH, ESC_DUTY_MIN, ESC_DUTY_MED, ESC_DUTY_MAX);
    esc_test_disarm(&g_escTest);
    esc_test_draw(&g_escTest);
}

void esc_test_update(uint32 nowMs, boolean sw2Pressed, boolean sw3Pressed, uint8 potLevel)
{
    Sw2Action_t sw2Action;
    Sw2Action_t sw3Action;
    sint16 cmd;

    (void)sw2Pressed;
    (void)sw3Pressed;

    g_escTest.lastPotLevel = potLevel;
    sw2Action = ButtonTracker_Update(&g_escTest.sw2Tracker, BUTTON_ID_SW2, nowMs);
    sw3Action = ButtonTracker_Update(&g_escTest.sw3Tracker, BUTTON_ID_SW3, nowMs);

    if (sw3Action == SW2_ACTION_HOLD)
    {
        esc_test_estop(&g_escTest);
    }
    else if (sw2Action == SW2_ACTION_HOLD)
    {
        if ((g_escTest.state == ESC_TEST_STATE_DISARMED) ||
            (g_escTest.state == ESC_TEST_STATE_ESTOP))
        {
            esc_test_start_arming(&g_escTest, nowMs);
        }
        else
        {
            esc_test_disarm(&g_escTest);
        }
    }
    else
    {
        if ((g_escTest.state == ESC_TEST_STATE_DISARMED) ||
            (g_escTest.state == ESC_TEST_STATE_ARMED_NEUTRAL) ||
            (g_escTest.state == ESC_TEST_STATE_ESTOP))
        {
            if (sw2Action == SW2_ACTION_CLICK)
            {
                g_escTest.mode = (EscTestModeId_t)(((uint8)g_escTest.mode + 1U) %
                                                   (uint8)ESC_TEST_MODE_COUNT);
            }
            if (sw3Action == SW2_ACTION_CLICK)
            {
                g_escTest.reverse = (g_escTest.reverse == TRUE) ? FALSE : TRUE;
            }
        }
    }

    cmd = esc_test_command_from_pot(potLevel);
    if (g_escTest.reverse != TRUE)
    {
        cmd = (sint16)(-cmd);
    }
    g_escTest.commandPct = cmd;

    if ((g_escTest.state == ESC_TEST_STATE_ARMING) &&
        (time_reached(nowMs, g_escTest.armDoneMs) == TRUE))
    {
        g_escTest.state = ESC_TEST_STATE_ARMED_NEUTRAL;
    }

    if ((g_escTest.state == ESC_TEST_STATE_ARMED_NEUTRAL) ||
        (g_escTest.state == ESC_TEST_STATE_RUNNING))
    {
        if (g_escTest.commandPct == 0)
        {
            g_escTest.requirePotZero = FALSE;
            g_escTest.state = ESC_TEST_STATE_ARMED_NEUTRAL;
        }
        else if (g_escTest.requirePotZero != TRUE)
        {
            g_escTest.state = ESC_TEST_STATE_RUNNING;
        }
        else
        {
            g_escTest.state = ESC_TEST_STATE_ARMED_NEUTRAL;
        }
    }

    esc_test_apply_outputs(&g_escTest);
    esc_test_update_led(&g_escTest, nowMs);

    if (time_reached(nowMs, g_escTest.nextDisplayMs) == TRUE)
    {
        g_escTest.nextDisplayMs = nowMs + DISPLAY_PERIOD_MS;
        esc_test_draw(&g_escTest);
    }
}

void esc_test_exit(void)
{
    Esc_StopNeutral();
}

static void esc_test_start_arming(EscManualTestState_t *st, uint32 nowMs)
{
    if (st == NULL_PTR)
    {
        return;
    }

    st->state = ESC_TEST_STATE_ARMING;
    st->armDoneMs = nowMs + ESC_ARM_TIME_MS;
    st->requirePotZero = TRUE;
    st->commandPct = 0;
    st->primaryCmdPct = 0;
    st->secondaryCmdPct = 0;
    Esc_StopNeutral();
}

static void esc_test_disarm(EscManualTestState_t *st)
{
    if (st == NULL_PTR)
    {
        return;
    }

    st->state = ESC_TEST_STATE_DISARMED;
    st->requirePotZero = TRUE;
    st->commandPct = 0;
    st->primaryCmdPct = 0;
    st->secondaryCmdPct = 0;
    Esc_StopNeutral();
    StatusLed_Blue();
}

static void esc_test_estop(EscManualTestState_t *st)
{
    if (st == NULL_PTR)
    {
        return;
    }

    st->state = ESC_TEST_STATE_ESTOP;
    st->requirePotZero = TRUE;
    st->commandPct = 0;
    st->primaryCmdPct = 0;
    st->secondaryCmdPct = 0;
    Esc_StopNeutral();
}

static sint16 esc_test_command_from_pot(uint8 pot)
{
    sint16 cmd = (sint16)(((uint32)pot * (uint32)ESC_TEST_MAX_SPEED_PCT) / 255U);

    if (cmd <= (sint16)MOTOR_DEADBAND_PCT)
    {
        cmd = 0;
    }

    return cmd;
}

static void esc_test_apply_outputs(EscManualTestState_t *st)
{
    sint16 cmd;

    if (st == NULL_PTR)
    {
        return;
    }

    st->primaryCmdPct = 0;
    st->secondaryCmdPct = 0;

    if (st->state == ESC_TEST_STATE_RUNNING)
    {
        cmd = st->commandPct;

        switch (st->mode)
        {
            case ESC_TEST_MODE_ESC1:
                st->primaryCmdPct = cmd;
                break;
            case ESC_TEST_MODE_ESC2:
                st->secondaryCmdPct = cmd;
                break;
            case ESC_TEST_MODE_DIFF:
                st->primaryCmdPct = cmd;
                st->secondaryCmdPct = (sint16)(-cmd);
                break;
            case ESC_TEST_MODE_BOTH:
            default:
                st->primaryCmdPct = cmd;
                st->secondaryCmdPct = cmd;
                break;
        }
    }

    Esc_SetBrake(0U, 0U);
    Esc_SetLogicalSpeed((int)st->primaryCmdPct, (int)st->secondaryCmdPct);
}

static void esc_test_update_led(const EscManualTestState_t *st, uint32 nowMs)
{
    boolean blinkOn = (((nowMs / (uint32)ESC_TEST_LED_BLINK_MS) & 1U) == 0U) ? TRUE : FALSE;

    if (st == NULL_PTR)
    {
        return;
    }

    switch (st->state)
    {
        case ESC_TEST_STATE_ARMING:
            if (blinkOn == TRUE) { StatusLed_Cyan(); } else { StatusLed_Off(); }
            break;
        case ESC_TEST_STATE_ARMED_NEUTRAL:
            if (blinkOn == TRUE) { StatusLed_Green(); } else { StatusLed_Off(); }
            break;
        case ESC_TEST_STATE_RUNNING:
            StatusLed_Green();
            break;
        case ESC_TEST_STATE_ESTOP:
            if (blinkOn == TRUE) { StatusLed_Red(); } else { StatusLed_Off(); }
            break;
        case ESC_TEST_STATE_DISARMED:
        default:
            StatusLed_Blue();
            break;
    }
}

static void esc_test_draw(const EscManualTestState_t *st)
{
    const char *stateText = "DIS ";
    const char *modeText = "BOTH";
    const char *dirText = "FWD";

    if (st == NULL_PTR)
    {
        return;
    }

    switch (st->state)
    {
        case ESC_TEST_STATE_ARMING:       stateText = "ARM "; break;
        case ESC_TEST_STATE_ARMED_NEUTRAL:stateText = "RDY "; break;
        case ESC_TEST_STATE_RUNNING:      stateText = "RUN "; break;
        case ESC_TEST_STATE_ESTOP:        stateText = "STOP"; break;
        case ESC_TEST_STATE_DISARMED:
        default:                          stateText = "DIS "; break;
    }

    switch (st->mode)
    {
        case ESC_TEST_MODE_ESC1: modeText = "ESC1"; break;
        case ESC_TEST_MODE_ESC2: modeText = "ESC2"; break;
        case ESC_TEST_MODE_DIFF: modeText = "DIFF"; break;
        case ESC_TEST_MODE_BOTH:
        default:                 modeText = "BOTH"; break;
    }

    if (st->reverse == TRUE)
    {
        dirText = "REV";
    }

    DisplayTextPadded(0U, "ESC TEST");
    DisplayText(0U, stateText, 4U, 9U);

    DisplayTextPadded(1U, "M:");
    DisplayText(1U, modeText, 4U, 2U);
    DisplayText(1U, dirText, 3U, 8U);

    DisplayTextPadded(2U, "E1:");
    DisplayValue(2U, (int)st->primaryCmdPct, 4U, 3U);
    DisplayText(2U, "E2:", 3U, 8U);
    DisplayValue(2U, (int)st->secondaryCmdPct, 4U, 11U);

    DisplayTextPadded(3U, "P:");
    DisplayValue(3U, (int)st->lastPotLevel, 3U, 2U);
    DisplayText(3U, "C:", 2U, 7U);
    DisplayValue(3U, (int)st->commandPct, 4U, 9U);
    DisplayText(3U, "%", 1U, 14U);

    DisplayRefresh();
}

void linear_camera_test_enter_common(uint32 nowMs, boolean servoEnabled)
{
    (void)memset(&g_linearCameraTest, 0, sizeof(g_linearCameraTest));

    g_linearCameraTest.servoEnabled = servoEnabled;
    g_linearCameraTest.steeringBaseSpeed = 20U;
    LinearCameraInit(CAM_CLK_PWM_CH, CAM_SHUTTER_GPT_CH, CAM_ADC_GROUP, CAM_SHUTTER_PCR);
    LinearCameraSetFrameIntervalTicks(CAM_FRAME_INTERVAL_TICKS);
    LineDetector_Init();
    VisionDebug_Init(&g_linearCameraTest.vdbg, 3200U);
    UartHost_ClearTxQueue();

    g_linearCameraTest.nextUiMs = nowMs;
    g_linearCameraTest.nextSteerMs = nowMs + STEER_UPDATE_MS;
    g_linearCameraTest.nextDisplayMs = nowMs;
    g_linearCameraTest.nextStreamMs = nowMs;

    if (servoEnabled == TRUE)
    {
        Servo_Init(SERVO_PWM_CH, SERVO_DUTY_MAX, SERVO_DUTY_MIN, SERVO_DUTY_MED);
        SteerStraight();
        SteeringController_Init(&g_linearCameraTest.ctrl);
        SteeringController_Reset(&g_linearCameraTest.ctrl);
    }

    /* Push the first visible OLED frame before enabling the camera ISR load.
       The display path is synchronous, while camera start immediately begins
       GPT/PWM/ADC activity that can disturb the first refresh. */
    linear_camera_test_draw_waiting(&g_linearCameraTest);
    (void)LinearCameraStartStream();
}

void linear_camera_test_enter(uint32 nowMs)
{
    linear_camera_test_enter_common(nowMs, FALSE);
}

static void linear_camera_test_draw_waiting(const LinearCameraTestState_t *st)
{
    LinearCameraDebugCounters counters;

    if (st == NULL_PTR)
    {
        return;
    }

    (void)memset(&counters, 0, sizeof(counters));
    LinearCameraGetDebugCounters(&counters);

    DisplayTextPadded(0U, "CAM WAIT FRAME");
    DisplayTextPadded(1U, "Req:    SI:    ");
    DisplayTextPadded(2U, "Frm:    Evt:   ");
    DisplayTextPadded(3U, "Drop:   P: B:  ");

    DisplayValue(1U, (int)counters.frameRequestCount, 4U, 4U);
    DisplayValue(1U, (int)counters.frameStartCount, 4U, 11U);
    DisplayValue(2U, (int)counters.frameCompleteCount, 4U, 4U);
    DisplayValue(2U, (int)counters.captureEventCount, 4U, 11U);
    DisplayValue(3U, (int)counters.droppedFrameCount, 4U, 5U);
    DisplayText(3U, (st->paused == TRUE) ? "Y" : "N", 1U, 12U);
    DisplayText(3U, (LinearCameraIsBusy() == TRUE) ? "Y" : "N", 1U, 15U);
    DisplayRefresh();
}

static sint8 linear_camera_test_stream_encode_error(float error)
{
    sint32 errorPct = (sint32)(error * 100.0f);

    if (errorPct > 127)
    {
        errorPct = 127;
    }
    else if (errorPct < -127)
    {
        errorPct = -127;
    }

    return (sint8)errorPct;
}

static sint8 linear_camera_test_stream_scale_gradient(sint16 gradient, uint16 scaleAbs)
{
    sint32 scaled;

    if (scaleAbs == 0U)
    {
        scaleAbs = 1U;
    }

    scaled = ((sint32)gradient * 127) / (sint32)scaleAbs;

    if (scaled > 127)
    {
        scaled = 127;
    }
    else if (scaled < -127)
    {
        scaled = -127;
    }

    return (sint8)scaled;
}

static uint8 linear_camera_test_stream_scale_sample(uint16 sample)
{
    sample = (uint16)(sample >> 4U);
    if (sample > 255U)
    {
        sample = 255U;
    }

    return (uint8)sample;
}

static void linear_camera_test_stream_frame(const LinearCameraTestState_t *st)
{
    uint8 packet[CAM_UART_STREAM_PACKET_BYTES];
    uint8 checksum = 0U;
    uint16 packetIndex = 10U;
    uint16 i;

    if (st == NULL_PTR)
    {
        return;
    }

    packet[0] = CAM_UART_STREAM_SYNC0;
    packet[1] = CAM_UART_STREAM_SYNC1;
    packet[2] = CAM_UART_STREAM_TYPE_FRAME;
    packet[3] = (uint8)(st->streamSeq & 0xFFU);
    packet[4] = (uint8)((st->streamSeq >> 8U) & 0xFFU);
    packet[5] = st->result.confidence;
    packet[6] = (uint8)st->result.status;
    packet[7] = (uint8)linear_camera_test_stream_encode_error(st->result.error);
    packet[8] = st->result.leftLineIdx;
    packet[9] = st->result.rightLineIdx;

    for (i = 0U; i < (uint16)VISION_LINEAR_BUFFER_SIZE; i++)
    {
        packet[packetIndex++] = linear_camera_test_stream_scale_sample(st->processedFrame.Values[i]);
    }

    for (i = 0U; i < (uint16)VISION_LINEAR_BUFFER_SIZE; i++)
    {
        packet[packetIndex++] = linear_camera_test_stream_scale_sample(st->filteredBuf[i]);
    }

    for (i = 0U; i < (uint16)VISION_LINEAR_BUFFER_SIZE; i++)
    {
        packet[packetIndex++] = (uint8)linear_camera_test_stream_scale_gradient(st->gradientBuf[i],
                                                                                st->dbg.maxAbsGradient);
    }

    packet[packetIndex++] = (uint8)(st->dbg.contrast & 0xFFU);
    packet[packetIndex++] = (uint8)((st->dbg.contrast >> 8U) & 0xFFU);
    packet[packetIndex++] = (uint8)(st->dbg.maxAbsGradient & 0xFFU);
    packet[packetIndex++] = (uint8)((st->dbg.maxAbsGradient >> 8U) & 0xFFU);
    packet[packetIndex++] = (uint8)(st->dbg.edgeHighThreshold & 0xFFU);
    packet[packetIndex++] = (uint8)((st->dbg.edgeHighThreshold >> 8U) & 0xFFU);
    packet[packetIndex++] = (uint8)(st->dbg.edgeLowThreshold & 0xFFU);
    packet[packetIndex++] = (uint8)((st->dbg.edgeLowThreshold >> 8U) & 0xFFU);

    packet[packetIndex++] = st->dbg.splitPoint;
    packet[packetIndex++] = st->dbg.finishGapLeftEdgeIdx;
    packet[packetIndex++] = st->dbg.finishGapRightEdgeIdx;
    packet[packetIndex++] = st->dbg.laneWidth;
    packet[packetIndex++] = st->dbg.expectedFinishGap;
    packet[packetIndex++] = st->dbg.measuredFinishGap;
    packet[packetIndex++] = st->dbg.edgeCount;

    for (i = 0U; i < (uint16)VLIN_MAX_EDGE_CANDIDATES; i++)
    {
        if (i < st->dbg.edgeCount)
        {
            packet[packetIndex++] = st->dbg.edges[i].idx;
            packet[packetIndex++] = (uint8)st->dbg.edges[i].polarity;
            packet[packetIndex++] = (uint8)(st->dbg.edges[i].strength & 0xFFU);
            packet[packetIndex++] = (uint8)((st->dbg.edges[i].strength >> 8U) & 0xFFU);
        }
        else
        {
            packet[packetIndex++] = VISION_LINEAR_INVALID_IDX;
            packet[packetIndex++] = 0U;
            packet[packetIndex++] = 0U;
            packet[packetIndex++] = 0U;
        }
    }

    for (i = 2U; i < (uint16)(CAM_UART_STREAM_PACKET_BYTES - 1U); i++)
    {
        checksum ^= packet[i];
    }
    packet[CAM_UART_STREAM_PACKET_BYTES - 1U] = checksum;

    if (UartHost_EnqueueBytes(packet, CAM_UART_STREAM_PACKET_BYTES) != TRUE)
    {
        g_linearCameraTest.streamDropCount++;
    }
}

static void linear_camera_test_process_latest_frame(uint32 nowMs)
{
    const LinearCameraFrame *latestFrame = NULL_PTR;
    boolean wantStream;

    if (g_linearCameraTest.paused == TRUE)
    {
        return;
    }

    if (LinearCameraGetLatestFrame(&latestFrame) != TRUE)
    {
        return;
    }

    wantStream = time_reached(nowMs, g_linearCameraTest.nextStreamMs);

    g_linearCameraTest.dbg.mask = (uint32)VLIN_DBG_NONE;
    g_linearCameraTest.dbg.filteredOut = NULL_PTR;
    g_linearCameraTest.dbg.gradientOut = NULL_PTR;

    if (wantStream == TRUE)
    {
        g_linearCameraTest.dbg.mask =
            (uint32)(VLIN_DBG_FILTERED | VLIN_DBG_GRADIENT | VLIN_DBG_STATS | VLIN_DBG_EDGES);
        g_linearCameraTest.dbg.filteredOut = g_linearCameraTest.filteredBuf;
        g_linearCameraTest.dbg.gradientOut = g_linearCameraTest.gradientBuf;
    }
    else if (VisionDebug_WantsVisionDebugData(&g_linearCameraTest.vdbg) == TRUE)
    {
        VisionDebug_PrepareVisionDbg(&g_linearCameraTest.vdbg,
                                     &g_linearCameraTest.dbg,
                                     g_linearCameraTest.filteredBuf,
                                     g_linearCameraTest.gradientBuf);
    }

    (void)memcpy(g_linearCameraTest.processedFrame.Values,
                 &latestFrame->Values[CAM_TRIM_LEFT_PX],
                 ((size_t)VISION_LINEAR_BUFFER_SIZE *
                  sizeof(g_linearCameraTest.processedFrame.Values[0])));
    LineDetector_ProcessDebug(g_linearCameraTest.processedFrame.Values,
                                &g_linearCameraTest.result,
                                &g_linearCameraTest.dbg);

    g_linearCameraTest.haveValidVision = TRUE;
    g_linearCameraTest.displayDirty = TRUE;

    if (wantStream == TRUE)
    {
        linear_camera_test_stream_frame(&g_linearCameraTest);
        g_linearCameraTest.streamSeq++;
        g_linearCameraTest.nextStreamMs = nowMs + CAM_UART_STREAM_PERIOD_MS;
    }
}

void linear_camera_test_update(uint32 nowMs, boolean modeNextPressed, uint8 potLevel)
{
    Sw2Action_t sw2Action;
    boolean sw2ClickHandled = FALSE;
    boolean screenTogglePressed;

    UartHost_ServiceTx();
    linear_camera_test_process_latest_frame(nowMs);

    if ((uint32)(nowMs - g_linearCameraTest.nextUiMs) < CAM_DEBUG_UI_PERIOD_MS)
    {
        return;
    }

    g_linearCameraTest.nextUiMs += CAM_DEBUG_UI_PERIOD_MS;

    sw2Action = Sw2Tracker_Update(&g_linearCameraTest.sw2Tracker, nowMs);

    if ((sw2Action == SW2_ACTION_HOLD) && (g_linearCameraTest.paused != TRUE))
    {
        if (LinearCameraIsBusy() == TRUE)
        {
            LinearCameraStopStream();
        }
        g_linearCameraTest.paused = TRUE;
        g_linearCameraTest.vdbg.cfg.paused = TRUE;
    }
    else if ((sw2Action == SW2_ACTION_CLICK) && (g_linearCameraTest.paused == TRUE))
    {
        g_linearCameraTest.paused = FALSE;
        g_linearCameraTest.vdbg.cfg.paused = FALSE;
        g_linearCameraTest.vdbg.screen = g_linearCameraTest.vdbg.debugEntryScreen;
        (void)LinearCameraStartStream();
        sw2ClickHandled = TRUE;
    }

    if (g_linearCameraTest.paused == TRUE)
    {
        g_linearCameraTest.vdbg.cfg.whiteSat = VisionDebug_WhiteMaxFromPot(potLevel);
        g_linearCameraTest.displayDirty = TRUE;
    }

    screenTogglePressed = (boolean)((sw2Action == SW2_ACTION_CLICK) &&
                                    (sw2ClickHandled != TRUE) &&
                                    (g_linearCameraTest.paused != TRUE));
    VisionDebug_OnTick(&g_linearCameraTest.vdbg,
                       screenTogglePressed,
                       modeNextPressed);

    if ((screenTogglePressed == TRUE) || (modeNextPressed == TRUE))
    {
        g_linearCameraTest.displayDirty = TRUE;
    }

    if ((g_linearCameraTest.servoEnabled == TRUE) &&
        (time_reached(nowMs, g_linearCameraTest.nextSteerMs) == TRUE))
    {
        g_linearCameraTest.nextSteerMs += STEER_UPDATE_MS;

        if ((g_linearCameraTest.paused == TRUE) || (g_linearCameraTest.haveValidVision != TRUE))
        {
            g_linearCameraTest.steerRaw = 0;
            g_linearCameraTest.steerFilt = 0;
            g_linearCameraTest.steerOut = 0;
            SteerStraight();
        }
        else
        {
            const float dt = ((float)STEER_UPDATE_MS) * 0.001f;
            VehicleControlOutput_t out =
                SteeringController_Update(&g_linearCameraTest.ctrl,
                                        &g_linearCameraTest.result,
                                        dt,
                                        g_linearCameraTest.steeringBaseSpeed);

            g_linearCameraTest.steerRaw = (sint16)out.steer_cmd;

            if (((g_linearCameraTest.steerRaw < 0) ? (sint16)(-g_linearCameraTest.steerRaw) : g_linearCameraTest.steerRaw) <= 2)
            {
                g_linearCameraTest.steerRaw = 0;
            }

            g_linearCameraTest.steerFilt = SteeringSmooth_IirS16(g_linearCameraTest.steerFilt,
                                                                 g_linearCameraTest.steerRaw,
                                                                 0.45f);

            {
                const sint16 steerRateMax = 8;
                sint16 delta = (sint16)(g_linearCameraTest.steerFilt - g_linearCameraTest.steerOut);

                delta = SteeringSmooth_ClampS16(delta, (sint16)(-steerRateMax), (sint16)(+steerRateMax));
                g_linearCameraTest.steerOut = (sint16)(g_linearCameraTest.steerOut + delta);
            }

            g_linearCameraTest.steerOut = SteeringSmooth_ClampS16(g_linearCameraTest.steerOut,
                                                                  (sint16)(-STEER_CMD_CLAMP),
                                                                  (sint16)(+STEER_CMD_CLAMP));

            Servo_SetSteer((int)g_linearCameraTest.steerOut);
        }
    }

    if (((g_linearCameraTest.displayDirty == TRUE) ||
         (g_linearCameraTest.haveValidVision != TRUE)) &&
        (time_reached(nowMs, g_linearCameraTest.nextDisplayMs) == TRUE))
    {
        g_linearCameraTest.nextDisplayMs = nowMs + CAM_DEBUG_DISPLAY_PERIOD_MS;

        if (g_linearCameraTest.haveValidVision == TRUE)
        {
            const uint16 *pFiltered =
                (g_linearCameraTest.dbg.filteredOut != NULL_PTR) ? g_linearCameraTest.filteredBuf : NULL_PTR;
            const sint16 *pGradient =
                (g_linearCameraTest.dbg.gradientOut != NULL_PTR) ? g_linearCameraTest.gradientBuf : NULL_PTR;
            const LineDetector_DebugOut_t *pDbg =
                (g_linearCameraTest.dbg.mask != (uint32)VLIN_DBG_NONE) ? &g_linearCameraTest.dbg : NULL_PTR;

            g_linearCameraTest.displayDirty = FALSE;
            VisionDebug_Draw(&g_linearCameraTest.vdbg,
                             g_linearCameraTest.processedFrame.Values,
                             pFiltered,
                             pGradient,
                             &g_linearCameraTest.result,
                             pDbg);
        }
        else
        {
            linear_camera_test_draw_waiting(&g_linearCameraTest);
        }
    }

}

void linear_camera_test_exit(void)
{
    if (g_linearCameraTest.servoEnabled == TRUE)
    {
        SteerStraight();
    }

    if (LinearCameraIsBusy() == TRUE)
    {
        LinearCameraStopStream();
    }

    UartHost_ClearTxQueue();
    StatusLed_Blue();
}
static void camservo_apply_profile(CamServoState_t *st, const CamTuneProfile_t *profile)
{
    if ((st == NULL_PTR) || (profile == NULL_PTR))
    {
        return;
    }

    st->activeTune = *profile;

    SteeringController_SetTunings(&st->ctrl,
                              profile->kp,
                              profile->kd,
                              1.0f);
    st->ctrl.ki = profile->ki;
    st->ctrl.iTermClamp = profile->iTermClamp;
    SteeringController_Reset(&st->ctrl);

    st->steerRaw = 0;
    st->steerFilt = 0;
    st->steerOut = 0;
    SteerStraight();
}

void camservo_enter_with_profile(CamServoState_t *st, uint32 nowMs, const CamTuneProfile_t *profile)
{
    Servo_Init(SERVO_PWM_CH, SERVO_DUTY_MAX, SERVO_DUTY_MIN, SERVO_DUTY_MED);
    SteerStraight();

    if (LinearCameraIsBusy() == TRUE)
    {
        LinearCameraStopStream();
    }

    LinearCameraInit(CAM_CLK_PWM_CH, CAM_SHUTTER_GPT_CH, CAM_ADC_GROUP, CAM_SHUTTER_PCR);
    LinearCameraSetFrameIntervalTicks(CAM_FRAME_INTERVAL_TICKS);
    LineDetector_Init();
    VisionDebug_Init(&st->vdbg, (uint8)V2_WHITE_SAT_U8);

    SteeringController_Init(&st->ctrl);
    SteeringController_Reset(&st->ctrl);

    (void)memset(&st->processedFrame, 0, sizeof(st->processedFrame));
    (void)memset(&st->result, 0, sizeof(st->result));
    (void)memset(&st->dbg, 0, sizeof(st->dbg));
    (void)memset(st->filteredBuf, 0, sizeof(st->filteredBuf));
    (void)memset(st->gradientBuf, 0, sizeof(st->gradientBuf));

    st->haveValidVision = FALSE;
    st->lastFrameMs = nowMs;
    st->nextControlMs = nowMs;
    st->nextSteerMs = nowMs + STEER_UPDATE_MS;
    st->nextDisplayMs = nowMs;
    st->displayDirty = FALSE;

    camservo_apply_profile(st, profile);

    (void)LinearCameraStartStream();
}

void camservo_enter(CamServoState_t *st, uint32 nowMs)
{
    RuntimeTune_InitDefaults();
    camservo_enter_with_profile(st, nowMs, &g_runtimeTune.profile);
}

static void camservo_process_latest_frame(CamServoState_t *st, uint32 nowMs)
{
    const LinearCameraFrame *latestFrame = NULL_PTR;

    if (st == NULL_PTR)
    {
        return;
    }

    if (LinearCameraGetLatestFrame(&latestFrame) != TRUE)
    {
        return;
    }

    st->dbg.mask = (uint32)VLIN_DBG_NONE;
    st->dbg.filteredOut = NULL_PTR;
    st->dbg.gradientOut = NULL_PTR;

    if (VisionDebug_WantsVisionDebugData(&st->vdbg) == TRUE)
    {
        VisionDebug_PrepareVisionDbg(&st->vdbg, &st->dbg, st->filteredBuf, st->gradientBuf);
    }

    (void)memcpy(st->processedFrame.Values,
                 &latestFrame->Values[CAM_TRIM_LEFT_PX],
                 ((size_t)VISION_LINEAR_BUFFER_SIZE * sizeof(st->processedFrame.Values[0])));
    LineDetector_ProcessDebug(st->processedFrame.Values, &st->result, &st->dbg);
    st->haveValidVision = TRUE;
    st->lastFrameMs = nowMs;
    st->displayDirty = TRUE;
}

void camservo_update(CamServoState_t *st, uint32 nowMs, boolean sw2)
{
    if (st == NULL_PTR)
    {
        return;
    }

    camservo_process_latest_frame(st, nowMs);

    if ((uint32)(nowMs - st->nextControlMs) < CAM_SERVO_CONTROL_PERIOD_MS)
    {
        return;
    }

    st->nextControlMs += CAM_SERVO_CONTROL_PERIOD_MS;

    VisionDebug_OnTick(&st->vdbg, sw2, FALSE);
    if (sw2 == TRUE)
    {
        st->displayDirty = TRUE;
    }

    if (time_reached(nowMs, st->nextSteerMs) == TRUE)
    {
        st->nextSteerMs += STEER_UPDATE_MS;

        if (st->haveValidVision != TRUE)
        {
            st->steerRaw = 0;
            st->steerFilt = 0;
            st->steerOut = 0;
            SteerStraight();
            return;
        }

        {
            const float dt = ((float)STEER_UPDATE_MS) * 0.001f;
            VehicleControlOutput_t out = SteeringController_Update(&st->ctrl,
                                                           &st->result,
                                                           dt,
                                                           st->activeTune.baseSpeedPct);

            st->steerRaw = (sint16)out.steer_cmd;
        }

        if (((st->steerRaw < 0) ? (sint16)(-st->steerRaw) : st->steerRaw) <= 2)
        {
            st->steerRaw = 0;
        }

        st->steerFilt = SteeringSmooth_IirS16(st->steerFilt,
                                              st->steerRaw,
                                              st->activeTune.steerLpfAlpha);

        {
            sint16 delta = (sint16)(st->steerFilt - st->steerOut);

            delta = SteeringSmooth_ClampS16(delta,
                                            (sint16)(-st->activeTune.steerRateMax),
                                            (sint16)(+st->activeTune.steerRateMax));
            st->steerOut = (sint16)(st->steerOut + delta);
        }

        st->steerOut = SteeringSmooth_ClampS16(st->steerOut,
                                               (sint16)(-st->activeTune.steerClamp),
                                               (sint16)(+st->activeTune.steerClamp));

        Servo_SetSteer((int)st->steerOut);
    }

    if ((st->haveValidVision == TRUE) &&
        (st->displayDirty == TRUE) &&
        (time_reached(nowMs, st->nextDisplayMs) == TRUE))
    {
        const uint16 *pFiltered = (st->dbg.filteredOut != NULL_PTR) ? st->filteredBuf : NULL_PTR;
        const sint16 *pGradient = (st->dbg.gradientOut != NULL_PTR) ? st->gradientBuf : NULL_PTR;
        const LineDetector_DebugOut_t *pDbg =
            (st->dbg.mask != (uint32)VLIN_DBG_NONE) ? &st->dbg : NULL_PTR;

        st->nextDisplayMs = nowMs + CAM_DEBUG_DISPLAY_PERIOD_MS;
        st->displayDirty = FALSE;
        VisionDebug_Draw(&st->vdbg, st->processedFrame.Values, pFiltered, pGradient, &st->result, pDbg);
    }
}

void camservo_exit(void)
{
    SteerStraight();

    if (LinearCameraIsBusy() == TRUE)
    {
        LinearCameraStopStream();
    }
}

void simple_drive_test_enter(uint32 nowMs)
{
    (void)memset(&g_simpleDriveTest, 0, sizeof(g_simpleDriveTest));

    Esc_Init(ESC_PWM_CH, ESC_SECOND_PWM_CH, ESC_DUTY_MIN, ESC_DUTY_MED, ESC_DUTY_MAX);
    Esc_StopNeutral();

    camservo_enter(&g_simpleDriveTest.camSt, nowMs);
    g_simpleDriveTest.armDoneMs = nowMs + ESC_ARM_TIME_MS;
    g_simpleDriveTest.nextAutoSpeedMs = g_simpleDriveTest.armDoneMs;
    g_simpleDriveTest.autoSpeedPct = 0;

    StatusLed_Blue();
}

void simple_drive_test_update(uint32 nowMs, boolean sw2Pressed, boolean sw3Pressed)
{
    camservo_update(&g_simpleDriveTest.camSt, nowMs, sw2Pressed);

    if (time_reached(nowMs, g_simpleDriveTest.armDoneMs) != TRUE)
    {
        g_simpleDriveTest.autoSpeedPct = 0;
        Esc_StopNeutral();
        StatusLed_Blue();
        return;
    }

    if (g_simpleDriveTest.runEnabled != TRUE)
    {
        if (sw3Pressed == TRUE)
        {
            g_simpleDriveTest.runEnabled = TRUE;
            g_simpleDriveTest.nextAutoSpeedMs = nowMs;
        }
        else
        {
            g_simpleDriveTest.autoSpeedPct = 0;
            Esc_StopNeutral();
            StatusLed_Blue();
            return;
        }
    }

    if (time_reached(nowMs, g_simpleDriveTest.nextAutoSpeedMs) == TRUE)
    {
        g_simpleDriveTest.nextAutoSpeedMs = nowMs + FULL_AUTO_RAMP_PERIOD_MS;

        if (g_simpleDriveTest.autoSpeedPct < (sint32)FULL_AUTO_SPEED_PCT)
        {
            g_simpleDriveTest.autoSpeedPct += (sint32)FULL_AUTO_RAMP_STEP_PCT;
            if (g_simpleDriveTest.autoSpeedPct > (sint32)FULL_AUTO_SPEED_PCT)
            {
                g_simpleDriveTest.autoSpeedPct = (sint32)FULL_AUTO_SPEED_PCT;
            }
        }

        if (g_simpleDriveTest.autoSpeedPct < 0)
        {
            g_simpleDriveTest.autoSpeedPct = 0;
        }
        if (g_simpleDriveTest.autoSpeedPct > 100)
        {
            g_simpleDriveTest.autoSpeedPct = 100;
        }

        Esc_SetBrake(0U, 0U);
        Esc_SetLogicalSpeed((int)(-g_simpleDriveTest.autoSpeedPct),
                            (int)(-g_simpleDriveTest.autoSpeedPct));
    }

    if ((g_simpleDriveTest.camSt.haveValidVision == TRUE) &&
        (g_simpleDriveTest.camSt.result.status != VISION_TRACK_LOST))
    {
        StatusLed_Cyan();
    }
    else
    {
        StatusLed_Yellow();
    }
}

void simple_drive_test_exit(void)
{
    camservo_exit();
    Esc_StopNeutral();
}
