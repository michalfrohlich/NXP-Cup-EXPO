#include "app/teensy_app_modes.h"

#include "config/display_config.h"
#include "drivers/board_inputs.h"
#include "services/teensy_race_runtime.h"
#include "teensy_config.h"

static TeensyRaceRuntime stackDriveRuntime;
static bool trackingLedEnabled = true;

void ModeStackDrive_Setup()
{
    const TeensyRaceRuntimeConfig config = {
        false, false, 0U, true, true, TEENSY_DISPLAY_STACK_DRIVE_REFRESH_PERIOD_MS,
    };

    BoardInputs_Init();
    trackingLedEnabled = true;
    stackDriveRuntime.begin(config);
}

void ModeStackDrive_Loop()
{
    BoardInputs_Update();
    if (BoardInputs_WasPressed(BoardButtonId::Button2))
    {
        trackingLedEnabled = !trackingLedEnabled;
        stackDriveRuntime.setTrackingLedEnabled(trackingLedEnabled);
    }

    stackDriveRuntime.service();
}
