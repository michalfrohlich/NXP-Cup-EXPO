#ifndef BUTTONS_H
#define BUTTONS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Std_Types.h"   /* for boolean, uint8, etc. */

/* Logical IDs for debounced digital inputs. */
typedef enum
{
    BUTTON_ID_SW2 = 0,  /* PTC12_sw2 */
    BUTTON_ID_SW3,      /* PTC13_sw3 */
    BUTTON_ID_SWPCB,    /* PTA12 swPcb toggle switch */
    BUTTON_ID_COUNT
} ButtonId_t;

/**
 * @brief Periodic update for debouncing and edge detection.
 *
 * Call this regularly from the main loop (e.g. every camera frame).
 */
void Buttons_Update(void);

/**
 * @brief Current debounced state of a button.
 *
 * @return TRUE if currently pressed (debounced), FALSE otherwise.
 */
boolean Buttons_IsPressed(ButtonId_t id);

/**
 * @brief Current debounced state of a maintained toggle switch.
 *
 * This is a semantic alias for level-based inputs that do not use click events.
 *
 * @return TRUE when the switch is in its active/on position, FALSE otherwise.
 */
boolean Buttons_IsOn(ButtonId_t id);

/**
 * @brief "Just pressed" event (rising edge) since last call.
 *
 * One-shot:
 *  - Returns TRUE once when a new press is detected (0 -> 1).
 *  - Clears the event flag internally.
 */
boolean Buttons_WasPressed(ButtonId_t id);

#ifdef __cplusplus
}
#endif

#endif /* BUTTONS_H */
