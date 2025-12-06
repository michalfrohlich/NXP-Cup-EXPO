/*Nxp_Cup_Linear_Camera_S32K144 - main file */

#ifdef __cplusplus
extern "C" {
#endif

/*==================================================================================================
 *                                        INCLUDE FILES
 * 1) system and project includes
 * 2) needed interfaces from external units
 * 3) internal and external interfaces from this unit
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

#include "Icu.h"

#include "S32K144.h" //bare-metal pot test


/*==================================================================================================
 *                          LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
==================================================================================================*/

/*==================================================================================================
 *                                       LOCAL MACROS
==================================================================================================*/
/* Test mode flags – enable only one at a time */
#define DISPLAY_TEST      0   /* Camera + display demo */
#define CAR_TEST          0   /* Line-following car demo */
#define ULTRASONIC_TEST   1		   /* Ultrasonic distance test */

/*==================================================================================================
 *                                      LOCAL CONSTANTS
==================================================================================================*/

/*==================================================================================================
 *                                      LOCAL VARIABLES
==================================================================================================*/
static uint32 EmuFrameCount = 0U; //number of times the graph was updated
/*==================================================================================================
 *                                      GLOBAL CONSTANTS
==================================================================================================*/

/*==================================================================================================
 *                                      GLOBAL VARIABLES
==================================================================================================*/
volatile boolean g_EmuNewFrameFlag = FALSE; //used for the GPT Channel 2 (EmuTimer)
volatile boolean g_GptChannel3Flag = FALSE;
static uint16 EmuTicks = 0U;
/*==================================================================================================
 *                                   LOCAL FUNCTION PROTOTYPES
==================================================================================================*/

/*==================================================================================================
 *                                       LOCAL FUNCTIONS
==================================================================================================*/
void EmuTimer_Notification(void)
{
	EmuTicks++;
	if(EmuTicks >= 1000U){
		g_EmuNewFrameFlag = TRUE;
		EmuTicks = 0U;
	}

}
void GptChannel3_notification(void)
{
	g_GptChannel3Flag = TRUE;
}



/*==================================================================================================
 *                                       GLOBAL FUNCTIONS
==================================================================================================*/

/* ========== Linera camera code ==========*/
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

/*==================================================================================================
 *                                       MAIN
==================================================================================================*/

int main(void)
{
    LinearCameraFrame FrameBuffer;
    /*Initialize RTD drivers with the compiled configurations*/
     DriversInit();

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
    /*CameraTest();*/

    /*Initialize Pixy2 camera driver*/
    /*First parameter: The I2c address that was configured on the Pixy2 that the driver should communicate with*/
    /*Second parameter: The I2c channel that was configured in Peripherals tool for communication with the camera and display*/
    /*Pixy2Init(0x54, 0U);*/
    /*Pixy2Test();*/

    /* Initialize ultrasonic driver (HC-SR04 on PTA15/PTE15) */
    Ultrasonic_Init(&Ultrasonic_Config);

    /* Temporarily force PTD0 as GPIO high → LED off */
    /* Set PTD0 as GPIO output, HIGH → LED off (active-low) */
        IP_PORTD->PCR[0] = PORT_PCR_MUX(1);      /* MUX = 1 → GPIO */
        IP_PTD->PDDR     |= (1UL << 0);          /* PTD0 as output */
        IP_PTD->PSOR     =  (1UL << 0);          /* set PTD0 high → LED off */


/*==================================================================================================
 *                                  ULTRASONIC DISPLAY TEST
==================================================================================================*/

#if ULTRASONIC_TEST

    for (;;)
    {
        /* 1) Trigger a measurement (TRIG pulse + start ICU) */
        Ultrasonic_TriggerMeasurement();

        /* 2) Poll ICU for up to 100 ms */
        uint32 waitedUs = 0U;
        uint32 rawTicks = 0U;
        const uint32 timeoutUs = 100000U;  /* 100 ms */

        while (waitedUs < timeoutUs)
        {
            rawTicks = Icu_GetTimeElapsed(Ultrasonic_Config.EchoChannel);

            if (rawTicks != 0U)
            {
                break; /* got something */
            }

            /* cheap ~1 ms delay */
            volatile uint32 d = 50000U;
            while (d-- > 0U) { __asm("NOP"); }

            waitedUs += 1000U;
        }

        /* 3) Convert ticks -> distance (same formula as driver) */
        uint16 distanceCm;
        if (rawTicks == 0U)
        {
            distanceCm = ULTRASONIC_DISTANCE_INVALID_CM;
        }
        else
        {
            uint32 timeUs = (rawTicks * 1000000UL) / Ultrasonic_Config.TimerFreqHz;
            uint32 dist = (timeUs * 343UL + 20000UL / 2UL) / 20000UL;
            if (dist > 500UL) { dist = 500UL; }
            distanceCm = (uint16)dist;
        }

        /* 4) Show everything on the display */
        DisplayClear();

        /* Line 0: raw ticks */
        DisplayText(0U, "Ticks:", 6U, 0U);
        DisplayValue(0U, (uint16)(rawTicks & 0xFFFFU), 5U, 7U);

        /* Line 1: upper bits of ticks (if any) */
        DisplayText(1U, "TicksH:", 7U, 0U);
        DisplayValue(1U, (uint16)(rawTicks >> 16), 5U, 8U);

        /* Line 2: distance / timeout */
        if (distanceCm != ULTRASONIC_DISTANCE_INVALID_CM)
        {
            DisplayText(2U, "Dist:", 5U, 0U);
            DisplayValue(2U, distanceCm, 4U, 6U);
            DisplayText(2U, "cm", 2U, 11U);
        }
        else
        {
            DisplayText(2U, "TIMEOUT", 7U, 0U);
        }

        /* Line 3: debug info */
        DisplayText(3U, "wait[ms]:", 9U, 0U);
        DisplayValue(3U, (uint16)(waitedUs / 1000U), 3U, 10U);

        DisplayRefresh();

        /* 5) Small pause between measurements (~100 ms) */
        volatile uint32 pause = 10000000U;
        while (pause-- > 0U) { __asm("NOP"); }
    }

#endif /* ULTRASONIC_TEST */

/*==================================================================================================
 *                                       DISPLAY TEST
==================================================================================================*/

	#if DISPLAY_TEST
		uint8 SimPixels[CAMERA_EMU_NUM_PIXELS];
		uint8 GraphValues[CAMERA_EMU_NUM_PIXELS];

		CameraEmulator_Init();

		/*Set up the timer used for determining the frequency of receiving emulated data*/

		Gpt_StartTimer(2U, 8000U); //ticks = 8e6 * seconds -> 1ms = 8000
		Gpt_EnableNotification(2U); //enable channel's 2 notification

		//Gpt_StartTimer(3U, 8000U); //ticks = 8e6 * seconds -> 1ms = 8000
		//Gpt_EnableNotification(3U); //enable channel's 3 notification

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
	#endif

/*==================================================================================================
 *                                       CAR TEST = RUN STATE
==================================================================================================*/

	#if CAR_TEST

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
	#endif
}



#ifdef __cplusplus
}
#endif
/** @} */
