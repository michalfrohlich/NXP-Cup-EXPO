#ifndef ESP_S32K_UART_PROTOCOL_H
#define ESP_S32K_UART_PROTOCOL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define ESP_S32K_UART_BAUD_RATE            (115200U)

#define ESP_S32K_PID_FRAME_LEN             (11U)
#define ESP_S32K_PID_VALUE_MAX             (99U)
#define ESP_S32K_PID_FRAME_START           ('#')
#define ESP_S32K_PID_FRAME_END             ('_')

#define ESP_S32K_BUTTON_FRAME_LEN          (9U)
#define ESP_S32K_ACK_FRAME_LEN             (9U)
#define ESP_S32K_BUTTON_SEQ_MAX            (99U)

/* S32K -> ESP: #BxyzQnn_
 * ESP -> S32K: #AxyzQnn_
 *
 * x = SW2 pressed, y = SW3 pressed, z = swPcb on, nn = decimal sequence 00..99.
 */

typedef struct
{
    uint8_t proportional;
    uint8_t integral;
    uint8_t derivative;
} EspS32kPidFrame_t;

typedef struct
{
    bool sw2Pressed;
    bool sw3Pressed;
    bool swPcbOn;
    uint8_t sequence;
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

static inline bool EspS32k_EncodeButtonLikeFrame(uint8_t type,
                                                 const EspS32kButtonFrame_t *buttons,
                                                 uint8_t *frame,
                                                 size_t frameLen)
{
    uint8_t seq;

    if ((buttons == NULL) || (frame == NULL) || (frameLen < ESP_S32K_BUTTON_FRAME_LEN))
    {
        return false;
    }

    seq = EspS32k_NormalizeSeq(buttons->sequence);
    frame[0] = (uint8_t)ESP_S32K_PID_FRAME_START;
    frame[1] = type;
    frame[2] = EspS32k_BoolDigit(buttons->sw2Pressed);
    frame[3] = EspS32k_BoolDigit(buttons->sw3Pressed);
    frame[4] = EspS32k_BoolDigit(buttons->swPcbOn);
    frame[5] = (uint8_t)'Q';
    frame[6] = (uint8_t)('0' + (seq / 10U));
    frame[7] = (uint8_t)('0' + (seq % 10U));
    frame[8] = (uint8_t)ESP_S32K_PID_FRAME_END;

    return true;
}

static inline bool EspS32k_DecodeButtonLikeFrame(uint8_t expectedType,
                                                 const uint8_t *frame,
                                                 size_t frameLen,
                                                 EspS32kButtonFrame_t *outButtons)
{
    bool sw2;
    bool sw3;
    bool swPcb;
    uint8_t seqTens;
    uint8_t seqOnes;

    if ((frame == NULL) || (outButtons == NULL) || (frameLen != ESP_S32K_BUTTON_FRAME_LEN))
    {
        return false;
    }

    if ((frame[0] != (uint8_t)ESP_S32K_PID_FRAME_START) ||
        (frame[1] != expectedType) ||
        (frame[5] != (uint8_t)'Q') ||
        (frame[8] != (uint8_t)ESP_S32K_PID_FRAME_END))
    {
        return false;
    }

    if ((EspS32k_DecodeBoolDigit(frame[2], &sw2) != true) ||
        (EspS32k_DecodeBoolDigit(frame[3], &sw3) != true) ||
        (EspS32k_DecodeBoolDigit(frame[4], &swPcb) != true))
    {
        return false;
    }

    if ((frame[6] < (uint8_t)'0') || (frame[6] > (uint8_t)'9') ||
        (frame[7] < (uint8_t)'0') || (frame[7] > (uint8_t)'9'))
    {
        return false;
    }

    seqTens = (uint8_t)(frame[6] - (uint8_t)'0');
    seqOnes = (uint8_t)(frame[7] - (uint8_t)'0');

    outButtons->sw2Pressed = sw2;
    outButtons->sw3Pressed = sw3;
    outButtons->swPcbOn = swPcb;
    outButtons->sequence = (uint8_t)((seqTens * 10U) + seqOnes);
    return true;
}

static inline bool EspS32kButtonFrame_Encode(const EspS32kButtonFrame_t *buttons,
                                             uint8_t *frame,
                                             size_t frameLen)
{
    return EspS32k_EncodeButtonLikeFrame((uint8_t)'B', buttons, frame, frameLen);
}

static inline bool EspS32kButtonFrame_Decode(const uint8_t *frame,
                                             size_t frameLen,
                                             EspS32kButtonFrame_t *outButtons)
{
    return EspS32k_DecodeButtonLikeFrame((uint8_t)'B', frame, frameLen, outButtons);
}

static inline bool EspS32kAckFrame_Encode(const EspS32kAckFrame_t *ack,
                                          uint8_t *frame,
                                          size_t frameLen)
{
    return EspS32k_EncodeButtonLikeFrame((uint8_t)'A', ack, frame, frameLen);
}

static inline bool EspS32kAckFrame_Decode(const uint8_t *frame,
                                          size_t frameLen,
                                          EspS32kAckFrame_t *outAck)
{
    return EspS32k_DecodeButtonLikeFrame((uint8_t)'A', frame, frameLen, outAck);
}

static inline bool EspS32kPidFrame_Encode(const EspS32kPidFrame_t *pid,
                                          uint8_t *frame,
                                          size_t frameLen)
{
    if ((pid == NULL) || (frame == NULL) || (frameLen < ESP_S32K_PID_FRAME_LEN))
    {
        return false;
    }

    if ((pid->proportional > ESP_S32K_PID_VALUE_MAX) ||
        (pid->integral > ESP_S32K_PID_VALUE_MAX) ||
        (pid->derivative > ESP_S32K_PID_VALUE_MAX))
    {
        return false;
    }

    frame[0] = (uint8_t)ESP_S32K_PID_FRAME_START;
    frame[1] = (uint8_t)'P';
    frame[2] = (uint8_t)('0' + (pid->proportional / 10U));
    frame[3] = (uint8_t)('0' + (pid->proportional % 10U));
    frame[4] = (uint8_t)'I';
    frame[5] = (uint8_t)('0' + (pid->integral / 10U));
    frame[6] = (uint8_t)('0' + (pid->integral % 10U));
    frame[7] = (uint8_t)'D';
    frame[8] = (uint8_t)('0' + (pid->derivative / 10U));
    frame[9] = (uint8_t)('0' + (pid->derivative % 10U));
    frame[10] = (uint8_t)ESP_S32K_PID_FRAME_END;

    return true;
}

#endif /* ESP_S32K_UART_PROTOCOL_H */
