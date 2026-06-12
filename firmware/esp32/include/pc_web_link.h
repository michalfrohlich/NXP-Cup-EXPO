#ifndef PC_WEB_LINK_H
#define PC_WEB_LINK_H

#include "esp_err.h"
#include "esp32_app_status.h"
#include "esp_s32k_uart_protocol.h"

typedef esp_err_t (*PcWebLink_PidSubmitFn)(const EspS32kPidFrame_t *pid, void *context);

esp_err_t PcWebLink_Init(PcWebLink_PidSubmitFn submitPid, void *context);
void PcWebLink_GetStatus(EspAppStatus_t *outStatus);

#endif /* PC_WEB_LINK_H */
