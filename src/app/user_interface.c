#include "user_interface.h"

#include "display.h"

/* Menu parameters */
#define DBG_MENU_ITEMS              (7u)
#define DBG_MENU_VISIBLE_LINES      (3u)
#define DBG_POT_STABLE_MS           (80u)
#define DBG_POT_HYSTERESIS          (6u)

static const char g_dbgMenuTitle[] = "DEBUG MENU      "; /* 16 chars */

static const char g_dbgMenuItems[DBG_MENU_ITEMS][17] =
{
    "Ultrasonic      ",
    "Servo Manual    ",
    "ESC Manual      ",
    "Vision Debug    ",
    "Full Auto       ",
    "PID/Speed Tune  ",
    "Settings        "
};

static UiPageType g_dbgPages[DBG_MENU_ITEMS] =
{
    UI_PAGE_ULTRASONIC,
    UI_PAGE_SERVO_MANUAL,
    UI_PAGE_ESC_MANUAL,
    UI_PAGE_VISION_DEBUG,
    UI_PAGE_FULL_AUTO,
    UI_PAGE_FULL_AUTO,   /* tune belongs to full auto */
    UI_PAGE_SETTINGS
};

static uint8  g_dbgSelected  = 0u;
static uint8  g_dbgTopIndex  = 0u;
static uint32 g_dbgLastPotMs = 0u;

/* UI mode */
typedef enum
{
    UI_MODE_MENU = 0,
    UI_MODE_TERMINAL
} UiModeType;

static UiModeType g_uiMode = UI_MODE_MENU;

/* Latest inputs from app_modes.c */
static boolean g_sw2Pressed = FALSE;
static boolean g_sw3Pressed = FALSE; /* reserved for app; UI ignores */
static uint8   g_potValue   = 0u;

/* --- Helpers --- */
static uint8 UI_MapPotToIndex(uint8 pot)
{
    uint16 scaled = (uint16)pot * (uint16)DBG_MENU_ITEMS;
    uint8 idx = (uint8)(scaled / 256u);
    if (idx >= DBG_MENU_ITEMS) { idx = (uint8)(DBG_MENU_ITEMS - 1u); }
    return idx;
}

static void UI_UpdateSelection(uint8 pot, uint32 nowMs)
{
    if ((uint32)(nowMs - g_dbgLastPotMs) < DBG_POT_STABLE_MS)
    {
        return;
    }
    g_dbgLastPotMs = nowMs;

    uint8 newIdx = UI_MapPotToIndex(pot);

    if (newIdx != g_dbgSelected)
    {
        const uint16 regionW = 256u / DBG_MENU_ITEMS;
        uint16 newCenter = (uint16)newIdx * regionW + (regionW / 2u);
        uint16 potU = (uint16)pot;

        uint16 diff = (potU > newCenter) ? (potU - newCenter) : (newCenter - potU);
        if (diff >= DBG_POT_HYSTERESIS)
        {
            g_dbgSelected = newIdx;
        }
    }

    if (g_dbgSelected < g_dbgTopIndex)
    {
        g_dbgTopIndex = g_dbgSelected;
    }
    else if (g_dbgSelected >= (uint8)(g_dbgTopIndex + DBG_MENU_VISIBLE_LINES))
    {
        g_dbgTopIndex = (uint8)(g_dbgSelected - (DBG_MENU_VISIBLE_LINES - 1u));
    }

    if (g_dbgTopIndex > (uint8)(DBG_MENU_ITEMS - DBG_MENU_VISIBLE_LINES))
    {
        g_dbgTopIndex = (uint8)(DBG_MENU_ITEMS - DBG_MENU_VISIBLE_LINES);
    }
}

static void UI_DrawMenu(uint8 pot)
{
    DisplayClear();

    DisplayText(0U, g_dbgMenuTitle, 16U, 0U);
    DisplayValue(0U, (int)pot, 3U, 13U);

    for (uint8 line = 0u; line < DBG_MENU_VISIBLE_LINES; line++)
    {
        uint8 itemIndex = (uint8)(g_dbgTopIndex + line);

        DisplayText((uint8)(1u + line), "                ", 16U, 0U);

        if (itemIndex == g_dbgSelected)
        {
            DisplayText((uint8)(1u + line), ">", 1U, 0U);
        }

        DisplayText((uint8)(1u + line), g_dbgMenuItems[itemIndex], 15U, 1U);
    }

    DisplayRefresh();
}

static void UI_DrawUltrasonicTerminal(Ultrasonic_StatusType st,
                                     uint32 irqCnt,
                                     uint32 ticks,
                                     Braking_StateType brakeState,
                                     float distanceCm)
{
    const char H0[] = "ULTRASONIC      ";
    const char L1[] = "S:     I:      ";
    const char L2[] = "D:     cm      ";
    const char L3[] = "SW2:Back SW3:Go ";

    DisplayClear();
    DisplayText(0U, H0, 16U, 0U);

    DisplayText(1U, L1, 16U, 0U);
    if (st == ULTRA_STATUS_NEW_DATA)      { DisplayText(1U, "NEW  ", 5U, 2U); }
    else if (st == ULTRA_STATUS_BUSY)    { DisplayText(1U, "BUSY ", 5U, 2U); }
    else if (st == ULTRA_STATUS_ERROR)   { DisplayText(1U, "ERR  ", 5U, 2U); }
    else                                 { DisplayText(1U, "IDLE ", 5U, 2U); }
    DisplayValue(1U, (int)irqCnt, 6U, 10U);

    DisplayText(2U, L2, 16U, 0U);
    DisplayValue(2U, (int)(distanceCm + 0.5f), 4U, 2U);

    /* We don’t know your enum labels, so we only show numeric value */
    DisplayText(2U, "B:", 2U, 11U);
    DisplayValue(2U, (int)brakeState, 2U, 13U);

    DisplayText(3U, L3, 16U, 0U);
    DisplayRefresh();
}

static void UI_DrawSimpleTerminal(const char *title)
{
    DisplayClear();

    DisplayText(0U, title, 16U, 0U);
    DisplayText(1U, "SW2: Back       ", 16U, 0U);
    DisplayText(2U, "SW3: Start/Stop ", 16U, 0U);
    DisplayText(3U, "Pot: Adjust     ", 16U, 0U);

    DisplayRefresh();
}

/* --- Public API --- */
void UI_Init(void)
{
    g_dbgSelected  = 0u;
    g_dbgTopIndex  = 0u;
    g_dbgLastPotMs = 0u;
    g_uiMode       = UI_MODE_MENU;

    g_sw2Pressed = FALSE;
    g_sw3Pressed = FALSE;
    g_potValue   = 0u;
}

void UI_Input(boolean sw2Pressed, boolean sw3Pressed, uint8 pot)
{
    g_sw2Pressed = sw2Pressed;
    g_sw3Pressed = sw3Pressed; /* reserved for app logic, UI won’t consume */
    g_potValue   = pot;
}

UiPageType UI_GetSelectedPage(void)
{
    return g_dbgPages[g_dbgSelected];
}

boolean UI_IsInTerminal(void)
{
    return (g_uiMode == UI_MODE_TERMINAL) ? TRUE : FALSE;
}

void UI_Task(Ultrasonic_StatusType ultraStatus,
             uint32 ultraIrqCount,
             uint32 ultraTicks,
             Braking_StateType brakeState,
             float lastDistanceCm)
{
    uint32 nowMs = Timebase_GetMs();

    /* SW2 controls menu <-> terminal ONLY */
    if (g_sw2Pressed == TRUE)
    {
        if (g_uiMode == UI_MODE_MENU) { g_uiMode = UI_MODE_TERMINAL; }
        else                          { g_uiMode = UI_MODE_MENU; }
    }

    if (g_uiMode == UI_MODE_MENU)
    {
        UI_UpdateSelection(g_potValue, nowMs);
        UI_DrawMenu(g_potValue);
    }
    else
    {
        /* Terminal screen depends on selected item */
        switch (g_dbgSelected)
        {
            case 0u:
                UI_DrawUltrasonicTerminal(ultraStatus, ultraIrqCount, ultraTicks, brakeState, lastDistanceCm);
                break;

            case 1u: UI_DrawSimpleTerminal("SERVO MANUAL    "); break;
            case 2u: UI_DrawSimpleTerminal("ESC MANUAL      "); break;
            case 3u: UI_DrawSimpleTerminal("VISION DEBUG    "); break;
            case 4u: UI_DrawSimpleTerminal("FULL AUTO       "); break;
            case 5u: UI_DrawSimpleTerminal("PID/SPEED TUNE  "); break;
            case 6u: UI_DrawSimpleTerminal("SETTINGS        "); break;

            default:
                UI_DrawSimpleTerminal("DEBUG TERMINAL  ");
                break;
        }
    }
}
