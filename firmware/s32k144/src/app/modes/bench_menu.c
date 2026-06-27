#include "../app_internal.h"

#define TESTS_MENU_BUTTON_CHORD_MS (500U)

static uint32 g_testsMenuButtonChordSinceMs = 0U;
static boolean g_testsMenuButtonChordArmed = TRUE;
static boolean g_testsMenuSwitchBypass = FALSE;
static boolean g_testsMenuIgnoreSwitchUntilOff = FALSE;

static boolean tests_menu_button_chord_pressed(uint32 nowMs)
{
    boolean sw2Down = Buttons_IsPressed(BUTTON_ID_SW2);
    boolean sw3Down = Buttons_IsPressed(BUTTON_ID_SW3);

    if ((sw2Down != TRUE) || (sw3Down != TRUE))
    {
        g_testsMenuButtonChordSinceMs = 0U;
        g_testsMenuButtonChordArmed = TRUE;
        return FALSE;
    }

    if (g_testsMenuButtonChordSinceMs == 0U)
    {
        g_testsMenuButtonChordSinceMs = nowMs;
        return FALSE;
    }

    if ((g_testsMenuButtonChordArmed == TRUE) &&
        (time_reached(nowMs, g_testsMenuButtonChordSinceMs + TESTS_MENU_BUTTON_CHORD_MS) == TRUE))
    {
        g_testsMenuButtonChordArmed = FALSE;
        return TRUE;
    }

    return FALSE;
}

static boolean tests_menu_button_chord_is_down(void)
{
    return (boolean)((Buttons_IsPressed(BUTTON_ID_SW2) == TRUE) &&
                     (Buttons_IsPressed(BUTTON_ID_SW3) == TRUE));
}

static void tests_menu_drain_button_events(void)
{
    (void)Buttons_WasPressed(BUTTON_ID_SW2);
    (void)Buttons_WasPressed(BUTTON_ID_SW3);
}

static void runtime_test_enter(RuntimeTestId_t testId, uint32 nowMs)
{
    switch (testId)
    {
        case RUNTIME_TEST_ESC:
            esc_test_enter(nowMs);
            break;

        case RUNTIME_TEST_SERVO:
            servo_test_enter(nowMs);
            break;

        case RUNTIME_TEST_ULTRASONIC:
            ultrasonic_test_enter(nowMs);
            break;

        case RUNTIME_TEST_CAMSERVO:
            linear_camera_test_enter_common(nowMs, TRUE);
            break;

        case RUNTIME_TEST_SIMPLE_DRIVE:
            simple_drive_test_enter(nowMs);
            break;

        case RUNTIME_TEST_TUNE_DRIVE:
            tune_drive_test_enter(nowMs);
            break;

        case RUNTIME_TEST_STACK_DRIVE:
            stack_drive_test_enter(nowMs);
            break;

        case RUNTIME_TEST_SERIAL_TUNE:
            serial_tune_test_enter(nowMs);
            break;

        case RUNTIME_TEST_ULTRA_ESC:
            ultrasonic_esc_test_enter(nowMs);
            break;

        case RUNTIME_TEST_ESP_LINK:
            esp_link_bench_test_enter(nowMs);
            break;

        case RUNTIME_TEST_TEENSY_LINK:
            teensy_link_test_enter(nowMs);
            break;

        case RUNTIME_TEST_VICTORY_LAP:
            victory_lap_test_enter(nowMs);
            break;

        case RUNTIME_TEST_LINEAR_CAMERA:
        default:
            linear_camera_test_enter(nowMs);
            break;
    }
}

static void runtime_test_update(RuntimeTestId_t testId, uint32 nowMs, boolean sw2Pressed,
                                boolean modeNextPressed, uint8 potLevel)
{
    switch (testId)
    {
        case RUNTIME_TEST_ESC:
            esc_test_update(nowMs, sw2Pressed, modeNextPressed, potLevel);
            break;

        case RUNTIME_TEST_SERVO:
            servo_test_update(nowMs, sw2Pressed, potLevel);
            break;

        case RUNTIME_TEST_ULTRASONIC:
            ultrasonic_test_update(nowMs, sw2Pressed);
            break;

        case RUNTIME_TEST_CAMSERVO:
            linear_camera_test_update(nowMs, FALSE, potLevel);
            break;

        case RUNTIME_TEST_SIMPLE_DRIVE:
            simple_drive_test_update(nowMs, sw2Pressed, modeNextPressed);
            break;

        case RUNTIME_TEST_TUNE_DRIVE:
            tune_drive_test_update(nowMs, modeNextPressed);
            break;

        case RUNTIME_TEST_STACK_DRIVE:
            stack_drive_test_update(nowMs, sw2Pressed, modeNextPressed);
            break;

        case RUNTIME_TEST_SERIAL_TUNE:
            serial_tune_test_update();
            break;

        case RUNTIME_TEST_ULTRA_ESC:
            ultrasonic_esc_test_update(nowMs);
            break;

        case RUNTIME_TEST_ESP_LINK:
            esp_link_bench_test_update(nowMs);
            break;

        case RUNTIME_TEST_TEENSY_LINK:
            teensy_link_test_update(nowMs, sw2Pressed);
            break;

        case RUNTIME_TEST_VICTORY_LAP:
            victory_lap_test_update(nowMs);
            break;

        case RUNTIME_TEST_LINEAR_CAMERA:
        default:
            linear_camera_test_update(nowMs, modeNextPressed, potLevel);
            break;
    }
}

static void runtime_test_exit(RuntimeTestId_t testId)
{
    switch (testId)
    {
        case RUNTIME_TEST_ESC:
            esc_test_exit();
            break;

        case RUNTIME_TEST_SERVO:
            servo_test_exit();
            break;

        case RUNTIME_TEST_ULTRASONIC:
            ultrasonic_test_exit();
            break;

        case RUNTIME_TEST_CAMSERVO:
            linear_camera_test_exit();
            break;

        case RUNTIME_TEST_SIMPLE_DRIVE:
            simple_drive_test_exit();
            break;

        case RUNTIME_TEST_TUNE_DRIVE:
            tune_drive_test_exit();
            break;

        case RUNTIME_TEST_STACK_DRIVE:
            stack_drive_test_exit();
            break;

        case RUNTIME_TEST_SERIAL_TUNE:
            serial_tune_test_exit();
            break;

        case RUNTIME_TEST_ULTRA_ESC:
            ultrasonic_esc_test_exit();
            break;

        case RUNTIME_TEST_ESP_LINK:
            esp_link_bench_test_exit();
            break;

        case RUNTIME_TEST_TEENSY_LINK:
            teensy_link_test_exit();
            break;

        case RUNTIME_TEST_VICTORY_LAP:
            victory_lap_test_exit();
            break;

        case RUNTIME_TEST_LINEAR_CAMERA:
        default:
            linear_camera_test_exit();
            break;
    }
}
static uint8 tests_menu_map_pot_to_index(uint8 pot, uint8 nItems)
{
    uint16 scaled = (uint16)pot * (uint16)nItems;
    uint8 idx = (uint8)(scaled / 256u);

    if (idx >= nItems)
    {
        idx = (uint8)(nItems - 1u);
    }

    return idx;
}

static void tests_menu_select_index(TestsMenuState_t *st, uint8 pot)
{
    uint32 nowMs;
    uint8 newIdx;

    if (st == NULL_PTR)
    {
        return;
    }

    nowMs = Timebase_GetMs();
    if ((uint32)(nowMs - st->lastPotMs) < TESTS_MENU_POT_STABLE_MS)
    {
        return;
    }
    st->lastPotMs = nowMs;

    newIdx = tests_menu_map_pot_to_index(pot, (uint8)RUNTIME_TEST_COUNT);

    if (newIdx != st->selectedIndex)
    {
        uint16 regionW = 256u / (uint16)RUNTIME_TEST_COUNT;
        uint16 newCenter = (uint16)newIdx * regionW + (regionW / 2u);
        uint16 potU = (uint16)pot;
        uint16 diff = (potU > newCenter) ? (potU - newCenter) : (newCenter - potU);

        if (diff >= TESTS_MENU_POT_HYSTERESIS)
        {
            st->selectedIndex = newIdx;
        }
    }

    if (st->selectedIndex < st->topIndex)
    {
        st->topIndex = st->selectedIndex;
    }
    else if (st->selectedIndex >= (uint8)(st->topIndex + TESTS_MENU_VISIBLE_LINES))
    {
        st->topIndex = (uint8)(st->selectedIndex - (TESTS_MENU_VISIBLE_LINES - 1u));
    }

    if (st->topIndex > (uint8)(RUNTIME_TEST_COUNT - TESTS_MENU_VISIBLE_LINES))
    {
        st->topIndex = (uint8)(RUNTIME_TEST_COUNT - TESTS_MENU_VISIBLE_LINES);
    }
}

static void tests_menu_draw(const TestsMenuState_t *st)
{
    if (st == NULL_PTR)
    {
        return;
    }

    DisplayClear();
    DisplayTextPadded(0U, "NXP CUP TESTS");

    for (uint8 line = 0u; line < TESTS_MENU_VISIBLE_LINES; line++)
    {
        uint8 idx = (uint8)(st->topIndex + line);
        uint8 row = (uint8)(1u + line);

        DisplayTextPadded(row, "");
        if (idx == st->selectedIndex)
        {
            DisplayTextPadded(row, ">");
        }
        DisplayText(row, g_testsMenuItems[idx], 15U, 1U);
    }

    DisplayRefresh();
    StatusLed_Blue();
}

static void tests_menu_draw_release_switch(void)
{
    DisplayClear();
    DisplayTextPadded(0U, "NXP CUP TESTS");
    DisplayTextPadded(1U, "SWPCB ACTIVE");
    DisplayTextPadded(2U, "MOVE SWITCH");
    DisplayTextPadded(3U, "TO MENU POS");
    DisplayRefresh();
    StatusLed_Yellow();
}

void mode_nxp_cup_tests(void)
{
    uint32 nextButtonsMs;

    App_InitRuntimeCommon();
    (void)memset(&g_testsMenu, 0, sizeof(g_testsMenu));
    g_testsMenuButtonChordSinceMs = 0U;
    g_testsMenuButtonChordArmed = TRUE;
    g_testsMenuSwitchBypass = FALSE;
    g_testsMenuIgnoreSwitchUntilOff = FALSE;
    g_testsMenu.activeTest = RUNTIME_TEST_LINEAR_CAMERA;
    g_testsMenu.requireSwitchOff = TRUE;
    g_testsMenu.switchGuardUntilMs = Timebase_GetMs() + TESTS_MENU_SWITCH_BOOT_GUARD_MS;
    tests_menu_draw_release_switch();

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
        if ((g_testsMenuIgnoreSwitchUntilOff == TRUE) && (modeSwitchOn != TRUE))
        {
            g_testsMenuIgnoreSwitchUntilOff = FALSE;
        }
        effectiveModeSwitchOn =
            (boolean)((g_testsMenuIgnoreSwitchUntilOff == TRUE) ? FALSE : modeSwitchOn);

        if (g_testsMenu.requireSwitchOff == TRUE)
        {
            boolean buttonChordPressed;

            tests_menu_drain_button_events();
            buttonChordPressed = tests_menu_button_chord_pressed(nowMs);

            if (time_reached(nowMs, g_testsMenu.switchGuardUntilMs) != TRUE)
            {
                continue;
            }

            if ((modeSwitchOn == TRUE) && (buttonChordPressed != TRUE))
            {
                tests_menu_draw_release_switch();
                continue;
            }

            if ((modeSwitchOn == TRUE) && (buttonChordPressed == TRUE))
            {
                g_testsMenuIgnoreSwitchUntilOff = TRUE;
            }

            g_testsMenu.requireSwitchOff = FALSE;
            tests_menu_select_index(&g_testsMenu, potLevel);
            tests_menu_draw(&g_testsMenu);
            continue;
        }

        if (g_testsMenu.testActive != TRUE)
        {
            boolean buttonChordPressed;

            tests_menu_drain_button_events();
            buttonChordPressed = tests_menu_button_chord_pressed(nowMs);
            tests_menu_select_index(&g_testsMenu, potLevel);
            tests_menu_draw(&g_testsMenu);

            if ((effectiveModeSwitchOn == TRUE) || (buttonChordPressed == TRUE))
            {
                g_testsMenu.activeTest = (RuntimeTestId_t)g_testsMenu.selectedIndex;
                g_testsMenuSwitchBypass = buttonChordPressed;
                runtime_test_enter(g_testsMenu.activeTest, nowMs);
                g_testsMenu.testActive = TRUE;
            }
        }
        else
        {
            boolean buttonChordDown = tests_menu_button_chord_is_down();
            boolean buttonChordPressed = tests_menu_button_chord_pressed(nowMs);
            boolean switchExitRequested =
                (boolean)((g_testsMenuSwitchBypass != TRUE) && (effectiveModeSwitchOn != TRUE));

            if ((switchExitRequested == TRUE) || (buttonChordPressed == TRUE))
            {
                runtime_test_exit(g_testsMenu.activeTest);
                g_testsMenu.testActive = FALSE;
                g_testsMenuSwitchBypass = FALSE;
                tests_menu_drain_button_events();
                tests_menu_draw(&g_testsMenu);
            }
            else
            {
                if (buttonChordDown == TRUE)
                {
                    tests_menu_drain_button_events();
                    sw2Pressed = FALSE;
                    sw3Pressed = FALSE;
                }
                else
                {
                    sw2Pressed = Buttons_WasPressed(BUTTON_ID_SW2);
                    sw3Pressed = Buttons_WasPressed(BUTTON_ID_SW3);
                }
                runtime_test_update(g_testsMenu.activeTest, nowMs, sw2Pressed, sw3Pressed,
                                    potLevel);
            }
        }
    }
}
void mode_linear_camera_test(void)
{
    uint32 nextButtonsMs;

    App_InitRuntimeCommon();
    runtime_test_enter(RUNTIME_TEST_LINEAR_CAMERA, Timebase_GetMs());

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

        runtime_test_update(RUNTIME_TEST_LINEAR_CAMERA, nowMs, FALSE, sw3Pressed, potLevel);
    }
}

void mode_esc_test(void)
{
    uint32 nextButtonsMs;

    App_InitRuntimeCommon();
    runtime_test_enter(RUNTIME_TEST_ESC, Timebase_GetMs());

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
        runtime_test_update(RUNTIME_TEST_ESC, nowMs, FALSE, FALSE, potLevel);
    }
}
