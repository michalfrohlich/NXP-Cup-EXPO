#ifndef RGB_LED_H
#define RGB_LED_H

#include <stdbool.h>
#include "Dio.h"

/* Simple channel selector */
typedef enum
{
    RGB_LED_RED = 0,
    RGB_LED_GREEN,
    RGB_LED_BLUE
} RgbLed_Channel;

/* Optional convenience color struct */
typedef struct
{
    bool r;
    bool g;
    bool b;
} RgbLed_Color;

/**
 * @brief Set a single LED channel ON/OFF.
 *
 * @param ch  Channel (R/G/B)
 * @param on  true = LED emits light, false = LED off
 */
void RgbLed_SetChannel(RgbLed_Channel ch, bool on);

/**
 * @brief Set the RGB LED in one call.
 *
 * @param c  Desired state; true means ON for that color.
 */
void RgbLed_ChangeColor(RgbLed_Color c);

#endif /* RGB_LED_H */
