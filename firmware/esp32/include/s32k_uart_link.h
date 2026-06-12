#ifndef S32K_UART_LINK_H
#define S32K_UART_LINK_H

#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"
#include "esp32_app_status.h"
#include "esp_s32k_uart_protocol.h"

esp_err_t S32kUartLink_Init(void);
esp_err_t S32kUartLink_StartTask(void);

esp_err_t S32kUartLink_SendTuneAndWait(EspS32kTuneFrame_t *tune);
esp_err_t S32kUartLink_Read(uint8_t *data, size_t maxLength, size_t *outRead);
void S32kUartLink_Service(void);
void S32kUartLink_GetStatus(EspAppStatus_t *outStatus);

#endif /* S32K_UART_LINK_H */
