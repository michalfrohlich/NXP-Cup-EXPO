#include "../app_internal.h"

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

        case RUNTIME_TEST_SERIAL_TUNE:
            serial_tune_test_enter(nowMs);
            break;

        case RUNTIME_TEST_ULTRA_ESC:
            ultrasonic_esc_test_enter(nowMs);
            break;

        case RUNTIME_TEST_RECEIVER:
            receiver_test_enter(nowMs);
            break;

        case RUNTIME_TEST_TEENSY_IMU:
            teensy_imu_test_enter(nowMs);
            break;

        case RUNTIME_TEST_LINEAR_CAMERA:
        default:
            linear_camera_test_enter(nowMs);
            break;
    }
}

static void runtime_test_update(RuntimeTestId_t testId,
                                uint32 nowMs,
                                boolean sw2Pressed,
                                boolean modeNextPressed,
                                uint8 potLevel)
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

        case RUNTIME_TEST_SERIAL_TUNE:
            serial_tune_test_update();
            break;

        case RUNTIME_TEST_ULTRA_ESC:
            ultrasonic_esc_test_update(nowMs);
            break;

        case RUNTIME_TEST_RECEIVER:
            receiver_test_update(nowMs);
            break;

        case RUNTIME_TEST_TEENSY_IMU:
            teensy_imu_test_update(nowMs, sw2Pressed);
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

        case RUNTIME_TEST_SERIAL_TUNE:
            serial_tune_test_exit();
            break;

        case RUNTIME_TEST_ULTRA_ESC:
            ultrasonic_esc_test_exit();
            break;

        case RUNTIME_TEST_RECEIVER:
            receiver_test_exit();
            break;

        case RUNTIME_TEST_TEENSY_IMU:
            teensy_imu_test_exit();
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

void mode_nxp_cup_tests(void)
{
    uint32 nextButtonsMs;

    App_InitRuntimeCommon();
    (void)memset(&g_testsMenu, 0, sizeof(g_testsMenu));
    g_testsMenu.activeTest = RUNTIME_TEST_LINEAR_CAMERA;

    nextButtonsMs = Timebase_GetMs();

    for (;;)
    {
        uint32 nowMs = Timebase_GetMs();
        boolean sw2Pressed = FALSE;
        boolean sw3Pressed = FALSE;
        boolean modeSwitchOn;
        uint8 potLevel;

        while (time_reached(nowMs, nextButtonsMs) == TRUE)
        {
            Buttons_Update();
            nextButtonsMs += BUTTONS_PERIOD_MS;
        }

        potLevel = OnboardPot_ReadLevelFiltered();

        modeSwitchOn = Buttons_IsOn(BUTTON_ID_SWPCB);

        if (g_testsMenu.testActive != TRUE)
        {
            (void)Buttons_WasPressed(BUTTON_ID_SW2);
            (void)Buttons_WasPressed(BUTTON_ID_SW3);
            tests_menu_select_index(&g_testsMenu, potLevel);
            tests_menu_draw(&g_testsMenu);

            if (modeSwitchOn == TRUE)
            {
                g_testsMenu.activeTest = (RuntimeTestId_t)g_testsMenu.selectedIndex;
                runtime_test_enter(g_testsMenu.activeTest, nowMs);
                g_testsMenu.testActive = TRUE;
            }
        }
        else
        {
            if (modeSwitchOn != TRUE)
            {
                runtime_test_exit(g_testsMenu.activeTest);
                g_testsMenu.testActive = FALSE;
                tests_menu_draw(&g_testsMenu);
            }
            else
            {
                sw2Pressed = Buttons_WasPressed(BUTTON_ID_SW2);
                sw3Pressed = Buttons_WasPressed(BUTTON_ID_SW3);
                runtime_test_update(g_testsMenu.activeTest, nowMs, sw2Pressed, sw3Pressed, potLevel);
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
