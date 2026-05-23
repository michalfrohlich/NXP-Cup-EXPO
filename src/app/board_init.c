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

#include "debug/serial_debug.h"

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

    SerialDebug_Init();
}
