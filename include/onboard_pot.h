#ifndef ONBOARD_POT_H
#define ONBOARD_POT_H

#include "Std_Types.h"   /* for uint8, uint16 */

/* Initialize anything needed for the onboard potentiometer ADC reading.
 * For our simple use-case this is mostly just for symmetry. */
void OnboardPot_Init(void);

/* Return raw ADC sample (0..4095 for 12-bit). */
uint16 OnboardPot_ReadRaw(void);

/* Map raw ADC sample to 0..255 range. */
uint8 OnboardPot_ReadLevel_0_255(void);

/* Same as above but with simple low-pass filtering
 * to reduce jitter when the pot is near a value. */
uint8 OnboardPot_ReadLevelFiltered(void);

#endif /* ONBOARD_POT_H */
