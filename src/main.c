/*EXPO_03_Nxp_Cup_project - main file */
#ifdef __cplusplus
extern "C" {
#endif

/*==================================================================================================
 *                                        INCLUDE FILES
==================================================================================================*/
#include "display.h"
#include "receiver.h"
#include "CDD_I2c.h"
#include "esc.h"
#include "servo.h"
#include "linear_camera.h"
#include "camera_emulator.h"
#include "main_functions.h"
#include "hbridge.h"
#include "Mcal.h"
#include "pixy2.h"
#include <stdio.h>
#include <string.h>
#include "onboard_pot.h"
#include "buttons.h"
#include "ultrasonic.h"
#include "timebase.h"

#include "S32K144.h" //bare-metal pot test
/*==================================================================================================
 *                                       LOCAL MACROS
==================================================================================================*/
#define MAIN_ENABLE     1 /* All macros are here */
#define CAR_CAMERA_TEST          0   /* Line-following car demo from linear camera example project*/
#define ULTRASONIC_TEST_LOW_LEVEL 0 /* Used for separate low level control off the ultrasonic pins*/

/* Test mode flags – enable only one at a time */
#define DISPLAY_TEST      0   /* Camera signal simulation + display + buttons + potentiometer demo */
#define ICU_RAW_TEST  0
#define ULTRASONIC_100MS_TEST     0
#define ULTRA_TRIG_SLOW_TEST 0
#define ULTRA_LOOPBACK_ICU_TEST 1
/*==================================================================================================
 *                                       MAIN
==================================================================================================*/
#if MAIN_ENABLE
int main(void)
{
    //LinearCameraFrame FrameBuffer; - part of nxp example code (not using that camera now
    /*Initialize RTD drivers with the compiled configurations*/
     DriversInit();

     Timebase_Init();

     OnboardPot_Init();

    /*Initialize Esc driver*/
    /*First parameter: The Pwm Channel that was configured in Peripherals tool for the Esc*/
    /*Next parameters: Amount of Pwm ticks needed to achieve 1ms, 1.5ms and 2ms long signals, standard for controlling an esc.*/
    /*EscInit(0U, 1638U, 2457U, 3276U);*/

    /*Initialize Servo driver*/
    /*First parameter: The Pwm Channel that was configured in Peripherals tool for the Servo*/
    /*Next parameters: Amount of Pwm ticks needed to achieve maximum desired left turn, right turn and middle position for your servo.*/
    ServoInit(1U, 3300U, 1700U, 2500U);
    /*ServoTest();*/

    /*Initialize Hbridge driver*/
    /*First two parameters: The Pwm Channels that were configured in Peripherals tool for the motors' speeds*/
    /*Next parameters: The Pcr of the pins used for the motors' directions*/
    HbridgeInit(2U, 3U, 32U, 33U, 6U, 64U);
    /*HbridgeTest();*/

    /*Initialize display driver*/
    /*The display driver is for a 0.96", 128x32 OLED monochrome using a SSD1306 chip, like the one in the MR-CANHUBK344 kit*/
    /*Parameter: Should the driver rotate the fontmap before printing. You should leave this to 1U unless you have a special use case*/
    DisplayInit(0U, STD_ON);
    /*DisplayTest();*/

    /*Initialize receiver driver*/
    /*First parameter: The Icu channel that was configured in Peripherals tool for the input pin connected to the receiver's PPM signal*/
    /*Second parameter: The Gpt channel that was configured in Peripherals tool for measuring the input signal's length*/
    /*Next parameters: Amount of Gpt ticks needed to measure the shortest channel signal length, median signal length,
     * maximum signal length and the minimum signal length for the portion between the PPM signals*/
    /*ReceiverInit(0U, 0U, 11700U, 17700U, 23700U, 26000U);*/
    /*ReceiverTest();*/

    /*Initialize linear camera driver*/
    /*First parameter: The Pwm channel that was configured in Peripherals tool for the clock sent to the camera*/
    /*Second parameter: The Gpt channel that was configured in Peripherals tool for determining the length of the shutter signal*/
    /*Third parameter: The Adc group that was configured in Peripherals tool for sampling the analog signal output of the camera*/
    /*Fourth parameter: The Pcr of the pin used for sending the shutter signal to the camera*/
    LinearCameraInit(4U, 1U, 0U, 97U);
    /*LinearCameraTest();*/

    /*Initialize Pixy2 camera driver*/
    /*First parameter: The I2c address that was configured on the Pixy2 that the driver should communicate with*/
    /*Second parameter: The I2c channel that was configured in Peripherals tool for communication with the camera and display*/
    /*Pixy2Init(0x54, 0U);*/
    /*Pixy2Test();*/

    /* Initialize ultrasonic driver (HC-SR04 on PTA15/PTE15) */
    //Ultrasonic_Init(&Ultrasonic_Config); - older drivers

    /* Temporarily force PTD0 as GPIO high → LED off */
    /* Set PTD0 as GPIO output, HIGH → LED off (active-low) */
        IP_PORTD->PCR[0] = PORT_PCR_MUX(1);      /* MUX = 1 → GPIO */
        IP_PTD->PDDR     |= (1UL << 0);          /* PTD0 as output */
        IP_PTD->PSOR     =  (1UL << 0);          /* set PTD0 high → LED off */


	/* Initialize ultrasonic driver (TRIG low, ICU notification, internal state) */
	Ultrasonic_Init();     /* uses Dio + Icu, so it must come after DriversInit */

/*==================================================================================================
*                                  NEW DRIVER TEST
==================================================================================================*/
/*This version:

	triggers every 100 ms, but only if not BUSY

	shows status, cm, ticks, IRQ count

	shows “NEW” reliably via a justGot flag (without changing your driver semantics) */
#if ULTRA_LOOPBACK_ICU_TEST

    /* 16-char fixed lines (padded) */
    const char L0[] = "ULTRA LOOPBACK  ";  /* 16 */
    const char L1[] = "I:     x:      ";   /* 16 */
    const char L2[] = "t:              ";  /* 16 */
    const char L3[] = "Hms:   Lms:    ";   /* 16 */

    /* Use a big pulse so timing is obvious even without scope */
    const uint32 HIGH_MS = 500u;
    const uint32 LOW_MS  = 2000u;

    /* Local timestamp buffer for raw capture */
    static Icu_ValueType tsBuf[8];

    /* Debug values */
    uint32 lastTicks = 0u;
    uint16 lastIdx   = 0u;

    for (;;)
    {
        /* Clear buffer */
        for (uint32 i = 0u; i < 8u; i++)
        {
            tsBuf[i] = 0u;
        }

        /* Arm timestamp capture BEFORE generating the pulse */
        Icu_StartTimestamp(ULTRA_ICU_ECHO_CHANNEL, tsBuf, 8u, 1u);

        /* Generate a clean loopback pulse on TRIG */
        Dio_WriteChannel(ULTRA_DIO_TRIG_CHANNEL, STD_LOW);
        uint32 t0 = Timebase_GetMs();
        while ((Timebase_GetMs() - t0) < 1u) {}

        Dio_WriteChannel(ULTRA_DIO_TRIG_CHANNEL, STD_HIGH);
        uint32 th = Timebase_GetMs();
        while ((Timebase_GetMs() - th) < HIGH_MS) {}

        Dio_WriteChannel(ULTRA_DIO_TRIG_CHANNEL, STD_LOW);
        uint32 tl = Timebase_GetMs();
        while ((Timebase_GetMs() - tl) < LOW_MS) {}

        /* Read how many edges were captured */
        Icu_IndexType idx = Icu_GetTimestampIndex(ULTRA_ICU_ECHO_CHANNEL);
        lastIdx = (uint16)idx;

        /* Compute high time in ticks if we got at least 2 edges */
        if (idx >= 2u)
        {
            uint32 rise = (uint32)tsBuf[0];
            uint32 fall = (uint32)tsBuf[1];
            lastTicks = (uint32)(fall - rise); /* unsigned wrap-safe */
        }
        else
        {
            lastTicks = 0u;
        }

        /* Stop capture until next cycle */
        Icu_StopTimestamp(ULTRA_ICU_ECHO_CHANNEL);

        /* Optional: if you have your ultrasonic driver debug getters, show IRQ count.
           If not, just show idx/ticks and ignore irq. */
        uint32 irqCnt = Ultrasonic_GetIrqCount();

        /* ----- OLED ----- */
        DisplayClear();

        DisplayText(0U, L0, 16U, 0U);

        DisplayText(1U, L1, 16U, 0U);
        DisplayValue(1U, (int)irqCnt, 5U, 2U);      /* I: at col 2..6 */
        DisplayValue(1U, (int)lastIdx, 6U, 10U);     /* x: at col 10..15 */

        DisplayText(2U, L2, 16U, 0U);
        DisplayValue(2U, (int)lastTicks, 14U, 2U);   /* t: at col 2..15 */

        DisplayText(3U, L3, 16U, 0U);
        DisplayValue(3U, (int)HIGH_MS, 3U, 4U);      /* Hms: at col 4..6 */
        DisplayValue(3U, (int)LOW_MS,  4U, 11U);     /* Lms: at col 11..14 */

        DisplayRefresh();
    }

#endif /* ULTRA_LOOPBACK_ICU_TEST */


#if ULTRA_TRIG_SLOW_TEST

    const char L0[] = "TRIG SLOW TEST  ";
    const char L1[] = "TRIG:           ";

    for (;;)
    {
        /* Force TRIG high long enough to observe */
        Dio_WriteChannel(ULTRA_DIO_TRIG_CHANNEL, STD_HIGH);
        uint32 t0 = Timebase_GetMs();
        while ((Timebase_GetMs() - t0) < 200u) {}

        /* Display HIGH */
        DisplayClear();
        DisplayText(0U, L0, 16U, 0U);
        DisplayText(1U, L1, 16U, 0U);
        DisplayText(1U, "TRIG: HIGH      ", 16U, 0U);
        DisplayRefresh();

        /* Force TRIG low long enough to observe */
        Dio_WriteChannel(ULTRA_DIO_TRIG_CHANNEL, STD_LOW);
        t0 = Timebase_GetMs();
        while ((Timebase_GetMs() - t0) < 200u) {}

        /* Display LOW */
        DisplayClear();
        DisplayText(0U, L0, 16U, 0U);
        DisplayText(1U, L1, 16U, 0U);
        DisplayText(1U, "TRIG: LOW       ", 16U, 0U);
        DisplayRefresh();
    }

#endif

#if ULTRASONIC_100MS_TEST

    /* 16-char fixed lines (padded) — safe for your DisplayText() */
    const char L0[] = "ULTRA 100ms     ";  /* 16 */
    const char L1[] = "S:     I:       ";  /* 16 */
    const char L2[] = "cm:    x:       ";  /* 16 */
    const char L3[] = "t:              ";  /* 16 */

    uint32 lastTrigMs     = Timebase_GetMs();
    float  lastDistanceCm = 0.0f;

    for (;;)
    {
        uint32 nowMs = Timebase_GetMs();

        /* Trigger every 100ms, but never interrupt BUSY */
        if ((uint32)(nowMs - lastTrigMs) >= 1000u)
        {
            if (Ultrasonic_GetStatus() != ULTRA_STATUS_BUSY)
            {
                lastTrigMs = nowMs;
                Ultrasonic_StartMeasurement();
            }
        }

        Ultrasonic_Task();

        /* Latch new value if available */
        float distanceCm = 0.0f;
        boolean gotDistance = Ultrasonic_GetDistanceCm(&distanceCm);
        if (gotDistance == TRUE)
        {
            lastDistanceCm = distanceCm;
        }

        Ultrasonic_StatusType st = Ultrasonic_GetStatus();

        uint32 irqCnt = Ultrasonic_GetIrqCount();
        uint32 ticks  = Ultrasonic_GetLastHighTicks();
        uint16 idx    = Ultrasonic_GetTsIndex();

        /* ----- OLED ----- */
        DisplayClear();

        /* Line 0 */
        DisplayText(0U, L0, 16U, 0U);

        /* Line 1 template */
        DisplayText(1U, L1, 16U, 0U);

        /* Status field (Line 1, col 2..6) */
        if (gotDistance == TRUE)
        {
            DisplayText(1U, "NEW  ", 5U, 2U);     /* 5 chars */
        }
        else if (st == ULTRA_STATUS_BUSY)
        {
            DisplayText(1U, "BUSY ", 5U, 2U);
        }
        else if (st == ULTRA_STATUS_ERROR)
        {
            DisplayText(1U, "ERROR", 5U, 2U);
        }
        else
        {
            DisplayText(1U, "IDLE ", 5U, 2U);
        }

        /* IRQ count (Line 1, col 11..15 => len=5) */
        DisplayValue(1U, (int)irqCnt, 5U, 11U);

        /* Line 2 template */
        DisplayText(2U, L2, 16U, 0U);

        /* Distance (Line 2, col 3..6 => len=4) */
        {
            int distInt = (int)(lastDistanceCm + 0.5f);
            DisplayValue(2U, distInt, 4U, 3U);
        }

        /* Timestamp index (Line 2, col 10..15 => len=6) */
        DisplayValue(2U, (int)idx, 6U, 10U);

        /* Line 3 template */
        DisplayText(3U, L3, 16U, 0U);

        /* Ticks (Line 3, col 2..15 => len=14) */
        DisplayValue(3U, (int)ticks, 14U, 2U);

        DisplayRefresh();
    }

#endif /* ULTRASONIC_100MS_TEST */




#if ICU_RAW_TEST
/* ===============================================================
 * ICU RAW TEST WITH TRIG + UsTimer microsecond pulses
 *
 * Sequence per loop:
 *   1) Generate TRIG pulse using UsTimer (2us low → 10us high)
 *   2) Start ICU HIGH-TIME measurement
 *   3) Wait a short time for echo
 *   4) Read ICU value and print on display
 * =============================================================== */

    /* Configure TRIG pin = PTE15 */
    IP_PORTE->PCR[15] = PORT_PCR_MUX(1u);
    IP_PTE->PDDR     |= (1UL << 15);

    /* Init the microsecond timer flag */
    UsTimer_Init();

    DisplayClear();
    DisplayText(0U, "ICU UsTimer", 11U, 0U);
    DisplayRefresh();

    for (;;)
    {
        /* -------------------------------------
         * 1) TRIG PULSE USING UsTimer
         * ------------------------------------- */

        /* LOW for 2 µs */
        IP_PTE->PCOR = (1UL << 15);
        UsTimer_Start(2u);
        while (!UsTimer_HasElapsed()) {}
        UsTimer_ClearFlag();

        /* HIGH for 10 µs */
        IP_PTE->PSOR = (1UL << 15);
        UsTimer_Start(10u);
        while (!UsTimer_HasElapsed()) {}
        UsTimer_ClearFlag();

        /* back LOW */
        IP_PTE->PCOR = (1UL << 15);

        /* -------------------------------------
         * 2) Start ICU HIGH-TIME measurement
         * ------------------------------------- */
        Icu_StopSignalMeasurement(ULTRA_ICU_ECHO_CHANNEL);
        Icu_StartSignalMeasurement(ULTRA_ICU_ECHO_CHANNEL);

        /* -------------------------------------
         * 3) Wait for echo window
         * ------------------------------------- */
        Timebase_DelayMs(5u);

        /* -------------------------------------
         * 4) Read ICU echo pulse width
         * ------------------------------------- */
        Icu_ValueType ticks = Icu_GetTimeElapsed(ULTRA_ICU_ECHO_CHANNEL);

        /* Convert to microseconds: 8 MHz → 8 ticks per µs */
        uint32 us_times100 = (ticks * 100u) / 8u;
        uint32 us_int = us_times100 / 100u;
        uint32 us_dec = us_times100 % 100u;

        /* -------------------------------------
         * 5) DISPLAY
         * ------------------------------------- */
        char buf[20];

        DisplayClear();
        DisplayText(0U, "ICU UsTimer", 11U, 0U);

        DisplayText(1U, "Ticks:", 6U, 0U);
        DisplayValue(1U, ticks, 10U, 7U);

        snprintf(buf, sizeof(buf), "%lu.%02lu us", us_int, us_dec);
        DisplayText(2U, buf, 16U, 0U);

        DisplayRefresh();

        Timebase_DelayMs(40u);
    }

#endif /* ICU_RAW_TEST */

/*==================================================================================================
 *                                       DISPLAY TEST
==================================================================================================*/

	#if DISPLAY_TEST
		uint8 SimPixels[CAMERA_EMU_NUM_PIXELS];
		uint8 GraphValues[CAMERA_EMU_NUM_PIXELS];
		static uint32 EmuFrameCount = 0U; //number of times the graph was updated

		CameraEmulator_Init();

		DisplayClear();
		DisplayText(0U, "HELLO", 5U, 0U);
		DisplayRefresh();

		for (;;) //loop forever
		{

			/* Update debounced button  */
			Buttons_Update();

			/* ----- Capture new frame and process it ----- (for now it is simulated */
			if (g_EmuNewFrameFlag)
			{
			    g_EmuNewFrameFlag = FALSE;

				/* 0) Read potentiometer -> 0..255 "brightness" */

			    uint8 baseLevelPot = OnboardPot_ReadLevelFiltered(); //Read pot normally
			    uint8 baseLevel = ReadBaselineWithButton(baseLevelPot); //Override with button test (read with a different function

				/* 1) Get emulated camera frame */
				CameraEmulator_GetFrame(SimPixels, baseLevel);
				EmuFrameCount++;

				/* 2) Scale 0..255 -> 0..100 for bar height */
				for (uint16 i = 0U; i < CAMERA_EMU_NUM_PIXELS; i++)
				{
					GraphValues[i] = (uint8)(((uint16)SimPixels[i] * 100U) / 255U);
				}

				/* 3) Clear entire display buffer */
				DisplayClear();

				/* 4) Draw bar graph in lower 3 pages (rows 8–31):
				 *    - DisplayLine = 1   → start at second page (row 8)
				 *    - LinesSpan   = 3   → use 3 pages (24 px high)
				 *      Bars will internally use only 2/3 of that height, with a ref line at the top.
				 */
				DisplayBarGraph(1U, GraphValues, CAMERA_EMU_NUM_PIXELS, 3U);

				/* 5) Prepare debug text in the top-left corner (page 0, row 0–7).
				 *    We keep the number reasonably small: modulo 100000 → max 5 digits. */
				/* Left side: read counter "R:xxxxx" */
				char debugLineLeft[16];
				uint32 displayCount = EmuFrameCount % 100000UL;
				(void)snprintf(debugLineLeft, sizeof(debugLineLeft), "R:%5lu", (unsigned long)displayCount);

				/* Right side: baseLevel "B:xxx" */
				/* The display is 128 px wide. Each character is 6 px (5 px glyph + 1 px spacing).
				 * "B:255" is 5 characters → 5 * 6 = 30 px.
				 * 128 - 30 ≈ 98 px → as column index ~98 / 6 ≈ 16 characters from left.
				 */
				char debugLineRight[16];
				(void)snprintf(debugLineRight, sizeof(debugLineRight), "B:%3u", baseLevel);

				/* Draw both texts on page 0.
				 * left text at column = 0 chars
				 * right text at column = 16 chars  (→ top-right area)
				 */
				DisplayText(0U, debugLineLeft, 7U, 0U);   /* top-left  */
				DisplayText(0U, debugLineRight, 5U, 11U); /* top-right */

				/* 6) Send everything to the OLED */
				DisplayRefresh();
			}
		}
	#endif /* DISPLAY_TEST */

} /*main function end */

#endif /* MAIN_ENABLE */

/*==================================================================================================
 *                                  ULTRASONIC_TEST_LOW_LEVEL (OWN MAIN)
==================================================================================================*/

#if ULTRASONIC_TEST_LOW_LEVEL
#include "timebase.h"
#include "Mcal.h"        /* for DriversInit() and MCAL drivers */

/* Simple state machine for the waveform */
typedef enum
{
    WAVE_STATE_LOW_PHASE = 0,
    WAVE_STATE_HIGH_PHASE
} WaveStateType;

static WaveStateType g_waveState = WAVE_STATE_LOW_PHASE;

int main(void)
{
    /* Initialise MCAL drivers (Port, Dio, Gpt, etc.) */
    DriversInit();

    /* Initialise UsTimer software state (flag) */
    UsTimer_Init();

    /* Configure PTE15 as GPIO output */
    IP_PORTE->PCR[15] &= ~PORT_PCR_MUX_MASK;
    IP_PORTE->PCR[15] |= PORT_PCR_MUX(1u);      /* ALT1 = GPIO */
    IP_PTE->PDDR      |= (1UL << 15);           /* PTE15 as output */

    /* --- Start in LOW phase: PTE15 = LOW for 2 µs --- */
    IP_PTE->PCOR = (1UL << 15);                 /* drive LOW */
    g_waveState  = WAVE_STATE_LOW_PHASE;
    UsTimer_Start(2u);                          /* schedule 2 µs */

    for (;;)
    {
        /* Check if UsTimer one-shot has elapsed (set in UsTimer_Notification) */
        if (UsTimer_HasElapsed() == TRUE)
        {
            /* Clear flag so we only handle this expiration once */
            UsTimer_ClearFlag();

            switch (g_waveState)
            {
                case WAVE_STATE_LOW_PHASE:
                    /* LOW phase finished → go HIGH for 10 µs */
                    IP_PTE->PSOR = (1UL << 15); /* drive HIGH */
                    g_waveState  = WAVE_STATE_HIGH_PHASE;
                    UsTimer_Start(2u);         /* schedule 10 µs */
                    break;

                case WAVE_STATE_HIGH_PHASE:
                    /* HIGH phase finished → go LOW for 2 µs */
                    IP_PTE->PCOR = (1UL << 15); /* drive LOW */
                    g_waveState  = WAVE_STATE_LOW_PHASE;
                    UsTimer_Start(10u);          /* schedule 2 µs */
                    break;

                default:
                    /* Should not happen; reset to a safe state */
                    IP_PTE->PCOR = (1UL << 15);
                    g_waveState  = WAVE_STATE_LOW_PHASE;
                    UsTimer_Start(2u);
                    break;
            }
        }

        /* No busy-wait delays here; loop just reacts to the timer flag. */
    }
}
#endif

/*==================================================================================================
 *                                  CAR TEST (LINEAR CAMERA EXAMPLE CODE) (OWN MAIN)
==================================================================================================*/

#if CAR_CAMERA_TEST
/* Linear camera code */
#define THRESHOLD 40
#define BUFFER_SIZE 128
#define MID_POINT BUFFER_SIZE/2

typedef struct {
    uint8 start;
    uint8 end;
}BlackRegion;

BlackRegion BlackRegionBuffer[25];
uint8 RegionIndex;
float TotalCenter;

uint8 findBlackRegions(uint8* Buff, BlackRegion* Regions, uint8 MaxRegions) {
    uint8 RegionIndex = 0;
    uint8 BRegFlag = 0;
    for(int i = 1 ; i < BUFFER_SIZE - 1; i++) {
        if(Buff[i] < THRESHOLD) {
            if(!BRegFlag) {
                BRegFlag = 1;
                Regions[RegionIndex].start = i;
            }
            Regions[RegionIndex].end = i;
        } else {
            if(BRegFlag) {
                BRegFlag = 0;
                RegionIndex ++;
                if(RegionIndex > MaxRegions) {
                    break;
                }
            }
        }
    }
    /* If the buffer ends and we are still in the black region set the black region end to the buffer end */
    if(BRegFlag) {
        Regions[RegionIndex].end = BUFFER_SIZE - 1;
        RegionIndex++;
    }
    return RegionIndex;
}

float calculateCenterOfRegions(BlackRegion Regions[], uint8 RegionCount) {
    uint32 TotalCenter = 0;

    for (int i = 0; i < RegionCount; i++) {
        uint32 RegionCenter = (Regions[i].start + Regions[i].end) / 2;
        TotalCenter += RegionCenter;
    }
    return (float)TotalCenter / RegionCount;
}

int main(void)
{
    LinearCameraFrame FrameBuffer;
    /*Initialize RTD drivers with the compiled configurations*/
     DriversInit();

    /*Initialize Servo driver*/
    /*First parameter: The Pwm Channel that was configured in Peripherals tool for the Servo*/
    /*Next parameters: Amount of Pwm ticks needed to achieve maximum desired left turn, right turn and middle position for your servo.*/
    ServoInit(1U, 3300U, 1700U, 2500U);
    /*ServoTest();*/

    /*Initialize Hbridge driver*/
    /*First two parameters: The Pwm Channels that were configured in Peripherals tool for the motors' speeds*/
    /*Next parameters: The Pcr of the pins used for the motors' directions*/
    HbridgeInit(2U, 3U, 32U, 33U, 6U, 64U);
    /*HbridgeTest();*/

    /*Initialize display driver*/
    /*The display driver is for a 0.96", 128x32 OLED monochrome using a SSD1306 chip, like the one in the MR-CANHUBK344 kit*/
    /*Parameter: Should the driver rotate the fontmap before printing. You should leave this to 1U unless you have a special use case*/
    DisplayInit(0U, STD_ON);
    /*DisplayTest();*/


    /*Initialize linear camera driver*/
    /*First parameter: The Pwm channel that was configured in Peripherals tool for the clock sent to the camera*/
    /*Second parameter: The Gpt channel that was configured in Peripherals tool for determining the length of the shutter signal*/
    /*Third parameter: The Adc group that was configured in Peripherals tool for sampling the analog signal output of the camera*/
    /*Fourth parameter: The Pcr of the pin used for sending the shutter signal to the camera*/
    LinearCameraInit(4U, 1U, 0U, 97U);
    /*CameraTest();*/

    HbridgeSetSpeed(50);
    while(1){
        LinearCameraGetFrame(&FrameBuffer);
        /* The black regions are the values in the CameraResultsBuffer < a chosen threshold */
        /* The threshold must be chosen manually depending on how you calibrated the camera */
        RegionIndex = findBlackRegions(FrameBuffer.Values, BlackRegionBuffer, 3);
        /* Take the middle of all the black regions detected */
        TotalCenter = calculateCenterOfRegions(BlackRegionBuffer, RegionIndex);

        switch(RegionIndex){
        case 1:
            if(TotalCenter < MID_POINT) {
                Steer(-50);
            } else {
                Steer(50);
            }
            break;
        case 2:
            /* Because the Steer takes values between -100 and 100, this formula is needed as totalCenter is unsigned*/
            Steer(2 *(TotalCenter - 50));
            break;
        }
        /*Update the display buffer*/
        DisplayClear();
        DisplayValue(0U, TotalCenter, 4U, 0U);
        DisplayGraph(1U, FrameBuffer.Values, 128U, 3U);
        DisplayRefresh();
    }
}
	#endif

#ifdef __cplusplus
}
#endif
/** @} */
