#include <stdint.h>

uint32_t startup_getControlRegisterValue(void)
{
    uint32_t reg;
    __asm volatile ("MRS %0, CONTROL" : "=r" (reg));
    return reg;
}

uint32_t startup_getAipsRegisterValue(void)
{
    return 0;   // OK for most S32K144 projects
}
