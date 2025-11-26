#include "onboard_pot.h"
#include "Adc.h"

#define ONBOARD_POT_ADC_GROUP   AdcGroup_pot

//expresion to watch for DEBUGING
volatile uint16_t adc_raw;

/* One-channel result buffer for the pot group */
static Adc_ValueGroupType s_PotResultBuffer[1];

/* Last filtered value for smoothing */
static uint8 s_LastLevel = 128U;

/* Optional init – for now just clear the last value.
 * You could later put ADC calibration or setup calls here if needed. */
void OnboardPot_Init(void)
{
    s_LastLevel = 128U;
}

/* Internal helper: perform one blocking conversion and fill s_PotResultBuffer[0]. */
static void OnboardPot_DoConversion(void)
{
    Adc_GroupType group = 1U; // group 1 is the pot (EXT12)

    /* Start conversion of the group (single channel for the pot) */
    Adc_StartGroupConversion(group);

    /* Busy-wait until conversion finishes.
     * For debug/low-frequency use this is fine. */
    while (Adc_GetGroupStatus(group) == ADC_BUSY)
    {
        /* spin */
    }

    /* Read result(s) into our buffer */
    (void)Adc_ReadGroup(group, s_PotResultBuffer);
}

/* Return raw ADC value 0..4095 (for 12-bit). */
uint16 OnboardPot_ReadRaw(void)
{
    OnboardPot_DoConversion();
    return (uint16)s_PotResultBuffer[0];
}

/* Map raw 0..4095 -> 0..255. */
uint8 OnboardPot_ReadLevel_0_255(void)
{
    uint16 raw = OnboardPot_ReadRaw();
    adc_raw = raw;

    /* If your ADC is 10-bit, change 4095U to 1023U. */
    uint32 level = ((uint32)raw * 255U) / 4095U;

    if (level > 255U)
    {
        level = 255U;
    }

    return (uint8)level;
}

/* Smoothed version: 3/4 old value + 1/4 new value. */
uint8 OnboardPot_ReadLevelFiltered(void)
{
    uint8 newLevel = OnboardPot_ReadLevel_0_255();

    uint16 smoothed = ((uint16)s_LastLevel * 3U + (uint16)newLevel) / 4U;
    s_LastLevel = (uint8)smoothed;

    return s_LastLevel;
}
