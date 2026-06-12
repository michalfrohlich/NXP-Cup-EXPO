#ifndef ESP_S32K_UART_PROTOCOL_H
#define ESP_S32K_UART_PROTOCOL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*
 * S32DS generates LPUART2 at 470588 baud from the configured 8 MHz source.
 * Both peers must use the generated wire rate, not the nominal preset name.
 */
#define ESP_S32K_UART_BAUD_RATE (470588U)

#define ESP_S32K_FRAME_START ('#')
#define ESP_S32K_FRAME_END ('_')

#define ESP_S32K_TUNE_FRAME_LEN (48U)
#define ESP_S32K_TUNE_RESULT_FRAME_LEN (9U)
#define ESP_S32K_TUNE_GAIN_MILLI_MAX (99999U)
#define ESP_S32K_TUNE_STEER_CLAMP_MAX (100U)
#define ESP_S32K_TUNE_LPF_MILLI_MAX (1000U)
#define ESP_S32K_TUNE_MIN_CONTRAST_MAX (4095U)
#define ESP_S32K_TUNE_EDGE_PCT_MIN (1U)
#define ESP_S32K_TUNE_EDGE_PCT_MAX (100U)

#define ESP_S32K_BUTTON_FRAME_LEN (17U)
#define ESP_S32K_ACK_FRAME_LEN (17U)
#define ESP_S32K_BUTTON_SEQ_MAX (99U)
#define ESP_S32K_TIME_MODULO_MS (10000U)

/* S32K -> ESP: #BxyzQnnTmmmmXcc_
 * ESP -> S32K: #AxyzQnnTmmmmXcc_
 *
 * x = SW2 pressed, y = SW3 pressed, z = swPcb on,
 * nn = decimal sequence 00..99, mmmm = S32K timestamp modulo 10000 ms,
 * cc = CRC-8/ATM over bytes between '#' and 'X'.
 */

typedef struct
{
    uint8_t sequence;
    uint32_t proportionalMilli;
    uint32_t integralMilli;
    uint32_t derivativeMilli;
    uint8_t steerClamp;
    uint16_t steerLpfMilli;
    uint16_t minContrast;
    uint8_t edgeHighPct;
    uint8_t edgeLowPct;
} EspS32kTuneFrame_t;

typedef struct
{
    uint8_t sequence;
    bool accepted;
} EspS32kTuneResultFrame_t;

typedef struct
{
    bool sw2Pressed;
    bool sw3Pressed;
    bool swPcbOn;
    uint8_t sequence;
    uint16_t timestampMs;
} EspS32kButtonFrame_t;

typedef EspS32kButtonFrame_t EspS32kAckFrame_t;

static inline uint8_t EspS32k_BoolDigit(bool value)
{
    return (uint8_t)(value ? '1' : '0');
}

static inline bool EspS32k_DecodeBoolDigit(uint8_t digit, bool *outValue)
{
    if (outValue == NULL)
    {
        return false;
    }

    if (digit == (uint8_t)'0')
    {
        *outValue = false;
        return true;
    }

    if (digit == (uint8_t)'1')
    {
        *outValue = true;
        return true;
    }

    return false;
}

static inline uint8_t EspS32k_NormalizeSeq(uint8_t sequence)
{
    return (uint8_t)(sequence % (ESP_S32K_BUTTON_SEQ_MAX + 1U));
}

static inline bool EspS32k_EncodeDecimal(uint32_t value, uint8_t *out, size_t width)
{
    size_t index;
    uint32_t limit = 1U;

    if ((out == NULL) || (width == 0U))
    {
        return false;
    }

    for (index = 0U; index < width; index++)
    {
        limit *= 10U;
    }
    if (value >= limit)
    {
        return false;
    }

    for (index = width; index > 0U; index--)
    {
        out[index - 1U] = (uint8_t)('0' + (value % 10U));
        value /= 10U;
    }
    return true;
}

static inline bool EspS32k_DecodeDecimal(const uint8_t *digits, size_t width, uint32_t *outValue)
{
    size_t index;
    uint32_t value = 0U;

    if ((digits == NULL) || (outValue == NULL) || (width == 0U))
    {
        return false;
    }

    for (index = 0U; index < width; index++)
    {
        if ((digits[index] < (uint8_t)'0') || (digits[index] > (uint8_t)'9'))
        {
            return false;
        }
        value = (value * 10U) + (uint32_t)(digits[index] - (uint8_t)'0');
    }

    *outValue = value;
    return true;
}

static inline uint8_t EspS32k_HexDigit(uint8_t value)
{
    value &= 0x0FU;
    return (value < 10U) ? (uint8_t)('0' + value) : (uint8_t)('A' + (value - 10U));
}

static inline bool EspS32k_DecodeHexDigit(uint8_t digit, uint8_t *outValue)
{
    if (outValue == NULL)
    {
        return false;
    }
    if ((digit >= (uint8_t)'0') && (digit <= (uint8_t)'9'))
    {
        *outValue = (uint8_t)(digit - (uint8_t)'0');
        return true;
    }
    if ((digit >= (uint8_t)'A') && (digit <= (uint8_t)'F'))
    {
        *outValue = (uint8_t)(digit - (uint8_t)'A' + 10U);
        return true;
    }
    return false;
}

static inline uint8_t EspS32k_FrameCrc8(const uint8_t *frame, size_t start, size_t end)
{
    uint8_t crc = 0U;
    size_t index;
    uint8_t bit;

    if (frame == NULL)
    {
        return 0U;
    }

    for (index = start; index < end; index++)
    {
        crc ^= frame[index];
        for (bit = 0U; bit < 8U; bit++)
        {
            crc = ((crc & 0x80U) != 0U) ? (uint8_t)((crc << 1U) ^ 0x07U) : (uint8_t)(crc << 1U);
        }
    }
    return crc;
}

static inline bool EspS32k_EncodeButtonLikeFrame(uint8_t type, const EspS32kButtonFrame_t *buttons,
                                                 uint8_t *frame, size_t frameLen)
{
    uint8_t seq;
    uint8_t checksum;
    uint16_t timestamp;

    if ((buttons == NULL) || (frame == NULL) || (frameLen < ESP_S32K_BUTTON_FRAME_LEN))
    {
        return false;
    }

    seq = EspS32k_NormalizeSeq(buttons->sequence);
    timestamp = (uint16_t)(buttons->timestampMs % ESP_S32K_TIME_MODULO_MS);
    frame[0] = (uint8_t)ESP_S32K_FRAME_START;
    frame[1] = type;
    frame[2] = EspS32k_BoolDigit(buttons->sw2Pressed);
    frame[3] = EspS32k_BoolDigit(buttons->sw3Pressed);
    frame[4] = EspS32k_BoolDigit(buttons->swPcbOn);
    frame[5] = (uint8_t)'Q';
    frame[6] = (uint8_t)('0' + (seq / 10U));
    frame[7] = (uint8_t)('0' + (seq % 10U));
    frame[8] = (uint8_t)'T';
    frame[9] = (uint8_t)('0' + (timestamp / 1000U));
    frame[10] = (uint8_t)('0' + ((timestamp / 100U) % 10U));
    frame[11] = (uint8_t)('0' + ((timestamp / 10U) % 10U));
    frame[12] = (uint8_t)('0' + (timestamp % 10U));
    frame[13] = (uint8_t)'X';
    checksum = EspS32k_FrameCrc8(frame, 1U, 13U);
    frame[14] = EspS32k_HexDigit((uint8_t)(checksum >> 4U));
    frame[15] = EspS32k_HexDigit(checksum);
    frame[16] = (uint8_t)ESP_S32K_FRAME_END;

    return true;
}

static inline bool EspS32k_DecodeButtonLikeFrame(uint8_t expectedType, const uint8_t *frame,
                                                 size_t frameLen, EspS32kButtonFrame_t *outButtons)
{
    bool sw2;
    bool sw3;
    bool swPcb;
    uint8_t seqTens;
    uint8_t seqOnes;
    uint8_t checksumHigh;
    uint8_t checksumLow;
    uint16_t timestamp;

    if ((frame == NULL) || (outButtons == NULL) || (frameLen != ESP_S32K_BUTTON_FRAME_LEN))
    {
        return false;
    }

    if ((frame[0] != (uint8_t)ESP_S32K_FRAME_START) || (frame[1] != expectedType) ||
        (frame[5] != (uint8_t)'Q') || (frame[8] != (uint8_t)'T') || (frame[13] != (uint8_t)'X') ||
        (frame[16] != (uint8_t)ESP_S32K_FRAME_END))
    {
        return false;
    }

    if ((EspS32k_DecodeBoolDigit(frame[2], &sw2) != true) ||
        (EspS32k_DecodeBoolDigit(frame[3], &sw3) != true) ||
        (EspS32k_DecodeBoolDigit(frame[4], &swPcb) != true))
    {
        return false;
    }

    if ((frame[6] < (uint8_t)'0') || (frame[6] > (uint8_t)'9') || (frame[7] < (uint8_t)'0') ||
        (frame[7] > (uint8_t)'9') || (frame[9] < (uint8_t)'0') || (frame[9] > (uint8_t)'9') ||
        (frame[10] < (uint8_t)'0') || (frame[10] > (uint8_t)'9') || (frame[11] < (uint8_t)'0') ||
        (frame[11] > (uint8_t)'9') || (frame[12] < (uint8_t)'0') || (frame[12] > (uint8_t)'9') ||
        !EspS32k_DecodeHexDigit(frame[14], &checksumHigh) ||
        !EspS32k_DecodeHexDigit(frame[15], &checksumLow) ||
        (((uint8_t)(checksumHigh << 4U) | checksumLow) != EspS32k_FrameCrc8(frame, 1U, 13U)))
    {
        return false;
    }

    seqTens = (uint8_t)(frame[6] - (uint8_t)'0');
    seqOnes = (uint8_t)(frame[7] - (uint8_t)'0');
    timestamp = (uint16_t)(((uint16_t)(frame[9] - (uint8_t)'0') * 1000U) +
                           ((uint16_t)(frame[10] - (uint8_t)'0') * 100U) +
                           ((uint16_t)(frame[11] - (uint8_t)'0') * 10U) +
                           (uint16_t)(frame[12] - (uint8_t)'0'));

    outButtons->sw2Pressed = sw2;
    outButtons->sw3Pressed = sw3;
    outButtons->swPcbOn = swPcb;
    outButtons->sequence = (uint8_t)((seqTens * 10U) + seqOnes);
    outButtons->timestampMs = timestamp;
    return true;
}

static inline bool EspS32kButtonFrame_Encode(const EspS32kButtonFrame_t *buttons, uint8_t *frame,
                                             size_t frameLen)
{
    return EspS32k_EncodeButtonLikeFrame((uint8_t)'B', buttons, frame, frameLen);
}

static inline bool EspS32kButtonFrame_Decode(const uint8_t *frame, size_t frameLen,
                                             EspS32kButtonFrame_t *outButtons)
{
    return EspS32k_DecodeButtonLikeFrame((uint8_t)'B', frame, frameLen, outButtons);
}

static inline bool EspS32kAckFrame_Encode(const EspS32kAckFrame_t *ack, uint8_t *frame,
                                          size_t frameLen)
{
    return EspS32k_EncodeButtonLikeFrame((uint8_t)'A', ack, frame, frameLen);
}

static inline bool EspS32kAckFrame_Decode(const uint8_t *frame, size_t frameLen,
                                          EspS32kAckFrame_t *outAck)
{
    return EspS32k_DecodeButtonLikeFrame((uint8_t)'A', frame, frameLen, outAck);
}

static inline bool EspS32kTuneFrame_Encode(const EspS32kTuneFrame_t *tune, uint8_t *frame,
                                           size_t frameLen)
{
    uint8_t checksum;

    if ((tune == NULL) || (frame == NULL) || (frameLen < ESP_S32K_TUNE_FRAME_LEN) ||
        (tune->proportionalMilli > ESP_S32K_TUNE_GAIN_MILLI_MAX) ||
        (tune->integralMilli > ESP_S32K_TUNE_GAIN_MILLI_MAX) ||
        (tune->derivativeMilli > ESP_S32K_TUNE_GAIN_MILLI_MAX) ||
        (tune->steerClamp > ESP_S32K_TUNE_STEER_CLAMP_MAX) ||
        (tune->steerLpfMilli > ESP_S32K_TUNE_LPF_MILLI_MAX) ||
        (tune->minContrast > ESP_S32K_TUNE_MIN_CONTRAST_MAX) ||
        (tune->edgeHighPct < ESP_S32K_TUNE_EDGE_PCT_MIN) ||
        (tune->edgeHighPct > ESP_S32K_TUNE_EDGE_PCT_MAX) ||
        (tune->edgeLowPct < ESP_S32K_TUNE_EDGE_PCT_MIN) ||
        (tune->edgeLowPct > ESP_S32K_TUNE_EDGE_PCT_MAX))
    {
        return false;
    }

    frame[0] = (uint8_t)ESP_S32K_FRAME_START;
    frame[1] = (uint8_t)'T';
    if (!EspS32k_EncodeDecimal(EspS32k_NormalizeSeq(tune->sequence), &frame[2], 2U) ||
        !EspS32k_EncodeDecimal(tune->proportionalMilli, &frame[5], 5U) ||
        !EspS32k_EncodeDecimal(tune->integralMilli, &frame[11], 5U) ||
        !EspS32k_EncodeDecimal(tune->derivativeMilli, &frame[17], 5U) ||
        !EspS32k_EncodeDecimal(tune->steerClamp, &frame[23], 3U) ||
        !EspS32k_EncodeDecimal(tune->steerLpfMilli, &frame[27], 4U) ||
        !EspS32k_EncodeDecimal(tune->minContrast, &frame[32], 4U) ||
        !EspS32k_EncodeDecimal(tune->edgeHighPct, &frame[37], 3U) ||
        !EspS32k_EncodeDecimal(tune->edgeLowPct, &frame[41], 3U))
    {
        return false;
    }

    frame[4] = (uint8_t)'P';
    frame[10] = (uint8_t)'I';
    frame[16] = (uint8_t)'D';
    frame[22] = (uint8_t)'C';
    frame[26] = (uint8_t)'L';
    frame[31] = (uint8_t)'M';
    frame[36] = (uint8_t)'H';
    frame[40] = (uint8_t)'E';
    frame[44] = (uint8_t)'X';
    checksum = EspS32k_FrameCrc8(frame, 1U, 44U);
    frame[45] = EspS32k_HexDigit((uint8_t)(checksum >> 4U));
    frame[46] = EspS32k_HexDigit(checksum);
    frame[47] = (uint8_t)ESP_S32K_FRAME_END;
    return true;
}

static inline bool EspS32kTuneFrame_Decode(const uint8_t *frame, size_t frameLen,
                                           EspS32kTuneFrame_t *outTune)
{
    uint32_t sequence;
    uint32_t steerClamp;
    uint32_t steerLpf;
    uint32_t minContrast;
    uint32_t edgeHigh;
    uint32_t edgeLow;
    uint8_t checksumHigh;
    uint8_t checksumLow;

    if ((frame == NULL) || (outTune == NULL) || (frameLen != ESP_S32K_TUNE_FRAME_LEN) ||
        (frame[0] != (uint8_t)ESP_S32K_FRAME_START) || (frame[1] != (uint8_t)'T') ||
        (frame[4] != (uint8_t)'P') || (frame[10] != (uint8_t)'I') || (frame[16] != (uint8_t)'D') ||
        (frame[22] != (uint8_t)'C') || (frame[26] != (uint8_t)'L') || (frame[31] != (uint8_t)'M') ||
        (frame[36] != (uint8_t)'H') || (frame[40] != (uint8_t)'E') || (frame[44] != (uint8_t)'X') ||
        (frame[47] != (uint8_t)ESP_S32K_FRAME_END))
    {
        return false;
    }

    if (!EspS32k_DecodeDecimal(&frame[2], 2U, &sequence) ||
        !EspS32k_DecodeDecimal(&frame[5], 5U, &outTune->proportionalMilli) ||
        !EspS32k_DecodeDecimal(&frame[11], 5U, &outTune->integralMilli) ||
        !EspS32k_DecodeDecimal(&frame[17], 5U, &outTune->derivativeMilli) ||
        !EspS32k_DecodeDecimal(&frame[23], 3U, &steerClamp) ||
        !EspS32k_DecodeDecimal(&frame[27], 4U, &steerLpf) ||
        !EspS32k_DecodeDecimal(&frame[32], 4U, &minContrast) ||
        !EspS32k_DecodeDecimal(&frame[37], 3U, &edgeHigh) ||
        !EspS32k_DecodeDecimal(&frame[41], 3U, &edgeLow) ||
        !EspS32k_DecodeHexDigit(frame[45], &checksumHigh) ||
        !EspS32k_DecodeHexDigit(frame[46], &checksumLow))
    {
        return false;
    }

    if ((((uint8_t)(checksumHigh << 4U) | checksumLow) != EspS32k_FrameCrc8(frame, 1U, 44U)) ||
        (outTune->proportionalMilli > ESP_S32K_TUNE_GAIN_MILLI_MAX) ||
        (outTune->integralMilli > ESP_S32K_TUNE_GAIN_MILLI_MAX) ||
        (outTune->derivativeMilli > ESP_S32K_TUNE_GAIN_MILLI_MAX) ||
        (steerClamp > ESP_S32K_TUNE_STEER_CLAMP_MAX) || (steerLpf > ESP_S32K_TUNE_LPF_MILLI_MAX) ||
        (minContrast > ESP_S32K_TUNE_MIN_CONTRAST_MAX) || (edgeHigh < ESP_S32K_TUNE_EDGE_PCT_MIN) ||
        (edgeHigh > ESP_S32K_TUNE_EDGE_PCT_MAX) || (edgeLow < ESP_S32K_TUNE_EDGE_PCT_MIN) ||
        (edgeLow > ESP_S32K_TUNE_EDGE_PCT_MAX))
    {
        return false;
    }

    outTune->sequence = (uint8_t)sequence;
    outTune->steerClamp = (uint8_t)steerClamp;
    outTune->steerLpfMilli = (uint16_t)steerLpf;
    outTune->minContrast = (uint16_t)minContrast;
    outTune->edgeHighPct = (uint8_t)edgeHigh;
    outTune->edgeLowPct = (uint8_t)edgeLow;
    return true;
}

static inline bool EspS32kTuneResultFrame_Encode(const EspS32kTuneResultFrame_t *result,
                                                 uint8_t *frame, size_t frameLen)
{
    uint8_t checksum;

    if ((result == NULL) || (frame == NULL) || (frameLen < ESP_S32K_TUNE_RESULT_FRAME_LEN))
    {
        return false;
    }

    frame[0] = (uint8_t)ESP_S32K_FRAME_START;
    frame[1] = (uint8_t)'R';
    if (!EspS32k_EncodeDecimal(EspS32k_NormalizeSeq(result->sequence), &frame[2], 2U))
    {
        return false;
    }
    frame[4] = result->accepted ? (uint8_t)'O' : (uint8_t)'E';
    frame[5] = (uint8_t)'X';
    checksum = EspS32k_FrameCrc8(frame, 1U, 5U);
    frame[6] = EspS32k_HexDigit((uint8_t)(checksum >> 4U));
    frame[7] = EspS32k_HexDigit(checksum);
    frame[8] = (uint8_t)ESP_S32K_FRAME_END;
    return true;
}

static inline bool EspS32kTuneResultFrame_Decode(const uint8_t *frame, size_t frameLen,
                                                 EspS32kTuneResultFrame_t *outResult)
{
    uint32_t sequence;
    uint8_t checksumHigh;
    uint8_t checksumLow;

    if ((frame == NULL) || (outResult == NULL) || (frameLen != ESP_S32K_TUNE_RESULT_FRAME_LEN) ||
        (frame[0] != (uint8_t)ESP_S32K_FRAME_START) || (frame[1] != (uint8_t)'R') ||
        ((frame[4] != (uint8_t)'O') && (frame[4] != (uint8_t)'E')) || (frame[5] != (uint8_t)'X') ||
        (frame[8] != (uint8_t)ESP_S32K_FRAME_END) ||
        !EspS32k_DecodeDecimal(&frame[2], 2U, &sequence) ||
        !EspS32k_DecodeHexDigit(frame[6], &checksumHigh) ||
        !EspS32k_DecodeHexDigit(frame[7], &checksumLow) ||
        (((uint8_t)(checksumHigh << 4U) | checksumLow) != EspS32k_FrameCrc8(frame, 1U, 5U)))
    {
        return false;
    }

    outResult->sequence = (uint8_t)sequence;
    outResult->accepted = (frame[4] == (uint8_t)'O');
    return true;
}

#endif /* ESP_S32K_UART_PROTOCOL_H */
