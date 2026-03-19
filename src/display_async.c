#include "display_async.h"

#define DISPLAY_I2C_ADDRESS         (0x3CU)
#define DISPLAY_CTRL_ALL_DATA       (0x40U)
#define DISPLAY_CTRL_COMMAND_STREAM (0x00U)

typedef enum
{
    DISPLAY_ASYNC_PHASE_IDLE = 0U,
    DISPLAY_ASYNC_PHASE_DATA,
    DISPLAY_ASYNC_PHASE_PAGE_RANGE,
    DISPLAY_ASYNC_PHASE_COLUMN_RANGE
} DisplayAsync_PhaseType;

static uint8 sI2cChannel = 0U;
static boolean sInitialized = FALSE;
static boolean sHealthy = TRUE;
static boolean sPendingFrame = FALSE;
static DisplayAsync_PhaseType sPhase = DISPLAY_ASYNC_PHASE_IDLE;

static I2c_DataType sFrontFrame[DISPLAY_ASYNC_FRAME_TOTAL_BYTES] = { DISPLAY_CTRL_ALL_DATA };
static I2c_DataType sBackFrame[DISPLAY_ASYNC_FRAME_TOTAL_BYTES] = { DISPLAY_CTRL_ALL_DATA };

static I2c_DataType sPageRangeCmd[4U] = { DISPLAY_CTRL_COMMAND_STREAM, 0x22U, 0x00U, 0x03U };
static I2c_DataType sColumnRangeCmd[4U] = { DISPLAY_CTRL_COMMAND_STREAM, 0x21U, 0x00U, 0x7FU };

static I2c_RequestType sFrameRequest = {
    DISPLAY_I2C_ADDRESS,
    FALSE,
    FALSE,
    FALSE,
    FALSE,
    DISPLAY_ASYNC_FRAME_TOTAL_BYTES,
    I2C_SEND_DATA,
    sFrontFrame
};

static I2c_RequestType sPageRangeRequest = {
    DISPLAY_I2C_ADDRESS,
    FALSE,
    FALSE,
    FALSE,
    FALSE,
    4U,
    I2C_SEND_DATA,
    sPageRangeCmd
};

static I2c_RequestType sColumnRangeRequest = {
    DISPLAY_I2C_ADDRESS,
    FALSE,
    FALSE,
    FALSE,
    FALSE,
    4U,
    I2C_SEND_DATA,
    sColumnRangeCmd
};

static void DisplayAsync_CopyFrame(I2c_DataType *Dst, const I2c_DataType *Src)
{
    uint16 idx;

    for (idx = 0U; idx < DISPLAY_ASYNC_FRAME_TOTAL_BYTES; idx++)
    {
        Dst[idx] = Src[idx];
    }
}

static Std_ReturnType DisplayAsync_StartTx(const I2c_RequestType *Request, DisplayAsync_PhaseType NextPhase)
{
    Std_ReturnType result;

    result = I2c_AsyncTransmit(sI2cChannel, Request);
    if (result == E_OK)
    {
        sPhase = NextPhase;
        sHealthy = TRUE;
    }

    return result;
}

static void DisplayAsync_TryStartFrame(void)
{
    if ((sPendingFrame != TRUE) || (sPhase != DISPLAY_ASYNC_PHASE_IDLE))
    {
        return;
    }

    DisplayAsync_CopyFrame(sFrontFrame, sBackFrame);
    sPendingFrame = FALSE;

    if (DisplayAsync_StartTx(&sFrameRequest, DISPLAY_ASYNC_PHASE_DATA) != E_OK)
    {
        sPendingFrame = TRUE;
        sHealthy = FALSE;
    }
}

void DisplayAsync_Init(uint8 I2cChannel)
{
    uint16 idx;

    sI2cChannel = I2cChannel;
    sInitialized = TRUE;
    sHealthy = TRUE;
    sPendingFrame = FALSE;
    sPhase = DISPLAY_ASYNC_PHASE_IDLE;

    sFrontFrame[0U] = DISPLAY_CTRL_ALL_DATA;
    sBackFrame[0U] = DISPLAY_CTRL_ALL_DATA;
    for (idx = 1U; idx < DISPLAY_ASYNC_FRAME_TOTAL_BYTES; idx++)
    {
        sFrontFrame[idx] = 0x00U;
        sBackFrame[idx] = 0x00U;
    }
}

void DisplayAsync_Service(void)
{
    I2c_StatusType status;

    if (sInitialized != TRUE)
    {
        return;
    }

    if (sPhase == DISPLAY_ASYNC_PHASE_IDLE)
    {
        DisplayAsync_TryStartFrame();
        return;
    }

    status = I2c_GetStatus(sI2cChannel);
    if ((status == I2C_CH_SEND) || (status == I2C_CH_RECEIVE))
    {
        return;
    }

    if (status == I2C_CH_ERROR_PRESENT)
    {
        sPhase = DISPLAY_ASYNC_PHASE_IDLE;
        sHealthy = FALSE;
        DisplayAsync_TryStartFrame();
        return;
    }

    if ((status != I2C_CH_FINISHED) && (status != I2C_CH_IDLE))
    {
        return;
    }

    switch (sPhase)
    {
        case DISPLAY_ASYNC_PHASE_DATA:
            if (DisplayAsync_StartTx(&sPageRangeRequest, DISPLAY_ASYNC_PHASE_PAGE_RANGE) != E_OK)
            {
                sPhase = DISPLAY_ASYNC_PHASE_IDLE;
                sHealthy = FALSE;
            }
            break;

        case DISPLAY_ASYNC_PHASE_PAGE_RANGE:
            if (DisplayAsync_StartTx(&sColumnRangeRequest, DISPLAY_ASYNC_PHASE_COLUMN_RANGE) != E_OK)
            {
                sPhase = DISPLAY_ASYNC_PHASE_IDLE;
                sHealthy = FALSE;
            }
            break;

        case DISPLAY_ASYNC_PHASE_COLUMN_RANGE:
            sPhase = DISPLAY_ASYNC_PHASE_IDLE;
            DisplayAsync_TryStartFrame();
            break;

        default:
            sPhase = DISPLAY_ASYNC_PHASE_IDLE;
            sHealthy = FALSE;
            break;
    }
}

boolean DisplayAsync_IsBusy(void)
{
    return (sPhase != DISPLAY_ASYNC_PHASE_IDLE) ? TRUE : FALSE;
}

boolean DisplayAsync_IsHealthy(void)
{
    return sHealthy;
}

I2c_DataType * DisplayAsync_GetBackBufferData(void)
{
    return &sBackFrame[1U];
}

Std_ReturnType DisplayAsync_QueueBackBuffer(void)
{
    if (sInitialized != TRUE)
    {
        return E_NOT_OK;
    }

    sPendingFrame = TRUE;
    DisplayAsync_TryStartFrame();
    return E_OK;
}

Std_ReturnType DisplayAsync_SubmitFrame(const I2c_DataType *Data, uint16 Length)
{
    uint16 idx;

    if ((sInitialized != TRUE) || (Data == NULL_PTR))
    {
        return E_NOT_OK;
    }

    if (Length == DISPLAY_ASYNC_FRAME_DATA_BYTES)
    {
        sBackFrame[0U] = DISPLAY_CTRL_ALL_DATA;
        for (idx = 0U; idx < DISPLAY_ASYNC_FRAME_DATA_BYTES; idx++)
        {
            sBackFrame[idx + 1U] = Data[idx];
        }
    }
    else if (Length == DISPLAY_ASYNC_FRAME_TOTAL_BYTES)
    {
        DisplayAsync_CopyFrame(sBackFrame, Data);
    }
    else
    {
        return E_NOT_OK;
    }

    return DisplayAsync_QueueBackBuffer();
}
