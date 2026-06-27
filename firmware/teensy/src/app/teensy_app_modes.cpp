#include "app/teensy_app_modes.h"

#include "app/teensy_app_config.h"

void TeensyApp_Setup()
{
#if TEENSY_APP_SELECTED_MODE == TEENSY_APP_MODE_LINK_BENCH
    ModeTeensyLink_Setup();
#elif TEENSY_APP_SELECTED_MODE == TEENSY_APP_MODE_CAMERA_BENCH
    ModeCameraBench_Setup();
#elif TEENSY_APP_SELECTED_MODE == TEENSY_APP_MODE_RACE
    ModeRace_Setup();
#elif TEENSY_APP_SELECTED_MODE == TEENSY_APP_MODE_HARDWARE_TEST
    ModeHardwareTest_Setup();
#elif TEENSY_APP_SELECTED_MODE == TEENSY_APP_MODE_SD_LOG_TEST
    ModeSdLogTest_Setup();
#elif TEENSY_APP_SELECTED_MODE == TEENSY_APP_MODE_STACK_DRIVE
    ModeStackDrive_Setup();
#else
#error "Unsupported TEENSY_APP_SELECTED_MODE"
#endif
}

void TeensyApp_Loop()
{
#if TEENSY_APP_SELECTED_MODE == TEENSY_APP_MODE_LINK_BENCH
    ModeTeensyLink_Loop();
#elif TEENSY_APP_SELECTED_MODE == TEENSY_APP_MODE_CAMERA_BENCH
    ModeCameraBench_Loop();
#elif TEENSY_APP_SELECTED_MODE == TEENSY_APP_MODE_RACE
    ModeRace_Loop();
#elif TEENSY_APP_SELECTED_MODE == TEENSY_APP_MODE_HARDWARE_TEST
    ModeHardwareTest_Loop();
#elif TEENSY_APP_SELECTED_MODE == TEENSY_APP_MODE_SD_LOG_TEST
    ModeSdLogTest_Loop();
#elif TEENSY_APP_SELECTED_MODE == TEENSY_APP_MODE_STACK_DRIVE
    ModeStackDrive_Loop();
#else
#error "Unsupported TEENSY_APP_SELECTED_MODE"
#endif
}
