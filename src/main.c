
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
#include "camera_emulator.h" //for testng
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
#include "vision_linear.h"

#include "S32K144.h"
/*==================================================================================================
 *                                       LOCAL MACROS
==================================================================================================*/
#define TESTS_ENABLE     0 /* All macros are here - used for testing */
	/* Test mode flags – enable only one at a time (disable if TESTS_ENABLE = 0 */
	#define DISPLAY_TEST      0   /* Camera signal simulation + display + buttons + potentiometer demo */
	#define ULTRASONIC_500MS_SAMPLE    0 //working example of using ultrasonic

#define CAR_MAIN         1   /* Line-following car main code that will run the whole car*/

/*==================================================================================================
 *                 SYSTEM INITIALIZATION - shared initialization for both test and car mains
==================================================================================================*/

static void System_Init(void)
{
	/*Initialize RTD drivers with the compiled configurations*/
	DriversInit();

	/*Initialize driver for timers used for timings in the main loop*/
	Timebase_Init();

	/*Initialize driver for the onboard potentiometer used for user input*/
	OnboardPot_Init();

	/* Initialize ultrasonic driver (TRIG low, ICU notification, internal state) */
	Ultrasonic_Init();     /* uses Dio + Icu, so it must come after DriversInit */

	/* Buttons driver:
	     *  - Uses Dio pins configured in the RTD (PTC12, PTC13).
	     *  - Has no explicit init function; its state is zero-initialized at startup.
	     *  - You MUST call Buttons_Update() periodically in the main loop
	     *    before using Buttons_IsPressed() / Buttons_WasPressed().
	     */

	/*Initialize Esc driver*/
	/*First parameter: The Pwm Channel that was configured in Peripherals tool for the Esc*/
	/*Next parameters: Amount of Pwm ticks needed to achieve 1ms, 1.5ms and 2ms long signals, standard for controlling an esc.*/
	EscInit(0U, 1638U, 2457U, 3276U);

	/*Initialize Servo driver*/
	/*First parameter: The Pwm Channel that was configured in Peripherals tool for the Servo*/
	/*Next parameters: Amount of Pwm ticks needed to achieve maximum desired left turn, right turn and middle position for your servo.*/
	ServoInit(1U, 3300U, 1700U, 2500U);

	/*Initialize Hbridge driver*/
	/*First two parameters: The Pwm Channels that were configured in Peripherals tool for the motors' speeds*/
	/*Next parameters: The Pcr of the pins used for the motors' directions*/
	HbridgeInit(2U, 3U, 32U, 33U, 6U, 64U);

	/*Initialize display driver*/
	/*The display driver is for a 0.96", 128x32 OLED monochrome using a SSD1306 chip, like the one in the MR-CANHUBK344 kit*/
	/*Parameter: Should the driver rotate the fontmap before printing. You should leave this to 1U unless you have a special use case*/
	DisplayInit(0U, STD_ON);

	/*Initialize linear camera driver*/
	/*First parameter: The Pwm channel that was configured in Peripherals tool for the clock sent to the camera*/
	/*Second parameter: The Gpt channel that was configured in Peripherals tool for determining the length of the shutter signal*/
	/*Third parameter: The Adc group that was configured in Peripherals tool for sampling the analog signal output of the camera*/
	/*Fourth parameter: The Pcr of the pin used for sending the shutter signal to the camera*/
	LinearCameraInit(4U, 1U, 0U, 97U);

	/*Initialize Pixy2 camera driver (optional)*/
	/*Pixy2Init(0x54, 0U);*/
	/*Pixy2Test();*/

	/* Force PTD0 (BLUE LED) OFF to indicate end of initialization */
	/* Set PTD0 as GPIO output, HIGH → LED off (active-low) */
	IP_PORTD->PCR[0] = PORT_PCR_MUX(1);      /* MUX = 1 → GPIO */
	IP_PTD->PDDR     |= (1UL << 0);          /* PTD0 as output */
	IP_PTD->PSOR     =  (1UL << 0);          /* set PTD0 high → LED off */
}

/*==================================================================================================
 *                                 MAIN CODE FOR THE CAR
==================================================================================================*/
#if CAR_MAIN
/*------------------------------------------------------------------------------------------------
 *                                 DEFINES
-------------------------------------------------------------------------------------------------*/

/* Vision-related globals used by main */
BlackRegion BlackRegionBuffer[25];
uint8 RegionIndex;
float TotalCenter;
#define MID_POINT   (VISION_BUFFER_SIZE / 2U)

/*------------------------------------------------------------------------------------------------
 *                                   MAIN
-------------------------------------------------------------------------------------------------*/
int main(void){

	System_Init(); //initialize everything

	LinearCameraFrame FrameBuffer;

    HbridgeSetSpeed(50);

    /* Force PTD0 (BLUE LED) OFF to indiate end of initialization */
	/* Set PTD0 as GPIO output, HIGH → LED off (active-low) */
	IP_PORTD->PCR[0] = PORT_PCR_MUX(1);      /* MUX = 1 → GPIO */
	IP_PTD->PDDR     |= (1UL << 0);          /* PTD0 as output */
	IP_PTD->PSOR     =  (1UL << 0);          /* set PTD0 high → LED off */

    /*------------------------------------------------------------------------------------------------
     *                                 LOOP FOEREVER
    -------------------------------------------------------------------------------------------------*/
    while(1){
        LinearCameraGetFrame(&FrameBuffer);
        /* The black regions are the values in the CameraResultsBuffer < a chosen threshold */
        /* The threshold must be chosen manually depending on how you calibrated the camera */
        RegionIndex = Vision_FindBlackRegions(FrameBuffer.Values, BlackRegionBuffer, 3U);
		/* Take the middle of all the black regions detected */
        TotalCenter = Vision_CalculateCenter(BlackRegionBuffer, RegionIndex);

        switch (RegionIndex)
        {
            case 0U:
                /* No line: choose a safe behavior (example: straight, maybe slow) */
                Steer(0);
                break;

            case 1U:
                if (TotalCenter < MID_POINT)
                {
                    Steer(-50);   /* line on left side → steer left */
                }
                else
                {
                    Steer(50);    /* line on right side → steer right */
                }
                break;

            case 2U:
            {
                /* Convert center to a steering command in -100..100 */
                float raw = 2.0f * (TotalCenter - (float)MID_POINT);  /* simple proportional rule */

                sint16 steer = (sint16)raw;
                if (steer > 100)  { steer = 100; }
                if (steer < -100) { steer = -100; }

                Steer(steer);
                break;
            }

            default:
                /* More than 2 regions – treat as ambiguous; keep straight for now */
                Steer(0);
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


/*==================================================================================================
 *                                       TESTS - must be enabled by the macros above
==================================================================================================*/
#if TESTS_ENABLE
int main(void)
{
	System_Init(); //initialize everything
/*==================================================================================================
*                                  ULTRASONIC TEST
==================================================================================================*/

#if ULTRASONIC_500MS_SAMPLE

    /* 16-char fixed lines (padded) — safe for your DisplayText() */
    const char L0[] = "ULTRA 500ms     ";  /* 16 */
    const char L1[] = "S:     I:      ";  /* 16 */
    const char L2[] = "cm:    x:      ";  /* 16 */
    const char L3[] = "t:     a:      ";  /* 16 */

    uint32 lastTrigMs     = Timebase_GetMs();
    float  lastDistanceCm = 0.0f;

    for (;;)
    {
        uint32 nowMs = Timebase_GetMs();

        /* Trigger every 500ms, but do not interrupt a BUSY measurement */
        if ((uint32)(nowMs - lastTrigMs) >= 500u)
        {
            if (Ultrasonic_GetStatus() != ULTRA_STATUS_BUSY)
            {
                lastTrigMs = nowMs;
                Ultrasonic_StartMeasurement();
            }
        }

        /* Let the driver progress/timeout */
        Ultrasonic_Task();

        /* Read status for display */
        Ultrasonic_StatusType st = Ultrasonic_GetStatus();

        /* consume valid and set invalid to max+1 */
        float d;
        if (Ultrasonic_GetDistanceCm(&d) == TRUE)
        {
            lastDistanceCm = d;
        }
        else if (st == ULTRA_STATUS_ERROR)
        {
            lastDistanceCm = ULTRA_MAX_DISTANCE_CM + 1.0f;  /* “no obstacle” sentinel */
        }

        /* Debug getters */
        uint32 irqCnt = Ultrasonic_GetIrqCount();
        uint32 ticks  = Ultrasonic_GetLastHighTicks();
        uint16 idx    = Ultrasonic_GetTsIndex();

        /* Age since last trigger (ms) */
        uint32 ageMs = (uint32)(nowMs - lastTrigMs);

        /* ----- OLED ----- */
        DisplayClear();

        DisplayText(0U, L0, 16U, 0U);

        /* Line 1: status + IRQ count */
        DisplayText(1U, L1, 16U, 0U);
        if (st == ULTRA_STATUS_NEW_DATA)      { DisplayText(1U, "NEW  ", 5U, 2U); }
        else if (st == ULTRA_STATUS_BUSY)    { DisplayText(1U, "BUSY ", 5U, 2U); }
        else if (st == ULTRA_STATUS_ERROR)   { DisplayText(1U, "ERR  ", 5U, 2U); }
        else                                 { DisplayText(1U, "IDLE ", 5U, 2U); }
        DisplayValue(1U, (int)irqCnt, 6U, 10U);   /* I: at col 10..15 */

        /* Line 2: distance + timestamp index */
        DisplayText(2U, L2, 16U, 0U);
        {
            int distInt = (int)(lastDistanceCm + 0.5f);
            DisplayValue(2U, distInt, 4U, 3U);     /* cm at col 3..6 */
        }
        DisplayValue(2U, (int)idx, 6U, 10U);       /* x: at col 10..15 */

        /* Line 3: ticks + age */
        DisplayText(3U, L3, 16U, 0U);
        DisplayValue(3U, (int)ticks, 5U, 2U);      /* t: at col 2..6 */
        DisplayValue(3U, (int)ageMs, 6U, 10U);     /* a: at col 10..15 */

        DisplayRefresh();
    }

#endif /* ULTRASONIC_500MS_TEST */

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

#endif /* TESTS_ENABLE */
