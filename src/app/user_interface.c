#include "user_interface.h"

#include "display.h"
#include "timebase.h"

#define UI_MAIN_ITEMS        (6u)
#define UI_VISIBLE_LINES     (3u)

#define UI_TERM_ITEMS        (2u)
#define UI_TERM_BACK_INDEX   (0u)
#define UI_TERM_VIEW_INDEX   (1u)

#define UI_POT_STABLE_MS     (90u)
#define UI_POT_HYSTERESIS    (6u)

#define UI_SETTINGS_ITEMS    (6u)
#define UI_SET_VISIBLE_LINES (2u)

static const char g_mainItems[UI_MAIN_ITEMS][17] =
{
    "Ultrasonic      ",
    "Servo Manual    ",
    "ESC Manual      ",
    "Vision Debug    ",
    "Full Auto       ",
    "Settings        "
};

static UiPageType g_mainPages[UI_MAIN_ITEMS] =
{
    UI_PAGE_ULTRASONIC,
    UI_PAGE_SERVO_MANUAL,
    UI_PAGE_ESC_MANUAL,
    UI_PAGE_VISION_DEBUG,
    UI_PAGE_FULL_AUTO,
    UI_PAGE_SETTINGS
};

static const char g_termItems[UI_TERM_ITEMS][17] =
{
    "< Back          ",
    "Live View       "
};

typedef enum
{
    UI_MODE_MENU = 0,
    UI_MODE_TERMINAL
} UiModeType;

static UiModeType  g_mode = UI_MODE_MENU;
static UiPageType  g_activePage = UI_PAGE_NONE;

/* selection */
static uint8  g_selected = 0u;
static uint8  g_topIndex = 0u;
static uint32 g_lastPotMs = 0u;

/* terminal */
static uint8  g_termSelected = 0u;

/* settings */
static boolean g_edit = FALSE;
static uint8   g_setSelected = 0u;
static uint8   g_setTop = 0u;

static UiSettingsType g_settings =
{
    .camExposureTicks = 1200u,
    .ultraStopCm      = 20u,
    .baseSpeedPct     = 20u,
    .kp               = 1.20f,
    .kd               = 0.10f,
    .steerScale       = 1.40f
};

/* latest inputs */
static boolean g_sw2 = FALSE;
static boolean g_sw3 = FALSE;
static uint8   g_pot = 0u;

/* run state */
static boolean g_running = FALSE;
static boolean g_fail    = FALSE;

/* telemetry */
static int   g_speedPct = 0;
static float g_lastDistCm = 0.0f;

/* ---------- helpers ---------- */

static uint8 mapPotToIndex(uint8 pot, uint8 nItems)
{
    uint16 scaled = (uint16)pot * (uint16)nItems;
    uint8 idx = (uint8)(scaled / 256u);
    if (idx >= nItems) idx = (uint8)(nItems - 1u);
    return idx;
}

static void potSelectIndex(uint8 pot, uint8 nItems, uint8 *sel, uint8 *top, uint8 visibleLines)
{
    uint32 nowMs = Timebase_GetMs();
    if ((uint32)(nowMs - g_lastPotMs) < UI_POT_STABLE_MS) return;
    g_lastPotMs = nowMs;

    uint8 newIdx = mapPotToIndex(pot, nItems);

    if (newIdx != *sel)
    {
        const uint16 regionW = 256u / nItems;
        uint16 newCenter = (uint16)newIdx * regionW + (regionW / 2u);
        uint16 potU = (uint16)pot;
        uint16 diff = (potU > newCenter) ? (potU - newCenter) : (newCenter - potU);

        if (diff >= UI_POT_HYSTERESIS)
        {
            *sel = newIdx;
        }
    }

    if (*sel < *top) *top = *sel;
    else if (*sel >= (uint8)(*top + visibleLines))
        *top = (uint8)(*sel - (visibleLines - 1u));

    if (*top > (uint8)(nItems - visibleLines))
        *top = (uint8)(nItems - visibleLines);
}

static void drawHeader(const char *title16)
{
    DisplayText(0U, "                ", 16U, 0U);
    DisplayText(0U, title16, 16U, 0U);

    /* keep letters exactly I / R / F */
    if (g_fail) DisplayText(0U, "F", 1U, 15U);
    else if (g_running) DisplayText(0U, "R", 1U, 15U);
    else DisplayText(0U, "I", 1U, 15U);
}

/* RUN HUD: do NOT clear screen (vision is drawn by app_modes) */
static void drawRunHudOverlay(void)
{
    /* bottom row only */
    DisplayText(3U, "                ", 16U, 0U);

    /* Format: "V:xxx% D:yyycm" */
    DisplayText(3U, "V:", 2U, 0U);
    DisplayValue(3U, g_speedPct, 3U, 2U);
    DisplayText(3U, "%", 1U, 5U);

    DisplayText(3U, " D:", 3U, 7U);
    DisplayValue(3U, (int)(g_lastDistCm + 0.5f), 3U, 10U);
    DisplayText(3U, "cm", 2U, 13U);

    DisplayRefresh();
}

static void drawMenu(void)
{
    DisplayClear();
    drawHeader("DEBUG MENU      ");

    for (uint8 line = 0u; line < UI_VISIBLE_LINES; line++)
    {
        uint8 idx = (uint8)(g_topIndex + line);
        uint8 row = (uint8)(1u + line);

        DisplayText(row, "                ", 16U, 0U);
        if (idx == g_selected) DisplayText(row, ">", 1U, 0U);
        DisplayText(row, g_mainItems[idx], 15U, 1U);
    }

    DisplayRefresh();
}

static void drawTerminalList(const char *title16)
{
    DisplayClear();
    drawHeader(title16);

    for (uint8 line = 0u; line < UI_TERM_ITEMS; line++)
    {
        uint8 row = (uint8)(1u + line);
        DisplayText(row, "                ", 16U, 0U);
        if (line == g_termSelected) DisplayText(row, ">", 1U, 0U);
        DisplayText(row, g_termItems[line], 15U, 1U);
    }

    DisplayText(3U, "SW2=OK  SW3=Run ", 16U, 0U);
    DisplayRefresh();
}

static void drawSettings(void)
{
    DisplayClear();
    drawHeader("SETTINGS        ");

    if (g_edit) DisplayText(1U, "EDIT=ON         ", 16U, 0U);
    else        DisplayText(1U, "EDIT=OFF        ", 16U, 0U);

    for (uint8 line = 0u; line < UI_SET_VISIBLE_LINES; line++)
    {
        uint8 idx = (uint8)(g_setTop + line);
        uint8 row = (uint8)(2u + line);

        DisplayText(row, "                ", 16U, 0U);
        if (idx == g_setSelected) DisplayText(row, ">", 1U, 0U);

        switch (idx)
        {
            case 0u:
                DisplayText(row, "Exp:", 4U, 1U);
                DisplayValue(row, (int)g_settings.camExposureTicks, 5U, 6U);
                break;

            case 1u:
                DisplayText(row, "Stop:", 5U, 1U);
                DisplayValue(row, (int)g_settings.ultraStopCm, 3U, 7U);
                DisplayText(row, "cm", 2U, 10U);
                break;

            case 2u:
                DisplayText(row, "Spd:", 4U, 1U);
                DisplayValue(row, (int)g_settings.baseSpeedPct, 3U, 6U);
                DisplayText(row, "%", 1U, 9U);
                break;

            case 3u:
                DisplayText(row, "Kp:", 3U, 1U);
                DisplayValue(row, (int)(g_settings.kp * 100.0f), 4U, 5U);
                DisplayText(row, "x0.01", 6U, 10U);
                break;

            case 4u:
                DisplayText(row, "Kd:", 3U, 1U);
                DisplayValue(row, (int)(g_settings.kd * 100.0f), 4U, 5U);
                DisplayText(row, "x0.01", 6U, 10U);
                break;

            default:
                DisplayText(row, "Scale:", 6U, 1U);
                DisplayValue(row, (int)(g_settings.steerScale * 100.0f), 4U, 8U);
                DisplayText(row, "x0.01", 6U, 12U);
                break;
        }
    }

    DisplayRefresh();
}

/* ---------- public API ---------- */

void UI_Init(void)
{
    g_mode = UI_MODE_MENU;
    g_activePage = UI_PAGE_NONE;

    g_selected = 0u;
    g_topIndex = 0u;
    g_termSelected = 0u;
    g_lastPotMs = 0u;

    g_edit = FALSE;
    g_setSelected = 0u;
    g_setTop = 0u;

    g_sw2 = FALSE;
    g_sw3 = FALSE;
    g_pot = 0u;

    g_running = FALSE;
    g_fail = FALSE;

    g_speedPct = 0;
    g_lastDistCm = 0.0f;
}

void UI_Input(boolean sw2Pressed, boolean sw3Pressed, uint8 pot)
{
    g_sw2 = sw2Pressed;
    g_sw3 = sw3Pressed;
    g_pot = pot;
}

void UI_SetTelemetry(int speedPct)
{
    if (speedPct > 100) speedPct = 100;
    if (speedPct < -100) speedPct = -100;
    g_speedPct = speedPct;
}

boolean UI_IsInTerminal(void)
{
    return (g_mode == UI_MODE_TERMINAL) ? TRUE : FALSE;
}

UiPageType UI_GetActivePage(void)
{
    return g_activePage;
}

const UiSettingsType* UI_GetSettings(void)
{
    return &g_settings;
}

void UI_SetRunState(boolean running, boolean fail)
{
    g_running = running;
    g_fail = fail;
}

void UI_Task(Ultrasonic_StatusType ultraStatus,
             uint32 ultraIrqCount,
             uint32 ultraTicks,
             Braking_StateType brakeState,
             float lastDistanceCm)
{
    (void)ultraStatus;
    (void)ultraIrqCount;
    (void)ultraTicks;
    (void)brakeState;

    g_lastDistCm = lastDistanceCm;

    /* RUN = HUD ONLY (vision drawn by app_modes) */
    if (g_running == TRUE)
    {
        drawRunHudOverlay();
        return;
    }

    /* MENU */
    if (g_mode == UI_MODE_MENU)
    {
        potSelectIndex(g_pot, UI_MAIN_ITEMS, &g_selected, &g_topIndex, UI_VISIBLE_LINES);

        if (g_sw2)
        {
            g_activePage = g_mainPages[g_selected];
            g_mode = UI_MODE_TERMINAL;
            g_termSelected = UI_TERM_BACK_INDEX;

            g_edit = FALSE;
            g_setSelected = 0u;
            g_setTop = 0u;
        }

        drawMenu();
        return;
    }

    /* TERMINAL */
    {
        uint8 dummyTop = 0u;
        potSelectIndex(g_pot, UI_TERM_ITEMS, &g_termSelected, &dummyTop, UI_TERM_ITEMS);
    }

    if (g_sw2)
    {
        if (g_termSelected == UI_TERM_BACK_INDEX)
        {
            g_mode = UI_MODE_MENU;
            g_activePage = UI_PAGE_NONE;
            g_edit = FALSE;
            return;
        }
    }

    if (g_termSelected == UI_TERM_BACK_INDEX)
    {
        switch (g_activePage)
        {
            case UI_PAGE_ULTRASONIC:   drawTerminalList("ULTRASONIC      "); break;
            case UI_PAGE_SERVO_MANUAL: drawTerminalList("SERVO MANUAL    "); break;
            case UI_PAGE_ESC_MANUAL:   drawTerminalList("ESC MANUAL      "); break;
            case UI_PAGE_VISION_DEBUG: drawTerminalList("VISION DEBUG    "); break;
            case UI_PAGE_FULL_AUTO:    drawTerminalList("FULL AUTO       "); break;
            case UI_PAGE_SETTINGS:     drawTerminalList("SETTINGS        "); break;
            default:                   drawTerminalList("TERMINAL        "); break;
        }
        return;
    }

    /* Live view selection: keep simple screens here (vision itself is app-owned) */
    switch (g_activePage)
    {
        case UI_PAGE_SETTINGS:
        default:
        {
            if (!g_edit)
            {
                potSelectIndex(g_pot, UI_SETTINGS_ITEMS, &g_setSelected, &g_setTop, UI_SET_VISIBLE_LINES);
            }

            if (g_sw2)
            {
                g_edit = (g_edit == TRUE) ? FALSE : TRUE;
            }

            if (g_edit)
            {
                switch (g_setSelected)
                {
                    case 0u: g_settings.camExposureTicks = 200u + ((uint32)g_pot * 15u); break;
                    case 1u: g_settings.ultraStopCm = (uint16)(5u + ((uint16)g_pot * 75u) / 255u); break;
                    case 2u: g_settings.baseSpeedPct = (uint8)(((uint16)g_pot * 60u) / 255u); break;
                    case 3u: g_settings.kp = 0.30f + (2.70f * ((float)g_pot / 255.0f)); break;
                    case 4u: g_settings.kd = 0.00f + (0.80f * ((float)g_pot / 255.0f)); break;
                    default: g_settings.steerScale = 0.80f + (1.70f * ((float)g_pot / 255.0f)); break;
                }
            }

            drawSettings();
        } break;
    }
}
