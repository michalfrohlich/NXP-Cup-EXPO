#include "board_init.h"

#include "Mcu.h"
#include "Mcl.h"
#include "Platform.h"
#include "Port.h"
#include "CDD_I2c.h"
#include "Pwm.h"
#include "Icu.h"
#include "Gpt.h"
#include "Adc.h"

#include "timebase.h"
#include "onboard_pot.h"
#include "ultrasonic.h"
#include "esc.h"
#include "servo.h"
#include "display.h"
#include "linear_camera.h"
#include "rgb_led.h"

#include "car_config.h"

void Board_InitDrivers(void)
{
    uint8 Index;

#if (MCU_PRECOMPILE_SUPPORT == STD_ON)
    Mcu_Init(NULL_PTR);
#elif (MCU_PRECOMPILE_SUPPORT == STD_OFF)
    Mcu_Init(&Mcu_Config_VS_0);
#endif

    Mcu_InitClock(McuClockSettingConfig_0);
#if (MCU_NO_PLL == STD_OFF)
    while (MCU_PLL_LOCKED != Mcu_GetPllStatus())
    {
        /* Busy wait until the System PLL is locked. */
    }
    Mcu_DistributePllClock();
#endif
    Mcu_SetMode(McuModeSettingConf_0);

    Platform_Init(NULL_PTR);
    Port_Init(NULL_PTR);
    Mcl_Init(NULL_PTR);
    I2c_Init(NULL_PTR);
    Icu_Init(NULL_PTR);
    Gpt_Init(NULL_PTR);

    Adc_Init(NULL_PTR);
    for (Index = 0U; Index <= 5U; Index++)
    {
        Adc_CalibrationStatusType CalibStatus;

        Adc_Calibrate(0U, &CalibStatus);
        if (CalibStatus.AdcUnitSelfTestStatus == E_OK)
        {
            break;
        }
    }

    Pwm_Init(NULL_PTR);
}

void Board_InitCommonApp(void)
{
    Board_InitDrivers();
    Timebase_Init();
    OnboardPot_Init();
    Ultrasonic_Init();

    EscInit(ESC_PWM_CH, ESC_DUTY_MIN, ESC_DUTY_MED, ESC_DUTY_MAX);
    ServoInit(SERVO_PWM_CH, 3300U, 1700U, 2500U);

    DisplayInit(0U, STD_ON);
    LinearCameraInit(CAM_CLK_PWM_CH, CAM_SHUTTER_GPT_CH, CAM_ADC_GROUP, CAM_SHUTTER_PCR);

    RgbLed_ChangeColor((RgbLed_Color){ .r=true, .g=false, .b=false });
    for (volatile uint32 i = 0; i < 700000u; i++) { }
    RgbLed_ChangeColor((RgbLed_Color){ .r=false, .g=true, .b=false });
    for (volatile uint32 i = 0; i < 600000u; i++) { }
    RgbLed_ChangeColor((RgbLed_Color){ .r=false, .g=false, .b=true });
    for (volatile uint32 i = 0; i < 500000u; i++) { }
    RgbLed_ChangeColor((RgbLed_Color){ .r=false, .g=false, .b=false });
}
