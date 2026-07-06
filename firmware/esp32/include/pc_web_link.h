#ifndef PC_WEB_LINK_H
#define PC_WEB_LINK_H

#include "esp_err.h"
#include "esp32_app_status.h"
#include "esp_s32k_uart_protocol.h"

typedef esp_err_t (*PcWebLink_TuneSubmitFn)(EspS32kTuneFrame_t *tune, void *context);
typedef esp_err_t (*PcWebLink_DriveCommandSubmitFn)(EspS32kDriveCommandFrame_t *command,
                                                    void *context);

esp_err_t PcWebLink_Init(PcWebLink_TuneSubmitFn submitTune,
                         PcWebLink_DriveCommandSubmitFn submitDriveCommand, void *context);
void PcWebLink_GetStatus(EspAppStatus_t *outStatus);

#endif /* PC_WEB_LINK_H */
