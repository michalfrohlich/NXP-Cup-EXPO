#include "../app_internal.h"

#define TEENSY_LINK_TEST_DISPLAY_MS       (100U)
#define TEENSY_LINK_TEST_PAGE_COUNT       (5U)

typedef struct
{
    uint32 nextServiceMs;
    uint32 nextDisplayMs;
    uint16 controlSeq;
    uint8 page;
    TeensyLinkS32kInputs_t input;
    TeensyLinkSnapshot_t snapshot;
    TeensyLinkDiagnostics_t diag;
} TeensyLinkTestState_t;

static TeensyLinkTestState_t g_teensyLinkTest;

static void teensy_link_test_fill_default_input(void)
{
    TeensyLinkS32kInputs_t *in = &g_teensyLinkTest.input;

    (void)memset(in, 0, sizeof(*in));

    in->controlLoopSeq = g_teensyLinkTest.controlSeq;
    in->controlDtUs = (uint16)((uint16)TEENSY_LINK_SERVICE_PERIOD_MS * 1000U);
    in->appMode = (uint8)APP_BUILD_MODE_NXP_CUP_TESTS;
    in->appState = (uint8)RUNTIME_TEST_TEENSY_LINK;

    for (uint8 i = 0U; i < (uint8)TEENSY_LINK_CAMERA_COUNT; i++)
    {
        in->camera[i].errorPct = 0;
        in->camera[i].status = (uint8)VISION_TRACK_LOST;
        in->camera[i].feature = (uint8)VISION_FEATURE_NONE;
        in->camera[i].confidence = 0U;
        in->camera[i].leftLineIdx = 255U;
        in->camera[i].rightLineIdx = 255U;
        in->camera[i].ageMs = 255U;
        in->camera[i].flags = (uint8)(TEENSY_LINK_CAMERA_FLAG_SOURCE_S32K |
                                      TEENSY_LINK_CAMERA_FLAG_STALE);
    }

    in->ultrasonicAgeMs = 65535U;
}

static const char *teensy_link_status_text(const TeensyLinkSnapshot_t *snapshot,
                                           const TeensyLinkDiagnostics_t *diag)
{
    if ((snapshot != NULL_PTR) && (snapshot->live == TRUE))
    {
        return "OK";
    }

    if ((diag != NULL_PTR) && (diag->crcErrorCount != 0U))
    {
        return "CRC";
    }

    if ((snapshot != NULL_PTR) && (snapshot->haveFrame == TRUE))
    {
        return "STALE";
    }

    return "WAIT";
}

static const char *teensy_link_camera_status_text(uint8 status)
{
    switch ((VisionTrackStatus_t)status)
    {
        case VISION_TRACK_BOTH:
            return "BOTH";
        case VISION_TRACK_LEFT:
            return "LEFT";
        case VISION_TRACK_RIGHT:
            return "RGHT";
        case VISION_TRACK_LOST:
        default:
            return "LOST";
    }
}

static void teensy_link_test_update_led(void)
{
    if (g_teensyLinkTest.snapshot.live == TRUE)
    {
        StatusLed_Green();
    }
    else if ((g_teensyLinkTest.diag.crcErrorCount != 0U) ||
             (g_teensyLinkTest.diag.protocolErrorCount > 20U) ||
             (g_teensyLinkTest.diag.spiErrorCount != 0U))
    {
        StatusLed_Red();
    }
    else if (g_teensyLinkTest.snapshot.haveFrame == TRUE)
    {
        StatusLed_Yellow();
    }
    else
    {
        StatusLed_Blue();
    }
}

static void teensy_link_test_draw_link(void)
{
    const char *statusText =
        teensy_link_status_text(&g_teensyLinkTest.snapshot, &g_teensyLinkTest.diag);

    DisplayTextPadded(0U, "TLINK ");
    DisplayText(0U, statusText, (uint16)strlen(statusText), 6U);

    DisplayTextPadded(1U, "S32:    TEN:");
    DisplayValue(1U, (int)g_teensyLinkTest.diag.txSeq, 4U, 4U);
    DisplayValue(1U, (int)g_teensyLinkTest.snapshot.teensySeq, 4U, 12U);

    DisplayTextPadded(2U, "RDY:  ACK:");
    DisplayText(2U, (g_teensyLinkTest.diag.readyHigh == TRUE) ? "Y" : "N", 1U, 4U);
    DisplayValue(2U, (int)g_teensyLinkTest.snapshot.ackS32kSeq, 4U, 10U);

    DisplayTextPadded(3U, "ERR:");
    DisplayValue(3U,
                 (int)(g_teensyLinkTest.diag.crcErrorCount +
                       g_teensyLinkTest.diag.protocolErrorCount +
                       g_teensyLinkTest.diag.spiErrorCount),
                 5U,
                 4U);
    DisplayRefresh();
}

static void teensy_link_test_draw_camera(uint8 cameraIndex)
{
    const TeensyLinkCameraResult_t *camera = &g_teensyLinkTest.snapshot.camera[cameraIndex];
    const char *statusText = teensy_link_camera_status_text(camera->status);

    DisplayTextPadded(0U, (cameraIndex == 0U) ? "TLINK CAM0" : "TLINK CAM1");

    DisplayTextPadded(1U, "ST:     CF:");
    DisplayText(1U, statusText, (uint16)strlen(statusText), 3U);
    DisplayValue(1U, (int)camera->confidence, 3U, 12U);

    DisplayTextPadded(2U, "ERR:    L:");
    DisplayValue(2U, (int)camera->errorPct, 4U, 4U);
    DisplayValue(2U, (int)camera->leftLineIdx, 3U, 12U);

    DisplayTextPadded(3U, "R:    AGE:");
    DisplayValue(3U, (int)camera->rightLineIdx, 3U, 2U);
    DisplayValue(3U, (int)camera->ageMs, 3U, 11U);
    DisplayRefresh();
}

static void teensy_link_test_draw_imu(void)
{
    DisplayTextPadded(0U, "TLINK IMU/LOG");

    DisplayTextPadded(1U, "YAW:    GZ:");
    DisplayValue(1U, (int)(g_teensyLinkTest.snapshot.imu.yawCdeg / 100), 4U, 4U);
    DisplayValue(1U, (int)(g_teensyLinkTest.snapshot.imu.gzDps10 / 10), 4U, 12U);

    DisplayTextPadded(2U, "LAT:    AGE:");
    DisplayValue(2U, (int)g_teensyLinkTest.snapshot.imu.lateralMg, 4U, 4U);
    DisplayValue(2U, (int)g_teensyLinkTest.snapshot.sensorAgeMs, 3U, 13U);

    DisplayTextPadded(3U, "SD:  DROP:");
    DisplayText(3U,
                ((g_teensyLinkTest.snapshot.loggerFlags &
                  (uint16)TEENSY_LINK_LOGGER_FLAG_READY) != 0U) ? "Y" : "N",
                1U,
                3U);
    DisplayValue(3U, (int)g_teensyLinkTest.snapshot.loggerDropCount, 5U, 10U);
    DisplayRefresh();
}

static void teensy_link_test_draw_rx_data(void)
{
    DisplayTextPadded(0U, "TLINK RX 128B");

    DisplayTextPadded(1U, "SEQ:    ACK:");
    DisplayValue(1U, (int)g_teensyLinkTest.snapshot.teensySeq, 4U, 4U);
    DisplayValue(1U, (int)g_teensyLinkTest.snapshot.ackS32kSeq, 4U, 12U);

    DisplayTextPadded(2U, "GOOD:    TX:");
    DisplayValue(2U, (int)(g_teensyLinkTest.diag.goodFrameCount % 10000U), 4U, 5U);
    DisplayValue(2U, (int)(g_teensyLinkTest.diag.txCount % 1000U), 3U, 13U);

    DisplayTextPadded(3U, "CRC:    SPI:");
    DisplayValue(3U, (int)(g_teensyLinkTest.diag.crcErrorCount % 10000U), 4U, 4U);
    DisplayValue(3U, (int)(g_teensyLinkTest.diag.spiErrorCount % 1000U), 3U, 13U);
    DisplayRefresh();
}

static void teensy_link_test_draw(void)
{
    DisplayClear();

    switch (g_teensyLinkTest.page)
    {
        case 1U:
            teensy_link_test_draw_camera(0U);
            break;
        case 2U:
            teensy_link_test_draw_camera(1U);
            break;
        case 3U:
            teensy_link_test_draw_imu();
            break;
        case 4U:
            teensy_link_test_draw_rx_data();
            break;
        case 0U:
        default:
            teensy_link_test_draw_link();
            break;
    }
}

void teensy_link_test_enter(uint32 nowMs)
{
    TeensyLink_Init();
    (void)memset(&g_teensyLinkTest, 0, sizeof(g_teensyLinkTest));

    g_teensyLinkTest.nextServiceMs = nowMs;
    g_teensyLinkTest.nextDisplayMs = nowMs;
    teensy_link_test_fill_default_input();
    (void)TeensyLink_GetSnapshot(&g_teensyLinkTest.snapshot);
    (void)TeensyLink_GetDiagnostics(&g_teensyLinkTest.diag);
    teensy_link_test_draw();
    StatusLed_Blue();
}

void teensy_link_test_update(uint32 nowMs, boolean sw2Pressed)
{
    if (sw2Pressed == TRUE)
    {
        g_teensyLinkTest.page =
            (uint8)((g_teensyLinkTest.page + 1U) % (uint8)TEENSY_LINK_TEST_PAGE_COUNT);
        g_teensyLinkTest.nextDisplayMs = nowMs;
    }

    while (time_reached(nowMs, g_teensyLinkTest.nextServiceMs) == TRUE)
    {
        g_teensyLinkTest.controlSeq++;
        teensy_link_test_fill_default_input();
        (void)TeensyLink_Service5ms(nowMs, &g_teensyLinkTest.input);
        g_teensyLinkTest.nextServiceMs += (uint32)TEENSY_LINK_SERVICE_PERIOD_MS;
    }

    (void)TeensyLink_GetSnapshot(&g_teensyLinkTest.snapshot);
    (void)TeensyLink_GetDiagnostics(&g_teensyLinkTest.diag);
    teensy_link_test_update_led();

    if (time_reached(nowMs, g_teensyLinkTest.nextDisplayMs) == TRUE)
    {
        g_teensyLinkTest.nextDisplayMs = nowMs + TEENSY_LINK_TEST_DISPLAY_MS;
        teensy_link_test_draw();
    }
}

void teensy_link_test_exit(void)
{
    DisplayClear();
    DisplayTextPadded(0U, "TLINK EXIT");
    DisplayRefresh();
    StatusLed_Blue();
}
