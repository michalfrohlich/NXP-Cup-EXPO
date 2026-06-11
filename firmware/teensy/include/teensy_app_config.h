#pragma once

/* Teensy build mode selection, same idea as the S32K app_config.h.

   0 = normal link app: SPI slave + SD logger + displays + status LED.
   1 = hardware self-test: no SPI slave; cycles the RGB LED, drives both
       displays with live values, prints pot/buttons/SD/SPI pin levels on
       serial, and writes SD test rows. Use it to verify the PCB alone. */
#define TEENSY_APP_HARDWARE_TEST 0
