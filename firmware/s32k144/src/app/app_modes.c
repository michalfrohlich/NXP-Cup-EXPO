#include "app_internal.h"

AppBuildMode_t App_GetSelectedBuildMode(void)
{
#if APP_TEST_NXP_CUP
    return APP_BUILD_MODE_NXP_CUP;
#elif APP_TEST_NXP_CUP_TESTS
    return APP_BUILD_MODE_NXP_CUP_TESTS;
#elif APP_TEST_HONOR_LAP
    return APP_BUILD_MODE_HONOR_LAP;
#elif APP_TEST_SERVO_RATE_TEST
    return APP_BUILD_MODE_SERVO_RATE_TEST;
#elif APP_TEST_TEENSY_LINK_TEST
    return APP_BUILD_MODE_TEENSY_LINK_TEST;
#elif APP_TEST_ESP_LINK_TEST
    return APP_BUILD_MODE_ESP_LINK_TEST;
#elif APP_TEST_TEENSY_CAM0_RACE
    return APP_BUILD_MODE_TEENSY_CAM0_RACE;
#elif APP_TEST_RACE_MODE
    return APP_BUILD_MODE_RACE_MODE;
#elif APP_TEST_LINEAR_CAMERA_TEST
    return APP_BUILD_MODE_LINEAR_CAMERA_TEST;
#elif APP_TEST_ESC_TEST
    return APP_BUILD_MODE_ESC_TEST;
#else
#error "CONFIG ERROR: App_GetSelectedBuildMode has no active APP_TEST_* selection"
#endif
}

void App_RunSelectedMode(void)
{
    switch (App_GetSelectedBuildMode())
    {
        case APP_BUILD_MODE_LINEAR_CAMERA_TEST:
            mode_linear_camera_test();
            break;

        case APP_BUILD_MODE_ESC_TEST:
            mode_esc_test();
            break;

        case APP_BUILD_MODE_NXP_CUP:
            mode_nxp_cup();
            break;

        case APP_BUILD_MODE_RACE_MODE:
            mode_race_mode();
            break;

        case APP_BUILD_MODE_TEENSY_CAM0_RACE:
            mode_teensy_cam0_race();
            break;

        case APP_BUILD_MODE_HONOR_LAP:
            mode_honor_lap();
            break;

        case APP_BUILD_MODE_SERVO_RATE_TEST:
            mode_servo_rate_test();
            break;

        case APP_BUILD_MODE_TEENSY_LINK_TEST:
            mode_teensy_link_test();
            break;

        case APP_BUILD_MODE_ESP_LINK_TEST:
            mode_esp_link_test();
            break;

        case APP_BUILD_MODE_NXP_CUP_TESTS:
            mode_nxp_cup_tests();
            break;

        default:
            while (1)
            {
            }
    }
}
