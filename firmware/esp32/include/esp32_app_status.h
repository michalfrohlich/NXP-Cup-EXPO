#ifndef ESP32_APP_STATUS_H
#define ESP32_APP_STATUS_H

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    bool sw2Pressed;
    bool sw3Pressed;
    bool swPcbOn;
    uint8_t sequence;
    uint16_t timestampMs;
    uint32_t rxFrames;
    uint32_t protocolErrors;
    uint32_t ackErrors;
    bool wifiEnabled;
    bool wifiConnected;
} EspAppStatus_t;

#endif /* ESP32_APP_STATUS_H */
