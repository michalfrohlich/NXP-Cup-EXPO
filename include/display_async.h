#ifndef DISPLAY_ASYNC_H
#define DISPLAY_ASYNC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "PlatformTypes.h"
#include "Std_Types.h"
#include "CDD_I2c.h"
#include "display.h"

#define DISPLAY_ASYNC_FRAME_DATA_BYTES   (CharacterRows * CharacterColumns * 8U)
#define DISPLAY_ASYNC_FRAME_TOTAL_BYTES  (DISPLAY_ASYNC_FRAME_DATA_BYTES + 1U)

void DisplayAsync_Init(uint8 I2cChannel);
void DisplayAsync_Service(void);
boolean DisplayAsync_IsBusy(void);
boolean DisplayAsync_IsHealthy(void);

I2c_DataType * DisplayAsync_GetBackBufferData(void);
Std_ReturnType DisplayAsync_QueueBackBuffer(void);
Std_ReturnType DisplayAsync_SubmitFrame(const I2c_DataType *Data, uint16 Length);

#ifdef __cplusplus
}
#endif

#endif
