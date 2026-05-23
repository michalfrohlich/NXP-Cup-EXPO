#include "app_internal.h"

static void nxp_cup_idle_motor(void)
{
    Esc_StopNeutral();
}

static void nxp_cup_obstacle_stop_motor(void)
{
    Esc_StopNeutral();
}

static void nxp_cup_launch_motor(int logicalCmd)
{
    EscSetBrake(0U, 0U);
    Esc_SetLogicalSpeed(logicalCmd, logicalCmd);

    for (uint8 pulse = 1U; pulse < (uint8)ESC_LAUNCH_PULSE_COUNT; pulse++)
    {
        busy_delay((uint32)ESC_LAUNCH_PULSE_DELAY_TICKS);
        EscSetBrake(0U, 0U);
        Esc_SetLogicalSpeed(logicalCmd, logicalCmd);
    }
}

static uint8 nxp_cup_profile_from_pot(uint8 pot)
{
    if (pot < 85U)
    {
        return (uint8)NXP_CUP_PROFILE_SUPERFAST;
    }

    if (pot < 171U)
    {
        return (uint8)NXP_CUP_PROFILE_5050;
    }

    return (uint8)NXP_CUP_PROFILE_SLOW;
}

static void nxp_cup_ultra_enter(NxpCupUltraState_t *st, uint32 nowMs)
{
    if (st == NULL_PTR)
    {
        return;
    }

    (void)memset(st, 0, sizeof(*st));
    Ultrasonic_Init();
    st->nextTriggerMs = nowMs;
    st->mode = NXP_CUP_ULTRA_CLEAR;
}

static void nxp_cup_ultra_arm_for_run(NxpCupUltraState_t *st, uint32 nowMs)
{
    if (st == NULL_PTR)
    {
        return;
    }

    st->ultraEnableMs = nowMs + NXP_ULTRA_ENABLE_AFTER_RUN_MS;
}

static boolean nxp_cup_ultra_is_active(const NxpCupUltraState_t *st, uint32 nowMs)
{
    if ((st == NULL_PTR) || (NXP_CUP_ULTRASONIC_ENABLE != 1))
    {
        return FALSE;
    }

    return (boolean)((st->ultraEnableMs != 0U) &&
                     (time_reached(nowMs, st->ultraEnableMs) == TRUE));
}

static boolean nxp_cup_ultra_mode_is_obstacle(const NxpCupUltraState_t *st)
{
    if (st == NULL_PTR)
    {
        return FALSE;
    }

    return (boolean)((st->mode == NXP_CUP_ULTRA_STOP_HOLD) ||
                     (st->mode == NXP_CUP_ULTRA_CRAWL) ||
                     (st->mode == NXP_CUP_ULTRA_CUTOFF));
}

static void nxp_cup_ultra_task(NxpCupUltraState_t *st, uint32 nowMs)
{
    const uint32 ultraStaleMs =
        ((uint32)NXP_CUP_ULTRA_TRIGGER_PERIOD_MS * 2U) + (uint32)ULTRA_TIMEOUT_MS;
    float distanceCm = 0.0f;

    if (st == NULL_PTR)
    {
        return;
    }

    Ultrasonic_Task();

    if (time_reached(nowMs, st->nextTriggerMs) == TRUE)
    {
        st->nextTriggerMs = nowMs + NXP_CUP_ULTRA_TRIGGER_PERIOD_MS;

        if (Ultrasonic_GetStatus() != ULTRA_STATUS_BUSY)
        {
            Ultrasonic_StartMeasurement();
        }
    }

    if (Ultrasonic_GetDistanceCm(&distanceCm) == TRUE)
    {
        st->lastDistanceCm = distanceCm;
        st->lastDistanceMs = nowMs;
        st->haveValidDistance = (boolean)(distanceCm > 0.0f);
    }
    else if ((st->haveValidDistance == TRUE) &&
             ((uint32)(nowMs - st->lastDistanceMs) > ultraStaleMs))
    {
        st->haveValidDistance = FALSE;
    }

    if (nxp_cup_ultra_is_active(st, nowMs) != TRUE)
    {
        st->mode = NXP_CUP_ULTRA_CLEAR;
        st->modeUntilMs = 0U;
        return;
    }

    if (st->haveValidDistance != TRUE)
    {
        return;
    }

    if (st->lastDistanceCm <= (float)NXP_CUP_ULTRA_CRAWL_STOP_CM)
    {
        st->mode = NXP_CUP_ULTRA_CUTOFF;
        st->modeUntilMs = 0U;
        return;
    }

    switch (st->mode)
    {
        case NXP_CUP_ULTRA_STOP_HOLD:
            if ((st->modeUntilMs != 0U) &&
                (time_reached(nowMs, st->modeUntilMs) == TRUE))
            {
                st->mode = (st->lastDistanceCm <= (float)NXP_CUP_ULTRA_STOP_CM) ?
                    NXP_CUP_ULTRA_CRAWL : NXP_CUP_ULTRA_CLEAR;
                st->modeUntilMs = 0U;
            }
            break;

        case NXP_CUP_ULTRA_CRAWL:
            if (st->lastDistanceCm > (float)NXP_CUP_ULTRA_STOP_CM)
            {
                st->mode = NXP_CUP_ULTRA_CLEAR;
            }
            break;

        case NXP_CUP_ULTRA_CUTOFF:
            break;

        case NXP_CUP_ULTRA_CLEAR:
        default:
            if (st->lastDistanceCm <= (float)NXP_CUP_ULTRA_STOP_CM)
            {
                st->mode = NXP_CUP_ULTRA_STOP_HOLD;
                st->modeUntilMs = nowMs + NXP_CUP_ULTRA_STOP_HOLD_MS;
            }
            break;
    }
}

static boolean nxp_cup_ultra_should_hold_servo(const NxpCupUltraState_t *st, uint32 nowMs)
{
    (void)nowMs;
    return nxp_cup_ultra_mode_is_obstacle(st);
}

static uint8 nxp_cup_ultra_target_speed_pct(const NxpCupUltraState_t *st, uint32 nowMs, uint8 defaultSpeedPct)
{
    if ((st == NULL_PTR) || (nxp_cup_ultra_is_active(st, nowMs) != TRUE))
    {
        return defaultSpeedPct;
    }

    switch (st->mode)
    {
        case NXP_CUP_ULTRA_STOP_HOLD:
        case NXP_CUP_ULTRA_CUTOFF:
            return 0U;

        case NXP_CUP_ULTRA_CRAWL:
            return (uint8)((NXP_CUP_ULTRA_CRAWL_LOGICAL_CMD < 0) ?
                (-NXP_CUP_ULTRA_CRAWL_LOGICAL_CMD) :
                NXP_CUP_ULTRA_CRAWL_LOGICAL_CMD);

        case NXP_CUP_ULTRA_CLEAR:
        default:
            return defaultSpeedPct;
    }
}

void mode_nxp_cup(void)
{
    uint32 nextButtonsMs;

    App_InitRuntimeCommon();
    (void)memset(&g_nxpCup, 0, sizeof(g_nxpCup));

    g_nxpCup.state = NXP_CUP_STATE_MENU;
    g_nxpCup.profileId = (NxpCupProfileId_t)NXP_CUP_DEFAULT_PROFILE;
    g_nxpCup.nextDisplayMs = Timebase_GetMs();
    nextButtonsMs = Timebase_GetMs();

    nxp_cup_idle_motor();
    nxp_cup_ultra_enter(&g_nxpCup.ultraSt, Timebase_GetMs());
    StatusLed_Blue();

    for (;;)
    {
        uint32 nowMs = Timebase_GetMs();
        boolean sw2Pressed;
        boolean sw3Pressed;
        boolean systemBad = FALSE;
        uint8 potLevel;

        while (time_reached(nowMs, nextButtonsMs) == TRUE)
        {
            Buttons_Update();
            nextButtonsMs += BUTTONS_PERIOD_MS;
        }

        sw2Pressed = Buttons_WasPressed(BUTTON_ID_SW2);
        sw3Pressed = Buttons_WasPressed(BUTTON_ID_SW3);
        potLevel = OnboardPot_ReadLevelFiltered();

        if ((g_nxpCup.state == NXP_CUP_STATE_RUN) &&
            (g_nxpCup.cameraStarted == TRUE) &&
            (nxp_cup_ultra_mode_is_obstacle(&g_nxpCup.ultraSt) != TRUE) &&
            ((g_nxpCup.camSt.haveValidVision != TRUE) ||
             ((uint32)(nowMs - g_nxpCup.camSt.lastFrameMs) > CAM_STEER_HOLD_MS) ||
             (g_nxpCup.camSt.result.status == VISION_TRACK_LOST)))
        {
            systemBad = TRUE;
        }

        if ((g_nxpCup.state == NXP_CUP_STATE_RUN) &&
            ((g_nxpCup.ultraSt.mode == NXP_CUP_ULTRA_STOP_HOLD) ||
             (g_nxpCup.ultraSt.mode == NXP_CUP_ULTRA_CUTOFF)))
        {
            StatusLed_Red();
        }
        else if ((g_nxpCup.state == NXP_CUP_STATE_RUN) &&
                 (g_nxpCup.ultraSt.mode == NXP_CUP_ULTRA_CRAWL))
        {
            StatusLed_Yellow();
        }
        else if (systemBad == TRUE)
        {
            StatusLed_Red();
            g_nxpCup.autoSpeedPct = 0;
            nxp_cup_idle_motor();
        }
        else if ((g_nxpCup.state == NXP_CUP_STATE_RUN) ||
                 (g_nxpCup.state == NXP_CUP_STATE_ESC_REARM))
        {
            StatusLed_Green();
        }
        else
        {
            StatusLed_Blue();
        }

        if (time_reached(nowMs, g_nxpCup.nextDisplayMs) == TRUE)
        {
            g_nxpCup.nextDisplayMs = nowMs + DISPLAY_PERIOD_MS;
            DisplayClear();

            switch (g_nxpCup.state)
            {
                case NXP_CUP_STATE_MENU:
                    g_nxpCup.profileId = (NxpCupProfileId_t)nxp_cup_profile_from_pot(potLevel);
                    DisplayTextPadded(0U, "NXP CUP MENU");
                    if (g_nxpCup.profileId == NXP_CUP_PROFILE_SUPERFAST)
                    {
                        DisplayTextPadded(1U, ">SUPERFAST");
                        DisplayTextPadded(2U, " 5050");
                        DisplayTextPadded(3U, " SLOW SW2 ENTER");
                    }
                    else if (g_nxpCup.profileId == NXP_CUP_PROFILE_5050)
                    {
                        DisplayTextPadded(1U, " SUPERFAST");
                        DisplayTextPadded(2U, ">5050");
                        DisplayTextPadded(3U, " SLOW SW2 ENTER");
                    }
                    else
                    {
                        DisplayTextPadded(1U, " SUPERFAST");
                        DisplayTextPadded(2U, " 5050");
                        DisplayTextPadded(3U, ">SLOW SW2 ENTER");
                    }
                    break;

                case NXP_CUP_STATE_READY:
                    DisplayTextPadded(0U, "NXP CUP READY");
                    if (g_nxpCup.profileId == NXP_CUP_PROFILE_SUPERFAST)
                    {
                        DisplayTextPadded(1U, "PROFILE:SUPER");
                    }
                    else if (g_nxpCup.profileId == NXP_CUP_PROFILE_5050)
                    {
                        DisplayTextPadded(1U, "PROFILE:5050");
                    }
                    else
                    {
                        DisplayTextPadded(1U, "PROFILE:SLOW");
                    }
                    DisplayTextPadded(2U, "SW3 START");
                    DisplayTextPadded(3U, "SW2 BACK");
                    break;

                case NXP_CUP_STATE_ESC_REARM:
                    DisplayTextPadded(0U, "NXP CUP ARM");
                    DisplayTextPadded(1U, "ESC re-arming");
                    DisplayTextPadded(2U, "SW3 ready");
                    DisplayTextPadded(3U, "SW2 menu");
                    break;

                case NXP_CUP_STATE_RUN:
                default:
                    DisplayTextPadded(0U, "NXP CUP RUN");
                    DisplayTextPadded(1U, "SPD:    D:--- ");
                    DisplayValue(1U, (int)g_nxpCup.autoSpeedPct, 3U, 4U);
                    if (g_nxpCup.ultraSt.haveValidDistance == TRUE)
                    {
                        DisplayValue(1U, (int)(g_nxpCup.ultraSt.lastDistanceCm + 0.5f), 3U, 11U);
                    }

                    if (systemBad == TRUE)
                    {
                        DisplayTextPadded(2U, "MODE: FAULT");
                    }
                    else if (g_nxpCup.ultraSt.mode == NXP_CUP_ULTRA_STOP_HOLD)
                    {
                        DisplayTextPadded(2U, "MODE: STOP");
                    }
                    else if (g_nxpCup.ultraSt.mode == NXP_CUP_ULTRA_CRAWL)
                    {
                        DisplayTextPadded(2U, "MODE: CRAWL");
                    }
                    else if (g_nxpCup.ultraSt.mode == NXP_CUP_ULTRA_CUTOFF)
                    {
                        DisplayTextPadded(2U, "MODE: CUTOFF");
                    }
                    else if ((g_nxpCup.camSt.haveValidVision == TRUE) &&
                             (g_nxpCup.camSt.result.status != VISION_TRACK_LOST))
                    {
                        DisplayTextPadded(2U, "MODE: TRACK");
                    }
                    else
                    {
                        DisplayTextPadded(2U, "MODE: WAIT");
                    }

                    DisplayTextPadded(3U, "SW3 READY SW2 M");
                    break;
            }

            DisplayRefresh();
        }

        if (g_nxpCup.state == NXP_CUP_STATE_MENU)
        {
            if (g_nxpCup.cameraStarted == TRUE)
            {
                camservo_exit();
                g_nxpCup.cameraStarted = FALSE;
            }

            nxp_cup_idle_motor();
            SteerStraight();
            g_nxpCup.autoSpeedPct = 0;
            g_nxpCup.nextAutoSpeedMs = 0U;
            g_nxpCup.escRearmDoneMs = 0U;

            if (sw2Pressed == TRUE)
            {
                g_nxpCup.state = NXP_CUP_STATE_READY;
            }

            continue;
        }

        if (g_nxpCup.state == NXP_CUP_STATE_READY)
        {
            if (g_nxpCup.cameraStarted == TRUE)
            {
                camservo_exit();
                g_nxpCup.cameraStarted = FALSE;
            }

            nxp_cup_idle_motor();
            SteerStraight();
            g_nxpCup.autoSpeedPct = 0;
            g_nxpCup.nextAutoSpeedMs = 0U;
            g_nxpCup.escRearmDoneMs = 0U;

            if (sw2Pressed == TRUE)
            {
                g_nxpCup.state = NXP_CUP_STATE_MENU;
                continue;
            }

            if (sw3Pressed == TRUE)
            {
                camservo_enter_with_profile(&g_nxpCup.camSt,
                                            nowMs,
                                            &g_nxpCupProfiles[g_nxpCup.profileId]);
                g_nxpCup.cameraStarted = TRUE;
                g_nxpCup.autoSpeedPct = 0;
                g_nxpCup.nextAutoSpeedMs = 0U;

                nxp_cup_ultra_enter(&g_nxpCup.ultraSt, nowMs);
                EscInit(ESC_PWM_CH, ESC_SECOND_PWM_CH, ESC_DUTY_MIN, ESC_DUTY_MED, ESC_DUTY_MAX);
                g_nxpCup.escRearmDoneMs = nowMs + ESC_ARM_TIME_MS + NXP_ESC_EXTRA_SETTLE_MS;
                g_nxpCup.state = NXP_CUP_STATE_ESC_REARM;
                continue;
            }

            continue;
        }

        if (g_nxpCup.state == NXP_CUP_STATE_ESC_REARM)
        {
            nxp_cup_ultra_task(&g_nxpCup.ultraSt, nowMs);

            if (g_nxpCup.cameraStarted == TRUE)
            {
                camservo_update(&g_nxpCup.camSt, nowMs, FALSE);
            }

            if (sw2Pressed == TRUE)
            {
                nxp_cup_idle_motor();
                if (g_nxpCup.cameraStarted == TRUE)
                {
                    camservo_exit();
                    g_nxpCup.cameraStarted = FALSE;
                }
                SteerStraight();
                g_nxpCup.autoSpeedPct = 0;
                g_nxpCup.nextAutoSpeedMs = 0U;
                g_nxpCup.escRearmDoneMs = 0U;
                g_nxpCup.state = NXP_CUP_STATE_MENU;
                continue;
            }

            if (sw3Pressed == TRUE)
            {
                nxp_cup_idle_motor();
                if (g_nxpCup.cameraStarted == TRUE)
                {
                    camservo_exit();
                    g_nxpCup.cameraStarted = FALSE;
                }
                SteerStraight();
                g_nxpCup.autoSpeedPct = 0;
                g_nxpCup.nextAutoSpeedMs = 0U;
                g_nxpCup.escRearmDoneMs = 0U;
                g_nxpCup.state = NXP_CUP_STATE_READY;
                continue;
            }

            nxp_cup_idle_motor();

            if (time_reached(nowMs, g_nxpCup.escRearmDoneMs) == TRUE)
            {
                int logicalCmd;

                g_nxpCup.autoSpeedPct = (sint32)g_nxpCup.camSt.activeTune.baseSpeedPct;
                if (g_nxpCup.autoSpeedPct < 0)
                {
                    g_nxpCup.autoSpeedPct = 0;
                }
                if (g_nxpCup.autoSpeedPct > 100)
                {
                    g_nxpCup.autoSpeedPct = 100;
                }

                nxp_cup_ultra_arm_for_run(&g_nxpCup.ultraSt, nowMs);
                nxp_cup_ultra_task(&g_nxpCup.ultraSt, nowMs);

                if ((g_nxpCup.ultraSt.mode == NXP_CUP_ULTRA_STOP_HOLD) ||
                    (g_nxpCup.ultraSt.mode == NXP_CUP_ULTRA_CUTOFF))
                {
                    g_nxpCup.autoSpeedPct = 0;
                    nxp_cup_obstacle_stop_motor();
                    SteerStraight();
                }
                else
                {
                    if (g_nxpCup.ultraSt.mode == NXP_CUP_ULTRA_CRAWL)
                    {
                        logicalCmd = (int)NXP_CUP_ULTRA_CRAWL_LOGICAL_CMD;
                        g_nxpCup.autoSpeedPct = (logicalCmd < 0) ? -logicalCmd : logicalCmd;
                    }
                    else
                    {
                        g_nxpCup.autoSpeedPct =
                            (sint32)nxp_cup_ultra_target_speed_pct(&g_nxpCup.ultraSt,
                                                                   nowMs,
                                                                   g_nxpCup.camSt.activeTune.baseSpeedPct);
                        logicalCmd = (int)(-g_nxpCup.autoSpeedPct);
                    }

                    nxp_cup_launch_motor(logicalCmd);
                }

                g_nxpCup.nextAutoSpeedMs = nowMs + FULL_AUTO_RAMP_PERIOD_MS;
                g_nxpCup.state = NXP_CUP_STATE_RUN;
            }

            continue;
        }

        nxp_cup_ultra_task(&g_nxpCup.ultraSt, nowMs);

        if (g_nxpCup.cameraStarted == TRUE)
        {
            if (nxp_cup_ultra_should_hold_servo(&g_nxpCup.ultraSt, nowMs) == TRUE)
            {
                SteerStraight();
            }
            else
            {
                camservo_update(&g_nxpCup.camSt, nowMs, FALSE);
            }
        }

        if ((g_nxpCup.ultraSt.mode == NXP_CUP_ULTRA_STOP_HOLD) ||
            (g_nxpCup.ultraSt.mode == NXP_CUP_ULTRA_CUTOFF))
        {
            g_nxpCup.autoSpeedPct = 0;
            nxp_cup_obstacle_stop_motor();
        }
        else if (g_nxpCup.ultraSt.mode == NXP_CUP_ULTRA_CRAWL)
        {
            int crawlCmd = (int)NXP_CUP_ULTRA_CRAWL_LOGICAL_CMD;

            g_nxpCup.autoSpeedPct = (crawlCmd < 0) ? -crawlCmd : crawlCmd;
            nxp_cup_launch_motor(crawlCmd);
        }

        if (sw2Pressed == TRUE)
        {
            nxp_cup_idle_motor();
            if (g_nxpCup.cameraStarted == TRUE)
            {
                camservo_exit();
                g_nxpCup.cameraStarted = FALSE;
            }
            SteerStraight();
            g_nxpCup.autoSpeedPct = 0;
            g_nxpCup.nextAutoSpeedMs = 0U;
            g_nxpCup.escRearmDoneMs = 0U;
            g_nxpCup.state = NXP_CUP_STATE_MENU;
            continue;
        }

        if (sw3Pressed == TRUE)
        {
            nxp_cup_idle_motor();
            if (g_nxpCup.cameraStarted == TRUE)
            {
                camservo_exit();
                g_nxpCup.cameraStarted = FALSE;
            }
            SteerStraight();
            g_nxpCup.autoSpeedPct = 0;
            g_nxpCup.nextAutoSpeedMs = 0U;
            g_nxpCup.escRearmDoneMs = 0U;
            g_nxpCup.state = NXP_CUP_STATE_READY;
            continue;
        }

        if (time_reached(nowMs, g_nxpCup.nextAutoSpeedMs) == TRUE)
        {
            sint32 targetSpeedPct;
            int logicalCmd;

            g_nxpCup.nextAutoSpeedMs = nowMs + FULL_AUTO_RAMP_PERIOD_MS;
            targetSpeedPct =
                (sint32)nxp_cup_ultra_target_speed_pct(&g_nxpCup.ultraSt,
                                                       nowMs,
                                                       g_nxpCup.camSt.activeTune.baseSpeedPct);

            if (g_nxpCup.ultraSt.mode == NXP_CUP_ULTRA_CRAWL)
            {
                logicalCmd = (int)NXP_CUP_ULTRA_CRAWL_LOGICAL_CMD;
                g_nxpCup.autoSpeedPct = (logicalCmd < 0) ? -logicalCmd : logicalCmd;
            }
            else if (g_nxpCup.autoSpeedPct < targetSpeedPct)
            {
                g_nxpCup.autoSpeedPct += (sint32)FULL_AUTO_RAMP_STEP_PCT;
                if (g_nxpCup.autoSpeedPct > targetSpeedPct)
                {
                    g_nxpCup.autoSpeedPct = targetSpeedPct;
                }
                logicalCmd = (int)(-g_nxpCup.autoSpeedPct);
            }
            else if (g_nxpCup.autoSpeedPct > targetSpeedPct)
            {
                g_nxpCup.autoSpeedPct -= (sint32)NXP_CUP_RAMP_DOWN_STEP_PCT;
                if (g_nxpCup.autoSpeedPct < targetSpeedPct)
                {
                    g_nxpCup.autoSpeedPct = targetSpeedPct;
                }
                logicalCmd = (int)(-g_nxpCup.autoSpeedPct);
            }
            else
            {
                logicalCmd = (int)(-g_nxpCup.autoSpeedPct);
            }

            if (g_nxpCup.autoSpeedPct < 0)
            {
                g_nxpCup.autoSpeedPct = 0;
            }
            if (g_nxpCup.autoSpeedPct > 100)
            {
                g_nxpCup.autoSpeedPct = 100;
            }

            nxp_cup_launch_motor(logicalCmd);
        }
    }
}
