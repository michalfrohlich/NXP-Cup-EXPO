#include "../app_internal.h"

sint32 honor_speed_from_distance(boolean hasValidDistance, float distanceCm)
{
    if (hasValidDistance == TRUE)
    {
        if (distanceCm <= HONOR_STOP_DISTANCE_CM)
        {
            return (sint32)HONOR_STOP_SPEED_PCT;
        }

        if (distanceCm <= HONOR_SLOW2_DISTANCE_CM)
        {
            return (sint32)HONOR_SLOW2_SPEED_PCT;
        }

        if (distanceCm <= HONOR_SLOW1_DISTANCE_CM)
        {
            return (sint32)HONOR_SLOW1_SPEED_PCT;
        }
    }

    return (sint32)HONOR_BASE_SPEED_PCT;
}

static void honor_lap_draw_overlay(void)
{
    DisplayTextPadded(3U, "V:     D:      ");
    DisplayValue(3U, (int)g_honorLap.commandedSpeedPct, 3U, 2U);
    DisplayText(3U, "%", 1U, 5U);

    if (g_honorLap.hasValidDistance == TRUE)
    {
        DisplayValue(3U, (int)(g_honorLap.lastDistanceCm + 0.5f), 3U, 10U);
        DisplayText(3U, "cm", 2U, 13U);
    }
    else
    {
        DisplayText(3U, "---cm", 5U, 10U);
    }

    DisplayRefresh();
}

static void honor_lap_enter(uint32 nowMs)
{
    uint32 armStartMs;

    (void)memset(&g_honorLap, 0, sizeof(g_honorLap));

    Esc_Init(ESC_PWM_CH, ESC_SECOND_PWM_CH, ESC_DUTY_MIN, ESC_DUTY_MED, ESC_DUTY_MAX);
    Esc_StopNeutral();

    armStartMs = Timebase_GetMs();
    while ((uint32)(Timebase_GetMs() - armStartMs) < ESC_ARM_TIME_MS)
    {
        Esc_StopNeutral();
    }

    linear_camera_test_enter_common(nowMs, TRUE);
    g_linearCameraTest.ctrl.kp = HONOR_KP;
    g_linearCameraTest.ctrl.kd = HONOR_KD;
    g_linearCameraTest.ctrl.ki = HONOR_KI;
    g_linearCameraTest.ctrl.iTermClamp = ITERM_CLAMP;
    g_linearCameraTest.ctrl.steerScale = HONOR_STEER_SCALE;
    g_linearCameraTest.steeringBaseSpeed = HONOR_FAKE_SPEED;

    Ultrasonic_Init();
    (void)Ultrasonic_GetDistanceCm(&g_honorLap.lastDistanceCm);
    Ultrasonic_StartMeasurement();

    g_honorLap.nextUltraTrigMs = nowMs + HONOR_ULTRA_TRIGGER_PERIOD_MS;
    g_honorLap.nextDisplayMs = nowMs;
    g_honorLap.commandedSpeedPct = 0;

    StatusLed_Cyan();
}

static void honor_lap_update(uint32 nowMs, boolean modeNextPressed, uint8 potLevel)
{
    const uint32 ultraStaleMs =
        ((uint32)HONOR_ULTRA_TRIGGER_PERIOD_MS * 2U) + (uint32)ULTRA_TIMEOUT_MS;
    float distanceCm = 0.0f;
    Ultrasonic_StatusType ultraStatus;

    linear_camera_test_update(nowMs, modeNextPressed, potLevel);

    Ultrasonic_Task();

    if (time_reached(nowMs, g_honorLap.nextUltraTrigMs) == TRUE)
    {
        g_honorLap.nextUltraTrigMs = nowMs + HONOR_ULTRA_TRIGGER_PERIOD_MS;

        if (Ultrasonic_GetStatus() != ULTRA_STATUS_BUSY)
        {
            Ultrasonic_StartMeasurement();
        }
    }

    ultraStatus = Ultrasonic_GetStatus();

    if (Ultrasonic_GetDistanceCm(&distanceCm) == TRUE)
    {
        g_honorLap.lastDistanceCm = distanceCm;
        g_honorLap.lastDistanceMs = nowMs;
        g_honorLap.hasValidDistance = TRUE;
    }
    else if (ultraStatus == ULTRA_STATUS_ERROR)
    {
        g_honorLap.hasValidDistance = FALSE;
    }
    else if ((g_honorLap.hasValidDistance == TRUE) &&
             ((uint32)(nowMs - g_honorLap.lastDistanceMs) > ultraStaleMs))
    {
        g_honorLap.hasValidDistance = FALSE;
    }

    g_honorLap.commandedSpeedPct =
        honor_speed_from_distance(g_honorLap.hasValidDistance, g_honorLap.lastDistanceCm);

    if ((g_linearCameraTest.haveValidVision != TRUE) ||
        (g_linearCameraTest.paused == TRUE) ||
        (g_linearCameraTest.result.status == VISION_TRACK_LOST))
    {
        g_honorLap.commandedSpeedPct = 0;
    }

    if ((g_honorLap.hasValidDistance == TRUE) &&
        (g_honorLap.lastDistanceCm <= (float)HONOR_SLOW1_DISTANCE_CM))
    {
        SteerStraight();
    }

    Esc_SetBrake(0U, 0U);
    Esc_SetLogicalSpeed((int)(-g_honorLap.commandedSpeedPct),
                        (int)(-g_honorLap.commandedSpeedPct));

    if (time_reached(nowMs, g_honorLap.nextDisplayMs) == TRUE)
    {
        g_honorLap.nextDisplayMs = nowMs + DISPLAY_PERIOD_MS;
        honor_lap_draw_overlay();
    }

    if (ultraStatus == ULTRA_STATUS_ERROR)
    {
        StatusLed_Red();
    }
    else if ((g_linearCameraTest.haveValidVision != TRUE) ||
        (g_linearCameraTest.paused == TRUE) ||
        (g_linearCameraTest.result.status == VISION_TRACK_LOST) ||
        (g_honorLap.hasValidDistance != TRUE))
    {
        StatusLed_Yellow();
    }
    else
    {
        StatusLed_Cyan();
    }
}

void mode_honor_lap(void)
{
    uint32 nextButtonsMs;

    App_InitRuntimeCommon();
    honor_lap_enter(Timebase_GetMs());

    nextButtonsMs = Timebase_GetMs();

    for (;;)
    {
        uint32 nowMs = Timebase_GetMs();
        boolean sw3Pressed = FALSE;
        uint8 potLevel;

        while (time_reached(nowMs, nextButtonsMs) == TRUE)
        {
            Buttons_Update();
            nextButtonsMs += BUTTONS_PERIOD_MS;
        }

        (void)Buttons_WasPressed(BUTTON_ID_SW2);
        sw3Pressed = Buttons_WasPressed(BUTTON_ID_SW3);
        potLevel = OnboardPot_ReadLevelFiltered();

        honor_lap_update(nowMs, sw3Pressed, potLevel);
    }
}
