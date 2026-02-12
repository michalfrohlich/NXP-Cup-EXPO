#include "buttons.h"

/* AUTOSAR MCAL Dio API */
#include "Dio.h"
#include "Dio_Cfg.h"

/* ===== Configuration ===== */

/* Debounce depth in samples.
 * If Buttons_Update() is called every 5 ms and this is 3,
 * debounce time is ~15 ms.
 */
#define BUTTON_DEBOUNCE_TICKS   (3u)

/* Active level for "pressed".
 * released -> 0, pressed -> 1 => STD_HIGH
 */
#define BUTTON_ACTIVE_LEVEL_PRESSED   STD_HIGH

/* ===== Internal types ===== */

typedef struct
{
    uint8   stableState;      /* 0 = not pressed, 1 = pressed (debounced) */
    uint8   lastStableState;  /* previous stable state */
    uint8   counter;          /* debounce counter */
    boolean pressedEvent;     /* latched "0->1" event */
} ButtonContext_t;

/* ===== Static configuration ===== */

/* One DIO channel per button ID; order MUST match ButtonId_t enum */
static const Dio_ChannelType Buttons_Channels[BUTTON_ID_COUNT] =
{
    DioConf_DioChannel_PTC12_sw2,   /* BUTTON_ID_SW2 */
    DioConf_DioChannel_PTC13_sw3    /* BUTTON_ID_SW3 */
};

/* For now both active-high (pressed = STD_HIGH) */
static const boolean Buttons_ActiveHigh[BUTTON_ID_COUNT] =
{
    (BUTTON_ACTIVE_LEVEL_PRESSED == STD_HIGH) ? TRUE : FALSE,  /* SW2 */
    (BUTTON_ACTIVE_LEVEL_PRESSED == STD_HIGH) ? TRUE : FALSE   /* SW3 */
};

/* Internal state array; zero-initialised by C startup code */
static ButtonContext_t Buttons_State[BUTTON_ID_COUNT];

/* ===== Public API ===== */

void Buttons_Update(void)
{
    for (uint8 i = 0u; i < (uint8)BUTTON_ID_COUNT; i++)
    {
        ButtonContext_t *btn = &Buttons_State[i];

        /* 1) Raw read */
        Dio_LevelType dioLevel = Dio_ReadChannel(Buttons_Channels[i]);

        /* 2) Convert to rawPressed = 0/1 */
        uint8 rawPressed;
        if (Buttons_ActiveHigh[i] == TRUE)
        {
            rawPressed = (dioLevel == STD_HIGH) ? 1u : 0u;
        }
        else
        {
            rawPressed = (dioLevel == STD_LOW) ? 1u : 0u;
        }

        /* 3) Debounce */
        if (rawPressed != btn->stableState)
        {
            if (btn->counter < 0xFFu)
            {
                btn->counter++;
            }

            if (btn->counter >= BUTTON_DEBOUNCE_TICKS)
            {
                btn->lastStableState = btn->stableState;
                btn->stableState     = rawPressed;
                btn->counter         = 0u;

                /* Rising edge: 0 -> 1 */
                if ((btn->lastStableState == 0u) && (btn->stableState == 1u))
                {
                    btn->pressedEvent = TRUE;
                }
            }
        }
        else
        {
            btn->counter = 0u;
        }
    }
}

boolean Buttons_IsPressed(ButtonId_t id)
{
    if ((uint8)id >= (uint8)BUTTON_ID_COUNT)
    {
        return FALSE;
    }

    return (Buttons_State[(uint8)id].stableState != 0u) ? TRUE : FALSE;
}

boolean Buttons_WasPressed(ButtonId_t id)
{
    if ((uint8)id >= (uint8)BUTTON_ID_COUNT)
    {
        return FALSE;
    }

    if (Buttons_State[(uint8)id].pressedEvent == TRUE)
    {
        Buttons_State[(uint8)id].pressedEvent = FALSE;
        return TRUE;
    }

    return FALSE;
}
