#include "buttons.h"
#include "Dio.h"
#include "Dio_Cfg.h"

/* ===== Configuration ===== */

/* Debounce depth in samples.
 * If Buttons_Update() is called every 5 ms and this is 3,
 * debounce time is about 15 ms.
 */
#define BUTTON_DEBOUNCE_TICKS         (3u)

/* Active level for asserted inputs.
 * SW2/SW3 were previously observed as released -> 0, pressed -> 1.
 * `swPcb` currently uses the same assumption until measured otherwise.
 */
#define BUTTON_ACTIVE_LEVEL_ASSERTED  STD_HIGH

/* `swPcb` is configured in Port as PTA12 GPIO input, but the current generated
 * Dio config does not publish a named DioConf_DioChannel_* symbol for it.
 * The RTD DIO channel encoding is (port << 5) | pin, so PTA12 is 0x000C.
 */
#define BUTTONS_DIO_CHANNEL_SWPCB     ((Dio_ChannelType)0x000CU)

/* ===== Internal types ===== */

typedef struct
{
    uint8 stableState;      /* 0 = inactive, 1 = active (debounced) */
    uint8 lastStableState;  /* previous stable state */
    uint8 counter;          /* debounce counter */
    boolean pressedEvent;   /* latched 0->1 event for momentary inputs */
} ButtonContext_t;

typedef enum
{
    BUTTON_INPUT_KIND_MOMENTARY = 0,
    BUTTON_INPUT_KIND_TOGGLE
} ButtonInputKind_t;

/* ===== Static configuration (no init needed) ===== */

/* One DIO channel per logical input; order must match ButtonId_t. */
static const Dio_ChannelType Buttons_Channels[BUTTON_ID_COUNT] =
{
    DioConf_DioChannel_PTC12_sw2,   /* BUTTON_ID_SW2 */
    DioConf_DioChannel_PTC13_sw3,   /* BUTTON_ID_SW3 */
    BUTTONS_DIO_CHANNEL_SWPCB       /* BUTTON_ID_SWPCB / PTA12 */
};

static const boolean Buttons_ActiveHigh[BUTTON_ID_COUNT] =
{
    (BUTTON_ACTIVE_LEVEL_ASSERTED == STD_HIGH) ? TRUE : FALSE,  /* SW2 */
    (BUTTON_ACTIVE_LEVEL_ASSERTED == STD_HIGH) ? TRUE : FALSE,  /* SW3 */
    (BUTTON_ACTIVE_LEVEL_ASSERTED == STD_HIGH) ? TRUE : FALSE   /* swPcb */
};

static const ButtonInputKind_t Buttons_InputKinds[BUTTON_ID_COUNT] =
{
    BUTTON_INPUT_KIND_MOMENTARY,  /* SW2 */
    BUTTON_INPUT_KIND_MOMENTARY,  /* SW3 */
    BUTTON_INPUT_KIND_TOGGLE      /* swPcb */
};

/* Internal state array; zero-initialized by C startup code. */
static ButtonContext_t Buttons_State[BUTTON_ID_COUNT];

/* ===== Public API ===== */

void Buttons_Update(void)
{
    for (uint8 i = 0u; i < (uint8)BUTTON_ID_COUNT; i++)
    {
        ButtonContext_t *btn = &Buttons_State[i];
        Dio_LevelType dioLevel = Dio_ReadChannel(Buttons_Channels[i]);
        uint8 rawAsserted;

        if (Buttons_ActiveHigh[i] == TRUE)
        {
            rawAsserted = (dioLevel == STD_HIGH) ? 1u : 0u;
        }
        else
        {
            rawAsserted = (dioLevel == STD_LOW) ? 1u : 0u;
        }

        if (rawAsserted != btn->stableState)
        {
            if (btn->counter < 0xFFu)
            {
                btn->counter++;
            }

            if (btn->counter >= BUTTON_DEBOUNCE_TICKS)
            {
                btn->lastStableState = btn->stableState;
                btn->stableState = rawAsserted;
                btn->counter = 0u;

                if ((Buttons_InputKinds[i] == BUTTON_INPUT_KIND_MOMENTARY) &&
                    (btn->lastStableState == 0u) &&
                    (btn->stableState == 1u))
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

boolean Buttons_IsOn(ButtonId_t id)
{
    return Buttons_IsPressed(id);
}

boolean Buttons_WasPressed(ButtonId_t id)
{
    if ((uint8)id >= (uint8)BUTTON_ID_COUNT)
    {
        return FALSE;
    }

    if (Buttons_InputKinds[(uint8)id] != BUTTON_INPUT_KIND_MOMENTARY)
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
