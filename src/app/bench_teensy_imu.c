#include "app_internal.h"

static boolean teensy_imu_packet_is_fresh(const TeensyImuSnapshot_t *snapshot, uint32 nowMs)
{
    if ((snapshot == NULL_PTR) || (snapshot->havePacket != TRUE))
    {
        return FALSE;
    }

    return (((uint32)(nowMs - snapshot->lastRxMs)) <= (uint32)TEENSY_IMU_PACKET_STALE_MS) ? TRUE : FALSE;
}

static void teensy_imu_draw_wait(const TeensyImuTestState_t *st)
{
    DisplayTextPadded(0U, "TEENSY IMU");
    DisplayTextPadded(1U, "SPI: WAIT");
    DisplayTextPadded(2U, "ERR:");
    DisplayValue(2U, (int)st->lastSnapshot.errorCount, 4U, 5U);
    DisplayTextPadded(3U, (st->demoEnabled == TRUE) ? "SW2 DEMO:ON" : "SW2 DEMO:OFF");
    DisplayRefresh();
}

static void teensy_imu_draw_packet(const TeensyImuTestState_t *st)
{
    const TeensyImuPacket_t *p = &st->lastSnapshot.packet;

    DisplayTextPadded(0U, "IMU OK  SQ:");
    DisplayValue(0U, (int)p->sequence, 4U, 11U);

    DisplayTextPadded(1U, "R10:    P10:");
    DisplayValue(1U, (int)(p->rollCdeg / 10), 4U, 4U);
    DisplayValue(1U, (int)(p->pitchCdeg / 10), 4U, 12U);

    DisplayTextPadded(2U, "Y10:    GZ:");
    DisplayValue(2U, (int)(p->yawCdeg / 10), 4U, 4U);
    DisplayValue(2U, (int)(p->gzDps10 / 10), 4U, 12U);

    DisplayTextPadded(3U, "C1:   C2:   S:");
    DisplayText(3U, TeensyImu_CameraStatusText(p->camera1Status), 3U, 3U);
    DisplayText(3U, TeensyImu_CameraStatusText(p->camera2Status), 3U, 9U);
    DisplayText(3U,
                ((p->statusFlags & (uint16)TEENSY_IMU_STATUS_SD_READY) != 0U) ? "Y" : "N",
                1U,
                15U);
    DisplayRefresh();
}

void teensy_imu_test_enter(uint32 nowMs)
{
    (void)memset(&g_teensyImuTest, 0, sizeof(g_teensyImuTest));
    TeensyImu_Init();

    g_teensyImuTest.nextDisplayMs = nowMs;
    teensy_imu_draw_wait(&g_teensyImuTest);
    StatusLed_Blue();
}

void teensy_imu_test_update(uint32 nowMs, boolean sw2Pressed)
{
    if (sw2Pressed == TRUE)
    {
        g_teensyImuTest.demoEnabled =
            (g_teensyImuTest.demoEnabled == TRUE) ? FALSE : TRUE;
    }

    if (g_teensyImuTest.demoEnabled == TRUE)
    {
        TeensyImu_InjectDemoSample(nowMs);
    }

    (void)TeensyImu_GetSnapshot(&g_teensyImuTest.lastSnapshot);

    if (time_reached(nowMs, g_teensyImuTest.nextDisplayMs) != TRUE)
    {
        return;
    }

    g_teensyImuTest.nextDisplayMs = nowMs + (uint32)TEENSY_IMU_TEST_DISPLAY_MS;

    if (teensy_imu_packet_is_fresh(&g_teensyImuTest.lastSnapshot, nowMs) == TRUE)
    {
        teensy_imu_draw_packet(&g_teensyImuTest);
        StatusLed_Green();
    }
    else
    {
        teensy_imu_draw_wait(&g_teensyImuTest);
        StatusLed_Yellow();
    }
}

void teensy_imu_test_exit(void)
{
    TeensyImu_Init();
    StatusLed_Blue();
}
