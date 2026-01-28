#include "rgb_led.h"
#include "Dio_Cfg.h"   /* Contains DioConf_DioChannel_* symbolic names */

/*
 * S32K144EVB RGB LED is active-low:
 *   - Write LOW  => LED ON
 *   - Write HIGH => LED OFF
 */
static inline Dio_LevelType RgbLed_LevelFromBool(bool on)
{
    return on ? (Dio_LevelType)STD_LOW : (Dio_LevelType)STD_HIGH;
}

/* Bind to generated symbolic names */
#define RGB_LED_DIO_CH_R   (DioConf_DioChannel_DioChannel_LED_R)
#define RGB_LED_DIO_CH_G   (DioConf_DioChannel_DioChannel_LED_G)
#define RGB_LED_DIO_CH_B   (DioConf_DioChannel_DioChannel_LED_B)

void RgbLed_SetChannel(RgbLed_Channel ch, bool on)
{
    const Dio_LevelType level = RgbLed_LevelFromBool(on);

    switch (ch)
    {
        case RGB_LED_RED:
            Dio_WriteChannel(RGB_LED_DIO_CH_R, level);
            break;

        case RGB_LED_GREEN:
            Dio_WriteChannel(RGB_LED_DIO_CH_G, level);
            break;

        case RGB_LED_BLUE:
            Dio_WriteChannel(RGB_LED_DIO_CH_B, level);
            break;

        default:
            /* Defensive: do nothing */
            break;
    }
}

void RgbLed_ChangeColor(RgbLed_Color c)
{
    Dio_WriteChannel(RGB_LED_DIO_CH_R, RgbLed_LevelFromBool(c.r));
    Dio_WriteChannel(RGB_LED_DIO_CH_G, RgbLed_LevelFromBool(c.g));
    Dio_WriteChannel(RGB_LED_DIO_CH_B, RgbLed_LevelFromBool(c.b));
}
