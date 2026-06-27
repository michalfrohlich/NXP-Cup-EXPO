#include "app/teensy_app_modes.h"

#include "config/camera_config.h"
#include "services/teensy_race_runtime.h"
#include "teensy_config.h"

static TeensyRaceRuntime linkBenchRuntime;

void ModeTeensyLink_Setup()
{
    const TeensyRaceRuntimeConfig config = {
        TEENSY_LINK_SERIAL_DEBUG_ENABLED,
        TEENSY_LINEAR_CAMERA_STREAM_ENABLED,
        TEENSY_LINK_SERIAL_PERIOD_MS,
        false,
        false,
        0U,
    };

    linkBenchRuntime.begin(config);
}

void ModeTeensyLink_Loop()
{
    linkBenchRuntime.service();
}
