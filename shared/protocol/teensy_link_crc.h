#ifndef TEENSY_LINK_CRC_H
#define TEENSY_LINK_CRC_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline uint16_t TeensyLink_Crc16CcittFalse(const uint8_t *bytes, uint16_t length)
{
    uint16_t crc = 0xFFFFu;

    for (uint16_t i = 0u; i < length; i++)
    {
        crc ^= (uint16_t)((uint16_t)bytes[i] << 8u);

        for (uint8_t bit = 0u; bit < 8u; bit++)
        {
            if ((crc & 0x8000u) != 0u)
            {
                crc = (uint16_t)((crc << 1u) ^ 0x1021u);
            }
            else
            {
                crc = (uint16_t)(crc << 1u);
            }
        }
    }

    return crc;
}

#ifdef __cplusplus
}
#endif

#endif /* TEENSY_LINK_CRC_H */
