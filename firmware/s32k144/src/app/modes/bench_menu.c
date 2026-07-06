#include "../app_internal.h"

#define BENCH_MENU_BUTTON_CHORD_MS (500U)

typedef struct
{
    uint8 selectedIndex;
    uint8 topIndex;
    uint32 lastPotMs;
    uint32 switchGuardUntilMs;
    boolean modeActive;
    boolean requireSwitchOff;
    BenchModeId_t activeMode;
} BenchMenuState_t;

static BenchMenuState_t g_benchMenu;
static const char g_benchMenuItems[BENCH_MODE_COUNT][17] = {
    "Servo          ", "ESC            ", "Ultrasonic     ", "Ultra+ESC      ", "Camera-s32k    ",
    "Cam+Servo-s32k ", "Drive mode-s32k", "Tune drive mode", "Stack drive    ", "Cable tune-s32k ",
    "ESP Link       ", "Teensy Link    ", "Victory Lap    "};

static uint32 g_benchMenuButtonChordSinceMs = 0U;
static boolean g_benchMenuButtonChordArmed = TRUE;
static boolean g_benchMenuSwitchBypass = FALSE;
static boolean g_benchMenuIgnoreSwitchUntilOff = FALSE;

static boolean bench_menu_button_chord_pressed(uint32 nowMs)
{
    boolean sw2Down = Buttons_IsPressed(BUTTON_ID_SW2);
    boolean sw3Down = Buttons_IsPressed(BUTTON_ID_SW3);

    if ((sw2Down != TRUE) || (sw3Down != TRUE))
    {
        g_benchMenuButtonChordSinceMs = 0U;
        g_benchMenuButtonChordArmed = TRUE;
        return FALSE;
    }

    if (g_benchMenuButtonChordSinceMs == 0U)
    {
        g_benchMenuButtonChordSinceMs = nowMs;
        return FALSE;
    }

    if ((g_benchMenuButtonChordArmed == TRUE) &&
        (time_reached(nowMs, g_benchMenuButtonChordSinceMs + BENCH_MENU_BUTTON_CHORD_MS) == TRUE))
    {
        g_benchMenuButtonChordArmed = FALSE;
        return TRUE;
    }

    return FALSE;
}

static boolean bench_menu_button_chord_is_down(void)
{
    return (boolean)((Buttons_IsPressed(BUTTON_ID_SW2) == TRUE) &&
                     (Buttons_IsPressed(BUTTON_ID_SW3) == TRUE));
}

static void bench_menu_drain_button_events(void)
{
    (void)Buttons_WasPressed(BUTTON_ID_SW2);
    (void)Buttons_WasPressed(BUTTON_ID_SW3);
}

static void bench_mode_enter(BenchModeId_t modeId, uint32 nowMs)
{
    switch (modeId)
    {
        case BENCH_MODE_ESC:
            esc_test_enter(nowMs);
            break;

        case BENCH_MODE_SERVO:
            servo_test_enter(nowMs);
            break;

        case BENCH_MODE_ULTRASONIC:
            ultrasonic_test_enter(nowMs);
            break;

        case BENCH_MODE_CAMSERVO:
            linear_camera_test_enter_common(nowMs, TRUE);
            break;

        case BENCH_MODE_SIMPLE_DRIVE:
            simple_drive_test_enter(nowMs);
            break;

        case BENCH_MODE_TUNE_DRIVE:
            tune_drive_test_enter(nowMs);
            break;

        case BENCH_MODE_STACK_DRIVE:
            stack_drive_test_enter(nowMs);
            break;

        case BENCH_MODE_SERIAL_TUNE:
            serial_tune_test_enter(nowMs);
            break;

        case BENCH_MODE_ULTRA_ESC:
            ultrasonic_esc_test_enter(nowMs);
            break;

        case BENCH_MODE_ESP_LINK:
            esp_link_bench_test_enter(nowMs);
            break;

        case BENCH_MODE_TEENSY_LINK:
            teensy_link_test_enter(nowMs);
            break;

        case BENCH_MODE_VICTORY_LAP:
            victory_lap_test_enter(nowMs);
            break;

        case BENCH_MODE_LINEAR_CAMERA:
        default:
            linear_camera_test_enter(nowMs);
            break;
    }
}

static void bench_mode_update(BenchModeId_t modeId, uint32 nowMs, boolean sw2Pressed,
                                boolean modeNextPressed, uint8 potLevel)
{
    switch (modeId)
    {
        case BENCH_MODE_ESC:
            esc_test_update(nowMs, sw2Pressed, modeNextPressed, potLevel);
            break;

        case BENCH_MODE_SERVO:
            servo_test_update(nowMs, sw2Pressed, potLevel);
            break;

        case BENCH_MODE_ULTRASONIC:
            ultrasonic_test_update(nowMs, sw2Pressed);
            break;

        case BENCH_MODE_CAMSERVO:
            linear_camera_test_update(nowMs, FALSE, potLevel);
            break;

        case BENCH_MODE_SIMPLE_DRIVE:
            simple_drive_test_update(nowMs, sw2Pressed, modeNextPressed);
            break;

        case BENCH_MODE_TUNE_DRIVE:
            tune_drive_test_update(nowMs, modeNextPressed);
            break;

        case BENCH_MODE_STACK_DRIVE:
            stack_drive_test_update(nowMs, sw2Pressed, modeNextPressed);
            break;

        case BENCH_MODE_SERIAL_TUNE:
            serial_tune_test_update();
            break;

        case BENCH_MODE_ULTRA_ESC:
            ultrasonic_esc_test_update(nowMs);
            break;

        case BENCH_MODE_ESP_LINK:
            esp_link_bench_test_update(nowMs);
            break;

        case BENCH_MODE_TEENSY_LINK:
            teensy_link_test_update(nowMs, sw2Pressed);
            break;

        case BENCH_MODE_VICTORY_LAP:
            victory_lap_test_update(nowMs);
            break;

        case BENCH_MODE_LINEAR_CAMERA:
        default:
            linear_camera_test_update(nowMs, modeNextPressed, potLevel);
            break;
    }
}

static void bench_mode_exit(BenchModeId_t modeId)
{
    switch (modeId)
    {
        case BENCH_MODE_ESC:
            esc_test_exit();
            break;

        case BENCH_MODE_SERVO:
            servo_test_exit();
            break;

        case BENCH_MODE_ULTRASONIC:
            ultrasonic_test_exit();
            break;

        case BENCH_MODE_CAMSERVO:
            linear_camera_test_exit();
            break;

        case BENCH_MODE_SIMPLE_DRIVE:
            simple_drive_test_exit();
            break;

        case BENCH_MODE_TUNE_DRIVE:
            tune_drive_test_exit();
            break;

        case BENCH_MODE_STACK_DRIVE:
            stack_drive_test_exit();
            break;

        case BENCH_MODE_SERIAL_TUNE:
            serial_tune_test_exit();
            break;

        case BENCH_MODE_ULTRA_ESC:
            ultrasonic_esc_test_exit();
            break;

        case BENCH_MODE_ESP_LINK:
            esp_link_bench_test_exit();
            break;

        case BENCH_MODE_TEENSY_LINK:
            teensy_link_test_exit();
            break;

        case BENCH_MODE_VICTORY_LAP:
            victory_lap_test_exit();
            break;

        case BENCH_MODE_LINEAR_CAMERA:
        default:
            linear_camera_test_exit();
            break;
    }
}
static uint8 bench_menu_map_pot_to_index(uint8 pot, uint8 nItems)
{
    uint16 scaled = (uint16)pot * (uint16)nItems;
    uint8 idx = (uint8)(scaled / 256u);

    if (idx >= nItems)
    {
        idx = (uint8)(nItems - 1u);
    }

    return idx;
}

static void bench_menu_select_index(BenchMenuState_t *st, uint8 pot)
{
    uint32 nowMs;
    uint8 newIdx;

    if (st == NULL_PTR)
    {
        return;
    }

    nowMs = Timebase_GetMs();
    if ((uint32)(nowMs - st->lastPotMs) < BENCH_MENU_POT_STABLE_MS)
    {
        return;
    }
    st->lastPotMs = nowMs;

    newIdx = bench_menu_map_pot_to_index(pot, (uint8)BENCH_MODE_COUNT);

    if (newIdx != st->selectedIndex)
    {
        uint16 regionW = 256u / (uint16)BENCH_MODE_COUNT;
        uint16 newCenter = (uint16)newIdx * regionW + (regionW / 2u);
        uint16 potU = (uint16)pot;
        uint16 diff = (potU > newCenter) ? (potU - newCenter) : (newCenter - potU);

        if (diff >= BENCH_MENU_POT_HYSTERESIS)
        {
            st->selectedIndex = newIdx;
        }
    }

    if (st->selectedIndex < st->topIndex)
    {
        st->topIndex = st->selectedIndex;
    }
    else if (st->selectedIndex >= (uint8)(st->topIndex + BENCH_MENU_VISIBLE_LINES))
    {
        st->topIndex = (uint8)(st->selectedIndex - (BENCH_MENU_VISIBLE_LINES - 1u));
    }

    if (st->topIndex > (uint8)(BENCH_MODE_COUNT - BENCH_MENU_VISIBLE_LINES))
    {
        st->topIndex = (uint8)(BENCH_MODE_COUNT - BENCH_MENU_VISIBLE_LINES);
    }
}

static void bench_menu_draw(const BenchMenuState_t *st)
{
    if (st == NULL_PTR)
    {
        return;
    }

    DisplayClear();
    DisplayTextPadded(0U, "NXP CUP TESTS");

    for (uint8 line = 0u; line < BENCH_MENU_VISIBLE_LINES; line++)
    {
        uint8 idx = (uint8)(st->topIndex + line);
        uint8 row = (uint8)(1u + line);

        DisplayTextPadded(row, "");
        if (idx == st->selectedIndex)
        {
            DisplayTextPadded(row, ">");
        }
        DisplayText(row, g_benchMenuItems[idx], 15U, 1U);
    }

    Display_Refresh();
    StatusLed_Blue();
}

static void bench_menu_draw_release_switch(void)
{
    DisplayClear();
    DisplayTextPadded(0U, "NXP CUP TESTS");
    DisplayTextPadded(1U, "SWPCB ACTIVE");
    DisplayTextPadded(2U, "MOVE SWITCH");
    DisplayTextPadded(3U, "TO MENU POS");
    Display_Refresh();
    StatusLed_Yellow();
}

void mode_nxp_cup_tests(void)
{
    uint32 nextButtonsMs;

    App_InitRuntimeCommon();
    (void)memset(&g_benchMenu, 0, sizeof(g_benchMenu));
    g_benchMenuButtonChordSinceMs = 0U;
    g_benchMenuButtonChordArmed = TRUE;
    g_benchMenuSwitchBypass = FALSE;
    g_benchMenuIgnoreSwitchUntilOff = FALSE;
    g_benchMenu.activeMode = BENCH_MODE_LINEAR_CAMERA;
    g_benchMenu.requireSwitchOff = TRUE;
    g_benchMenu.switchGuardUntilMs = Timebase_GetMs() + BENCH_MENU_SWITCH_BOOT_GUARD_MS;
    bench_menu_draw_release_switch();

    nextButtonsMs = Timebase_GetMs();

    for (;;)
    {
        uint32 nowMs = Timebase_GetMs();
        boolean sw2Pressed = FALSE;
        boolean sw3Pressed = FALSE;
        boolean modeSwitchOn;
        boolean effectiveModeSwitchOn;
        uint8 potLevel;

        while (time_reached(nowMs, nextButtonsMs) == TRUE)
        {
            Buttons_Update();
            nextButtonsMs += BUTTONS_PERIOD_MS;
        }

        potLevel = OnboardPot_ReadLevelFiltered();

        modeSwitchOn = Buttons_IsOn(BUTTON_ID_SWPCB);
        if ((g_benchMenuIgnoreSwitchUntilOff == TRUE) && (modeSwitchOn != TRUE))
        {
            g_benchMenuIgnoreSwitchUntilOff = FALSE;
        }
        effectiveModeSwitchOn =
            (boolean)((g_benchMenuIgnoreSwitchUntilOff == TRUE) ? FALSE : modeSwitchOn);

        if (g_benchMenu.requireSwitchOff == TRUE)
        {
            boolean buttonChordPressed;

            bench_menu_drain_button_events();
            buttonChordPressed = bench_menu_button_chord_pressed(nowMs);

            if (time_reached(nowMs, g_benchMenu.switchGuardUntilMs) != TRUE)
            {
                continue;
            }

            if ((modeSwitchOn == TRUE) && (buttonChordPressed != TRUE))
            {
                bench_menu_draw_release_switch();
                continue;
            }

            if ((modeSwitchOn == TRUE) && (buttonChordPressed == TRUE))
            {
                g_benchMenuIgnoreSwitchUntilOff = TRUE;
            }

            g_benchMenu.requireSwitchOff = FALSE;
            bench_menu_select_index(&g_benchMenu, potLevel);
            bench_menu_draw(&g_benchMenu);
            continue;
        }

        if (g_benchMenu.modeActive != TRUE)
        {
            boolean buttonChordPressed;

            bench_menu_drain_button_events();
            buttonChordPressed = bench_menu_button_chord_pressed(nowMs);
            bench_menu_select_index(&g_benchMenu, potLevel);
            bench_menu_draw(&g_benchMenu);

            if ((effectiveModeSwitchOn == TRUE) || (buttonChordPressed == TRUE))
            {
                g_benchMenu.activeMode = (BenchModeId_t)g_benchMenu.selectedIndex;
                g_benchMenuSwitchBypass = buttonChordPressed;
                bench_mode_enter(g_benchMenu.activeMode, nowMs);
                g_benchMenu.modeActive = TRUE;
            }
        }
        else
        {
            boolean buttonChordDown = bench_menu_button_chord_is_down();
            boolean buttonChordPressed = bench_menu_button_chord_pressed(nowMs);
            boolean switchExitRequested =
                (boolean)((g_benchMenuSwitchBypass != TRUE) && (effectiveModeSwitchOn != TRUE));

            if ((switchExitRequested == TRUE) || (buttonChordPressed == TRUE))
            {
                bench_mode_exit(g_benchMenu.activeMode);
                g_benchMenu.modeActive = FALSE;
                g_benchMenuSwitchBypass = FALSE;
                bench_menu_drain_button_events();
                bench_menu_draw(&g_benchMenu);
            }
            else
            {
                if (buttonChordDown == TRUE)
                {
                    bench_menu_drain_button_events();
                    sw2Pressed = FALSE;
                    sw3Pressed = FALSE;
                }
                else
                {
                    sw2Pressed = Buttons_WasPressed(BUTTON_ID_SW2);
                    sw3Pressed = Buttons_WasPressed(BUTTON_ID_SW3);
                }
                bench_mode_update(g_benchMenu.activeMode, nowMs, sw2Pressed, sw3Pressed,
                                    potLevel);
            }
        }
    }
}
void mode_linear_camera_test(void)
{
    uint32 nextButtonsMs;

    App_InitRuntimeCommon();
    bench_mode_enter(BENCH_MODE_LINEAR_CAMERA, Timebase_GetMs());

    nextButtonsMs = Timebase_GetMs();

    for (;;)
    {
        uint32 nowMs = Timebase_GetMs();
        boolean sw3Pressed;
        uint8 potLevel;

        while (time_reached(nowMs, nextButtonsMs) == TRUE)
        {
            Buttons_Update();
            nextButtonsMs += BUTTONS_PERIOD_MS;
        }

        sw3Pressed = Buttons_WasPressed(BUTTON_ID_SW3);
        (void)Buttons_WasPressed(BUTTON_ID_SW2);
        potLevel = OnboardPot_ReadLevelFiltered();

        bench_mode_update(BENCH_MODE_LINEAR_CAMERA, nowMs, FALSE, sw3Pressed, potLevel);
    }
}

void mode_esc_test(void)
{
    uint32 nextButtonsMs;

    App_InitRuntimeCommon();
    bench_mode_enter(BENCH_MODE_ESC, Timebase_GetMs());

    nextButtonsMs = Timebase_GetMs();

    for (;;)
    {
        uint32 nowMs = Timebase_GetMs();
        uint8 potLevel;

        while (time_reached(nowMs, nextButtonsMs) == TRUE)
        {
            Buttons_Update();
            nextButtonsMs += BUTTONS_PERIOD_MS;
        }

        potLevel = OnboardPot_ReadLevelFiltered();
        bench_mode_update(BENCH_MODE_ESC, nowMs, FALSE, FALSE, potLevel);
    }
}
