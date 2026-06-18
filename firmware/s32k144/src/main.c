#ifdef __cplusplus
extern "C"
{
#endif

#include "app/app_modes.h"

    int main(void)
    {
        App_RunBootBanner();

        /* Run selected mode forever */
        App_RunSelectedMode();

        /* never reached */
        while (1)
        {
        }
    }

#ifdef __cplusplus
}
#endif
