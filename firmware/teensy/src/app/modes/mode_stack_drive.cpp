#include "app/teensy_app_modes.h"

#include "config/display_config.h"
#include "services/teensy_race_runtime.h"
#include "teensy_config.h"

static TeensyRaceRuntime stackDriveRuntime;

void ModeStackDrive_Setup()
{
    const TeensyRaceRuntimeConfig config = {
        false, false, 0U, true, true, TEENSY_DISPLAY_STACK_DRIVE_REFRESH_PERIOD_MS,
    };

    stackDriveRuntime.begin(config);
}

void ModeStackDrive_Loop()
{
    stackDriveRuntime.service();
}
