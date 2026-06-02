#include "../app_internal.h"

static sint16 servo_rate_test_pot_to_command(uint8 potLevel);
static void servo_rate_test_enter(uint32 nowMs);
static void servo_rate_test_update(uint32 nowMs, boolean sw2Pressed, boolean sw3Pressed, uint8 potLevel);
static void servo_rate_test_draw(const ServoRateTestState_t *st);

static sint16 servo_rate_test_pot_to_command(uint8 potLevel)
{
    return (sint16)((((sint32)potLevel * 200) / 255) - 100);
}

static void servo_rate_test_enter(uint32 nowMs)
{
    (void)memset(&g_servoRateTest, 0, sizeof(g_servoRateTest));

    Servo_Init(SERVO_PWM_CH, SERVO_DUTY_MAX, SERVO_DUTY_MIN, SERVO_DUTY_MED);
    SteerStraight();

    g_servoRateTest.freqId = SERVO_RATE_TEST_FREQ_50HZ;
    g_servoRateTest.motionId = SERVO_RATE_TEST_MOTION_POT;
    g_servoRateTest.nextButtonsMs = nowMs;
    g_servoRateTest.nextCommandMs = nowMs;
    g_servoRateTest.nextDisplayMs = nowMs;
    g_servoRateTest.command = 0;
    g_servoRateTest.sweepDirection = 1;
    g_servoRateTest.lastPotLevel = (uint8)POT_CENTER_RAW;
    g_servoRateTest.startMs = nowMs;
    g_servoRateTest.noPwmCallbackFault = FALSE;
    g_servoRateTest.bufferedLagFault = FALSE;
    Servo_GetDebugSnapshot(&g_servoRateTest.servoDbg);

    servo_rate_test_draw(&g_servoRateTest);
    StatusLed_Green();
}

static void servo_rate_test_update(uint32 nowMs, boolean sw2Pressed, boolean sw3Pressed, uint8 potLevel)
{
    uint16 periodMs;

    if (sw2Pressed == TRUE)
    {
        g_servoRateTest.freqId =
            (ServoRateTestFreqId_t)(((uint8)g_servoRateTest.freqId + 1U) %
                                    (uint8)SERVO_RATE_TEST_FREQ_COUNT);
        g_servoRateTest.nextCommandMs = nowMs;
        g_servoRateTest.noPwmCallbackFault = FALSE;
        g_servoRateTest.bufferedLagFault = FALSE;
    }

    if (sw3Pressed == TRUE)
    {
        g_servoRateTest.motionId =
            (ServoRateTestMotionId_t)(((uint8)g_servoRateTest.motionId + 1U) %
                                      (uint8)SERVO_RATE_TEST_MOTION_COUNT);
        g_servoRateTest.command = 0;
        g_servoRateTest.sweepDirection = 1;
        g_servoRateTest.nextCommandMs = nowMs;
        g_servoRateTest.noPwmCallbackFault = FALSE;
        g_servoRateTest.bufferedLagFault = FALSE;
    }

    if ((uint8)g_servoRateTest.freqId >= (uint8)SERVO_RATE_TEST_FREQ_COUNT)
    {
        g_servoRateTest.freqId = SERVO_RATE_TEST_FREQ_50HZ;
    }
    if ((uint8)g_servoRateTest.motionId >= (uint8)SERVO_RATE_TEST_MOTION_COUNT)
    {
        g_servoRateTest.motionId = SERVO_RATE_TEST_MOTION_POT;
    }

    periodMs = g_servoRateTestPeriodMs[g_servoRateTest.freqId];
    g_servoRateTest.lastPotLevel = potLevel;
    Servo_GetDebugSnapshot(&g_servoRateTest.servoDbg);

    if (((uint32)(nowMs - g_servoRateTest.startMs) > 100U) &&
        (g_servoRateTest.servoDbg.PeriodCallbackCount == 0U))
    {
        g_servoRateTest.noPwmCallbackFault = TRUE;
    }

    if (time_reached(nowMs, g_servoRateTest.nextCommandMs) == TRUE)
    {
        g_servoRateTest.bufferedLagFault = g_servoRateTest.servoDbg.PendingUpdate;

        switch (g_servoRateTest.motionId)
        {
            case SERVO_RATE_TEST_MOTION_FINE_SWEEP:
                g_servoRateTest.command =
                    (sint16)(g_servoRateTest.command +
                             (g_servoRateTest.sweepDirection * (sint16)SERVO_RATE_TEST_FINE_STEP));
                break;

            case SERVO_RATE_TEST_MOTION_COARSE_SWEEP:
                g_servoRateTest.command =
                    (sint16)(g_servoRateTest.command +
                             (g_servoRateTest.sweepDirection * (sint16)SERVO_RATE_TEST_COARSE_STEP));
                break;

            case SERVO_RATE_TEST_MOTION_POT:
            default:
                g_servoRateTest.command = servo_rate_test_pot_to_command(potLevel);
                break;
        }

        if (g_servoRateTest.command >= (sint16)SERVO_RATE_TEST_MAX_CMD)
        {
            g_servoRateTest.command = (sint16)SERVO_RATE_TEST_MAX_CMD;
            g_servoRateTest.sweepDirection = -1;
        }
        else if (g_servoRateTest.command <= (sint16)(-SERVO_RATE_TEST_MAX_CMD))
        {
            g_servoRateTest.command = (sint16)(-SERVO_RATE_TEST_MAX_CMD);
            g_servoRateTest.sweepDirection = 1;
        }

        Servo_SetSteer((int)g_servoRateTest.command);
        Servo_GetDebugSnapshot(&g_servoRateTest.servoDbg);
        g_servoRateTest.nextCommandMs = nowMs + (uint32)periodMs;
    }

    if (time_reached(nowMs, g_servoRateTest.nextDisplayMs) == TRUE)
    {
        g_servoRateTest.nextDisplayMs = nowMs + SERVO_RATE_TEST_DISPLAY_MS;
        servo_rate_test_draw(&g_servoRateTest);
    }

    if ((g_servoRateTest.noPwmCallbackFault == TRUE) ||
        (g_servoRateTest.servoDbg.Initialized != TRUE) ||
        (g_servoRateTest.bufferedLagFault == TRUE))
    {
        StatusLed_Red();
    }
    else
    {
        StatusLed_Green();
    }
}

static void servo_rate_test_draw(const ServoRateTestState_t *st)
{
    char lineBuf[17];

    if (st == NULL_PTR)
    {
        return;
    }

    DisplayTextPadded(0U, "SW2 Hz SW3 MODE");

    (void)snprintf(lineBuf,
                   sizeof(lineBuf),
                   "F:%4u %s",
                   (unsigned int)g_servoRateTestFreqHz[st->freqId],
                   (st->noPwmCallbackFault == TRUE) ? "NOISR" :
                   ((st->bufferedLagFault == TRUE) ? "BUF" : "OK"));
    DisplayTextPadded(1U, lineBuf);

    (void)snprintf(lineBuf,
                   sizeof(lineBuf),
                   "M:%s",
                   g_servoRateTestMotionText[st->motionId]);
    DisplayTextPadded(2U, lineBuf);

    (void)snprintf(lineBuf,
                   sizeof(lineBuf),
                   "A:%4d I:%4lu",
                   (int)st->command,
                   (unsigned long)st->servoDbg.PeriodCallbackCount);
    DisplayTextPadded(3U, lineBuf);
    DisplayRefresh();
}

void mode_servo_rate_test(void)
{
    uint32 nextButtonsMs;

    App_InitRuntimeCommon();
    servo_rate_test_enter(Timebase_GetMs());
    nextButtonsMs = Timebase_GetMs();

    for (;;)
    {
        uint32 nowMs = Timebase_GetMs();
        boolean sw2Pressed;
        boolean sw3Pressed;
        uint8 potLevel;

        while (time_reached(nowMs, nextButtonsMs) == TRUE)
        {
            Buttons_Update();
            nextButtonsMs += BUTTONS_PERIOD_MS;
        }

        sw2Pressed = Buttons_WasPressed(BUTTON_ID_SW2);
        sw3Pressed = Buttons_WasPressed(BUTTON_ID_SW3);
        potLevel = OnboardPot_ReadLevelFiltered();

        servo_rate_test_update(nowMs, sw2Pressed, sw3Pressed, potLevel);
    }
}
