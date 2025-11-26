#include "camera_emulator.h"

/* --- Simple camera emulator (no internal timing) ---
 * - Each call to CameraEmulator_GetFrame() generates a fresh frame
 * - Frame = "straight line" brightness with small random noise
 * - Values are in range [0..255], like a real ADC camera output
 */

static uint32 RngState = 1U;

/* Very simple pseudo-random generator (LCG) */
static sint16 Emu_SimpleRand(void)
{
    RngState = RngState * 1103515245U + 12345U;
    return (sint16)((RngState >> 16) & 0x7FFF);
}

// === OUTDATED FUNCTION THAT GENERATES THE BASELEVEL: ===
// Generate one new frame directly into the output buffer
//static void CameraEmulator_GenerateFrame(uint8 Pixels[CAMERA_EMU_NUM_PIXELS])
//{
//    /* Base brightness somewhere in a bright range (like an illuminated track) */
//    uint8 baseLevel = (uint8)(180U + (Emu_SimpleRand() % 60));  /* 180..239 */
//
//    for (uint16 i = 0U; i < CAMERA_EMU_NUM_PIXELS; i++)
//    {
//        /* Noise roughly in [-10 .. +10] */
//        //sint16 noise = (sint16)((Emu_SimpleRand() % 21) - 10); // -10..+10
//        sint16 noise = (sint16)((Emu_SimpleRand() % 61) - 30);  // -30..+30
//        sint16 val   = (sint16)baseLevel + noise;
//
//        if (val < 0)
//        {
//            val = 0;
//        }
//        else if (val > 255)
//        {
//            val = 255;
//        }
//
//        Pixels[i] = (uint8)val;
//    }
//}
// =================================================


/* Generate one new frame directly into the output buffer
 * using the caller-provided baseLevel.
 */
static void CameraEmulator_GenerateFrame(uint8 Pixels[CAMERA_EMU_NUM_PIXELS],
                                         uint8 baseLevel)
{
    for (uint16 i = 0U; i < CAMERA_EMU_NUM_PIXELS; i++)
    {
        /* Noise roughly in [-30 .. +30] for visible variation */
        sint16 noise = (sint16)((Emu_SimpleRand() % 61) - 30);  /* -30..+30 */
        sint16 val   = (sint16)baseLevel + noise;

        if (val < 0)
        {
            val = 0;
        }
        else if (val > 255)
        {
            val = 255;
        }

        Pixels[i] = (uint8)val;
    }
}

void CameraEmulator_Init(void)
{
    /* Seed RNG to a known value (optional: could be time-based later) */
    RngState = 1U;
}

void CameraEmulator_GetFrame(uint8 Pixels[CAMERA_EMU_NUM_PIXELS],
                             uint8 baseLevel)
{
    /* No timing, no delay: just generate a fresh frame right now */
    CameraEmulator_GenerateFrame(Pixels, baseLevel);
}
