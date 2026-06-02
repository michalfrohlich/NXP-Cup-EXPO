#include "../app_internal.h"

/* Mario-style victory fanfare target notes for a future motor-tone driver. */
static const VictoryLapNote_t g_victoryLapFanfare[] =
{
    { 392U, 80U, 10U },    /* G4 */
    { 523U, 80U, 10U },    /* C5 */
    { 659U, 80U, 10U },    /* E5 */
    { 784U, 80U, 10U },    /* G5 */
    { 1047U, 80U, 10U },   /* C6 */
    { 1319U, 80U, 10U },   /* E6 */
    { 1568U, 120U, 25U },  /* G6 */
    { 1319U, 120U, 35U },  /* E6 */
    { 415U, 80U, 10U },    /* G#4 */
    { 523U, 80U, 10U },    /* C5 */
    { 622U, 80U, 10U },    /* D#5 */
    { 831U, 80U, 10U },    /* G#5 */
    { 1047U, 80U, 10U },   /* C6 */
    { 1245U, 80U, 10U },   /* D#6 */
    { 1661U, 120U, 25U },  /* G#6 */
    { 1245U, 120U, 35U },  /* D#6 */
    { 466U, 80U, 10U },    /* A#4 */
    { 587U, 80U, 10U },    /* D5 */
    { 698U, 80U, 10U },    /* F5 */
    { 932U, 80U, 10U },    /* A#5 */
    { 1175U, 80U, 10U },   /* D6 */
    { 1397U, 80U, 10U },   /* F6 */
    { 1865U, 140U, 25U },  /* A#6 */
    { 1865U, 240U, 0U }    /* A#6 */
};

static void victory_lap_start_note(uint32 nowMs)
{
    const VictoryLapNote_t *note;

    if (g_victoryLapTest.noteIndex >=
        (uint8)(sizeof(g_victoryLapFanfare) / sizeof(g_victoryLapFanfare[0])))
    {
        g_victoryLapTest.phase = VICTORY_LAP_PHASE_DONE;
        g_victoryLapTest.currentNoteHz = 0U;
        return;
    }

    note = &g_victoryLapFanfare[g_victoryLapTest.noteIndex];
    g_victoryLapTest.currentNoteHz = note->hz;
    g_victoryLapTest.noteUntilMs = nowMs + (uint32)note->durationMs;
    g_victoryLapTest.phase = VICTORY_LAP_PHASE_NOTE;
}

static void victory_lap_update_distance(uint32 nowMs)
{
    const uint32 ultraStaleMs =
        ((uint32)ULTRA_TRIGGER_PERIOD_MS * 2U) + (uint32)ULTRA_TIMEOUT_MS;
    float distanceCm = 0.0f;
    Ultrasonic_StatusType st;

    Ultrasonic_Task();

    if (time_reached(nowMs, g_victoryLapTest.nextUltraTrigMs) == TRUE)
    {
        g_victoryLapTest.nextUltraTrigMs = nowMs + ULTRA_TRIGGER_PERIOD_MS;

        if (Ultrasonic_GetStatus() != ULTRA_STATUS_BUSY)
        {
            Ultrasonic_StartMeasurement();
        }
    }

    st = Ultrasonic_GetStatus();

    if (Ultrasonic_GetDistanceCm(&distanceCm) == TRUE)
    {
        g_victoryLapTest.lastDistanceCm = distanceCm;
        g_victoryLapTest.lastDistanceMs = nowMs;
        g_victoryLapTest.hasValidDistance = TRUE;
    }
    else if (st == ULTRA_STATUS_ERROR)
    {
        g_victoryLapTest.hasValidDistance = FALSE;
    }
    else if ((g_victoryLapTest.hasValidDistance == TRUE) &&
             ((uint32)(nowMs - g_victoryLapTest.lastDistanceMs) > ultraStaleMs))
    {
        g_victoryLapTest.hasValidDistance = FALSE;
    }
}

static void victory_lap_apply_note_led(uint16 hz)
{
    if (hz >= 1000U)
    {
        StatusLed_Cyan();
    }
    else if (hz >= 700U)
    {
        StatusLed_Green();
    }
    else
    {
        StatusLed_Yellow();
    }
}

static void victory_lap_update_fanfare(uint32 nowMs)
{
    const VictoryLapNote_t *note;

    Esc_StopNeutral();
    SteerStraight();

    if (g_victoryLapTest.phase == VICTORY_LAP_PHASE_NOTE)
    {
        victory_lap_apply_note_led(g_victoryLapTest.currentNoteHz);

        if (time_reached(nowMs, g_victoryLapTest.noteUntilMs) == TRUE)
        {
            note = &g_victoryLapFanfare[g_victoryLapTest.noteIndex];
            g_victoryLapTest.currentNoteHz = 0U;
            g_victoryLapTest.noteUntilMs = nowMs + (uint32)note->gapMs;
            g_victoryLapTest.phase = VICTORY_LAP_PHASE_GAP;
            StatusLed_Off();
        }
    }
    else if (g_victoryLapTest.phase == VICTORY_LAP_PHASE_GAP)
    {
        if (time_reached(nowMs, g_victoryLapTest.noteUntilMs) == TRUE)
        {
            g_victoryLapTest.noteIndex++;
            victory_lap_start_note(nowMs);
        }
    }
}

static void victory_lap_draw(uint32 nowMs)
{
    const char *phaseText = "WAIT";

    if (time_reached(nowMs, g_victoryLapTest.nextDisplayMs) != TRUE)
    {
        return;
    }

    g_victoryLapTest.nextDisplayMs = nowMs + VICTORY_LAP_DISPLAY_MS;

    switch (g_victoryLapTest.phase)
    {
        case VICTORY_LAP_PHASE_ARM:      phaseText = "ARM "; break;
        case VICTORY_LAP_PHASE_APPROACH: phaseText = "GO  "; break;
        case VICTORY_LAP_PHASE_NOTE:     phaseText = "NOTE"; break;
        case VICTORY_LAP_PHASE_GAP:      phaseText = "GAP "; break;
        case VICTORY_LAP_PHASE_DONE:     phaseText = "DONE"; break;
        default:                         break;
    }

    DisplayTextPadded(0U, "VICTORY LAP");
    DisplayTextPadded(1U, "    D: ---cm");
    DisplayText(1U, phaseText, 4U, 0U);

    if (g_victoryLapTest.hasValidDistance == TRUE)
    {
        DisplayValue(1U, (int)(g_victoryLapTest.lastDistanceCm + 0.5f), 3U, 8U);
        DisplayText(1U, "cm", 2U, 11U);
    }

    DisplayTextPadded(2U, "SPD:   0 NOTE:");
    DisplayValue(2U, (int)g_victoryLapTest.commandedSpeedPct, 3U, 4U);
    DisplayValue(2U, (int)g_victoryLapTest.noteIndex, 2U, 14U);

    DisplayTextPadded(3U, "HZ:    POLE: 8");
    if (g_victoryLapTest.currentNoteHz != 0U)
    {
        DisplayValue(3U, (int)g_victoryLapTest.currentNoteHz, 4U, 3U);
    }

    DisplayRefresh();
}

void victory_lap_test_enter(uint32 nowMs)
{
    (void)memset(&g_victoryLapTest, 0, sizeof(g_victoryLapTest));

    Esc_Init(ESC_PWM_CH, ESC_SECOND_PWM_CH, ESC_DUTY_MIN, ESC_DUTY_MED, ESC_DUTY_MAX);
    Esc_StopNeutral();

    Servo_Init(SERVO_PWM_CH, SERVO_DUTY_MAX, SERVO_DUTY_MIN, SERVO_DUTY_MED);
    SteerStraight();

    Ultrasonic_Init();

    g_victoryLapTest.phase = VICTORY_LAP_PHASE_ARM;
    g_victoryLapTest.armDoneMs = nowMs + ESC_ARM_TIME_MS;
    g_victoryLapTest.nextUltraTrigMs = nowMs;
    g_victoryLapTest.nextDisplayMs = nowMs;
    g_victoryLapTest.lastDistanceMs = nowMs;

    DisplayTextPadded(0U, "VICTORY LAP");
    DisplayTextPadded(1U, "ARMING ESC");
    DisplayTextPadded(2U, "POLE AT 8cm");
    DisplayTextPadded(3U, "MARIO FANFARE");
    DisplayRefresh();
    StatusLed_Blue();
}

void victory_lap_test_update(uint32 nowMs)
{
    victory_lap_update_distance(nowMs);

    if (g_victoryLapTest.phase == VICTORY_LAP_PHASE_ARM)
    {
        g_victoryLapTest.commandedSpeedPct = 0;
        Esc_StopNeutral();
        SteerStraight();
        StatusLed_Blue();

        if (time_reached(nowMs, g_victoryLapTest.armDoneMs) == TRUE)
        {
            g_victoryLapTest.phase = VICTORY_LAP_PHASE_APPROACH;
        }
    }
    else if (g_victoryLapTest.phase == VICTORY_LAP_PHASE_APPROACH)
    {
        SteerStraight();

        if ((g_victoryLapTest.hasValidDistance == TRUE) &&
            (g_victoryLapTest.lastDistanceCm <= (float)VICTORY_LAP_POLE_CM))
        {
            g_victoryLapTest.commandedSpeedPct = 0;
            Esc_StopNeutral();
            victory_lap_start_note(nowMs);
        }
        else if (g_victoryLapTest.hasValidDistance == TRUE)
        {
            g_victoryLapTest.commandedSpeedPct = (sint32)VICTORY_LAP_APPROACH_SPEED_PCT;
            Esc_SetBrake(0U, 0U);
            Esc_SetLogicalSpeed((int)(-g_victoryLapTest.commandedSpeedPct),
                                (int)(-g_victoryLapTest.commandedSpeedPct));
            StatusLed_Green();
        }
        else
        {
            g_victoryLapTest.commandedSpeedPct = 0;
            Esc_StopNeutral();
            StatusLed_Yellow();
        }
    }
    else if ((g_victoryLapTest.phase == VICTORY_LAP_PHASE_NOTE) ||
             (g_victoryLapTest.phase == VICTORY_LAP_PHASE_GAP))
    {
        victory_lap_update_fanfare(nowMs);
    }
    else
    {
        g_victoryLapTest.commandedSpeedPct = 0;
        Esc_StopNeutral();
        SteerStraight();
        StatusLed_Green();
    }

    victory_lap_draw(nowMs);
}

void victory_lap_test_exit(void)
{
    Esc_StopNeutral();
    SteerStraight();
    StatusLed_Off();
}
