#include "app/teensy_app_modes.h"

#include "services/teensy_race_runtime.h"

static TeensyRaceRuntime raceRuntime;

void ModeRace_Setup()
{
    const TeensyRaceRuntimeConfig config = {
        false, false, 0U, false, false, 0U,
    };

    raceRuntime.begin(config);
}

void ModeRace_Loop()
{
    raceRuntime.service();
}
