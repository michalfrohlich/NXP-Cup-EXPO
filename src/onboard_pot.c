#include "onboard_pot.h"
#include "Adc.h"

/* Symbolic group for the onboard pot.
 * This must match the group name generated in Adc_VS_0_PBcfg.h
 * (configured to use ADC0_EXT12 on the EVB).
 */
#define ONBOARD_POT_ADC_GROUP   AdcGroup_pot

/* Expression to watch for debugging (shows the last raw ADC value). */
volatile uint16_t adc_raw;

/* One-channel result buffer for the pot group. The MCAL driver writes
 * the conversion result into this buffer.
 */
static Adc_ValueGroupType s_PotResultBuffer[1];

/* Last filtered value for smoothing (0..255). */
static uint8 s_LastLevel = 128U;

/* Initialize the onboard potentiometer reading.
 * - Sets up the ADC result buffer for the pot group.
 * - Initializes the filtered value to mid-scale.
 *
 * Call this AFTER Adc_Init(&Adc_Config) in your startup code.
 */
void OnboardPot_Init(void)
{
    s_LastLevel = 128U;

    /* Tell the ADC driver where to store conversion results for this group.
     * This is REQUIRED before using Adc_StartGroupConversion/Adc_ReadGroup.
     */
    (void)Adc_SetupResultBuffer(ONBOARD_POT_ADC_GROUP, s_PotResultBuffer);
}

/* Internal helper: perform a blocking conversion on the pot group
 * and update s_PotResultBuffer[0].
 */
static void OnboardPot_DoConversion(void)
{
    Adc_GroupType group = ONBOARD_POT_ADC_GROUP;

    /* Start a one-shot conversion on the pot group. */
    (void)Adc_StartGroupConversion(group);

    /* Busy-wait until the conversion finishes. For a slow, occasionally
     * read potentiometer this is acceptable. If you ever want to avoid
     * blocking, you can move to notification-based handling.
     */
    while (Adc_GetGroupStatus(group) == ADC_BUSY)
    {
        /* spin */
    }

    /* Copy the result from the driver's internal buffer into our buffer. */
    (void)Adc_ReadGroup(group, s_PotResultBuffer);
}

/* Return the raw ADC value (0..4095 for 12-bit). */
uint16 OnboardPot_ReadRaw(void)
{
    OnboardPot_DoConversion();

    /* Store in a volatile for easy watching in the debugger. */
    adc_raw = (uint16)s_PotResultBuffer[0];

    return adc_raw;
}

/* Map the raw ADC value (0..4095) to a 0..255 range. */
uint8 OnboardPot_ReadLevel_0_255(void)
{
    uint16 raw = OnboardPot_ReadRaw();
    uint16 level;

    /* Simple linear scaling:
     * 0      -> 0
     * 4095   -> 255
     */
    level = (raw * 255U) / 4095U;

    if (level > 255U)
    {
        level = 255U;
    }

    return (uint8)level;
}

/* Smoothed version of the 0..255 reading:
 * new_filtered = 3/4 * old_filtered + 1/4 * new_sample
 */
uint8 OnboardPot_ReadLevelFiltered(void)
{
    uint8 newLevel = OnboardPot_ReadLevel_0_255();
    uint16 smoothed;

    smoothed = ((uint16)s_LastLevel * 3U + (uint16)newLevel) / 4U;
    s_LastLevel = (uint8)smoothed;

    return s_LastLevel;
}
