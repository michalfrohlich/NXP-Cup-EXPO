#pragma once
/*
  app_modes.h
  ===========
  This is the entry for the "application layer".

  The application layer is allowed to call hardware drivers.
  It runs the main loop and state machine.

  It chooses which mode runs based on car_config.h switches.
*/

#ifdef __cplusplus
extern "C" {
#endif

/* Run the selected mode forever (never returns). */
void App_RunSelectedMode(void);

#ifdef __cplusplus
}
#endif
