#include "drivers/display.h"

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <stdio.h>
#include <string.h>

static Adafruit_SSD1306 oled(TEENSY_DISPLAY_WIDTH_PX,
                             TEENSY_DISPLAY_HEIGHT_PX,
                             &Wire1,
                             -1);

static char screenCharBuffer[TEENSY_DISPLAY_CHARACTER_ROWS]
                            [TEENSY_DISPLAY_CHARACTER_COLUMNS];
static bool displayReady = false;
static bool refreshActive = false;
static bool pendingRefreshReady = false;
static uint16_t refreshOffset = 0U;
static uint8_t refreshActiveBuffer[TEENSY_DISPLAY_WIDTH_PX *
                                   TEENSY_DISPLAY_CHARACTER_ROWS] = {};
static uint8_t refreshPendingBuffer[TEENSY_DISPLAY_WIDTH_PX *
                                    TEENSY_DISPLAY_CHARACTER_ROWS] = {};

static bool regionIsValid(uint8_t displayLine, uint8_t linesSpan)
{
    return (linesSpan > 0U) &&
           (displayLine < TEENSY_DISPLAY_CHARACTER_ROWS) &&
           ((uint8_t)(displayLine + linesSpan) <=
            TEENSY_DISPLAY_CHARACTER_ROWS);
}

static void clearTextCell(uint8_t line, uint8_t column)
{
    oled.fillRect((int16_t)column * 8,
                  (int16_t)line * 8,
                  8,
                  8,
                  SSD1306_BLACK);
}

static void drawTextCell(uint8_t line, uint8_t column, char c)
{
    clearTextCell(line, column);
    oled.drawChar((int16_t)column * 8,
                  (int16_t)line * 8,
                  c,
                  SSD1306_WHITE,
                  SSD1306_BLACK,
                  1);
}

static void clearGraphRegion(uint8_t displayLine, uint8_t linesSpan)
{
    oled.fillRect(0,
                  (int16_t)displayLine * 8,
                  TEENSY_DISPLAY_WIDTH_PX,
                  (int16_t)linesSpan * 8,
                  SSD1306_BLACK);
}

static uint8_t clampPercent(uint8_t value)
{
    return (value > 100U) ? 100U : value;
}

static int16_t graphY(uint8_t displayLine, uint8_t linesSpan, uint8_t value)
{
    const uint8_t heightPx = (uint8_t)(linesSpan * 8U);
    const uint8_t clampedValue = clampPercent(value);
    return ((int16_t)displayLine * 8) +
           (int16_t)(((uint32_t)(100U - clampedValue) *
                      (uint32_t)(heightPx - 1U)) /
                     100U);
}

static void writeCommandList(const uint8_t *commands, uint8_t commandCount)
{
    if (commands == nullptr)
    {
        return;
    }

    Wire1.beginTransmission(TEENSY_DISPLAY_I2C_ADDRESS);
    Wire1.write(0x00);
    for (uint8_t i = 0U; i < commandCount; i++)
    {
        Wire1.write(commands[i]);
    }
    Wire1.endTransmission();
}

static void beginQueuedRefresh()
{
    if ((pendingRefreshReady != true) || (refreshActive == true))
    {
        return;
    }

    memcpy(refreshActiveBuffer,
           refreshPendingBuffer,
           sizeof(refreshActiveBuffer));
    pendingRefreshReady = false;
    refreshActive = true;
    refreshOffset = 0U;
}

static void sendRefreshDataChunk()
{
    const uint16_t remaining =
        (uint16_t)(sizeof(refreshActiveBuffer) - refreshOffset);
    uint8_t chunkLength = TEENSY_DISPLAY_ASYNC_CHUNK_BYTES;

    if (remaining == 0U)
    {
        refreshActive = false;
        beginQueuedRefresh();
        return;
    }

    if (chunkLength > remaining)
    {
        chunkLength = (uint8_t)remaining;
    }

    const uint8_t page = (uint8_t)(refreshOffset / TEENSY_DISPLAY_WIDTH_PX);
    const uint8_t column = (uint8_t)(refreshOffset % TEENSY_DISPLAY_WIDTH_PX);
    const uint8_t pageRemaining =
        (uint8_t)(TEENSY_DISPLAY_WIDTH_PX - column);
    if (chunkLength > pageRemaining)
    {
        chunkLength = pageRemaining;
    }

    const uint8_t windowCommands[] = {
        0x21,
        column,
        (uint8_t)(column + chunkLength - 1U),
        0x22,
        page,
        page,
    };
    writeCommandList(windowCommands, sizeof(windowCommands));

    Wire1.beginTransmission(TEENSY_DISPLAY_I2C_ADDRESS);
    Wire1.write(0x40);
    Wire1.write(&refreshActiveBuffer[refreshOffset], chunkLength);
    Wire1.endTransmission();

    refreshOffset = (uint16_t)(refreshOffset + chunkLength);
    if (refreshOffset >= sizeof(refreshActiveBuffer))
    {
        refreshActive = false;
        beginQueuedRefresh();
    }
}

bool DisplayInit()
{
    memset(screenCharBuffer, ' ', sizeof(screenCharBuffer));

    Wire1.setSCL(TEENSY_DISPLAY_SCL_PIN);
    Wire1.setSDA(TEENSY_DISPLAY_SDA_PIN);
    Wire1.begin();
    Wire1.setClock(TEENSY_DISPLAY_I2C_HZ);

    displayReady = oled.begin(SSD1306_SWITCHCAPVCC,
                              TEENSY_DISPLAY_I2C_ADDRESS);
    if (displayReady != true)
    {
        return false;
    }

    oled.setRotation(0);
    oled.setTextWrap(false);
    oled.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
    oled.clearDisplay();
    DisplayRefresh();

    return true;
}

bool DisplayIsReady()
{
    return displayReady;
}

bool DisplayRefreshInProgress()
{
    return (refreshActive == true) || (pendingRefreshReady == true);
}

void DisplayService()
{
    if (displayReady != true)
    {
        return;
    }

    beginQueuedRefresh();
    if (refreshActive != true)
    {
        return;
    }

    sendRefreshDataChunk();
}

void DisplayRefresh()
{
    if (displayReady != true)
    {
        return;
    }

    memcpy(refreshPendingBuffer,
           oled.getBuffer(),
           sizeof(refreshPendingBuffer));
    pendingRefreshReady = true;
    beginQueuedRefresh();
}

void DisplayClear()
{
    if (displayReady != true)
    {
        return;
    }

    memset(screenCharBuffer, ' ', sizeof(screenCharBuffer));
    oled.clearDisplay();
}

void DisplayText(uint16_t displayLine,
                 const char *text,
                 uint16_t textLength,
                 uint16_t textOffset)
{
    if ((displayReady != true) ||
        (text == nullptr) ||
        (displayLine >= TEENSY_DISPLAY_CHARACTER_ROWS) ||
        (textOffset >= TEENSY_DISPLAY_CHARACTER_COLUMNS))
    {
        return;
    }

    const uint16_t maxLength =
        TEENSY_DISPLAY_CHARACTER_COLUMNS - textOffset;
    if (textLength > maxLength)
    {
        textLength = maxLength;
    }

    for (uint16_t i = 0U; i < textLength; i++)
    {
        const char c = (text[i] == '\0') ? ' ' : text[i];
        screenCharBuffer[displayLine][textOffset + i] = c;
        drawTextCell((uint8_t)displayLine, (uint8_t)(textOffset + i), c);
    }
}

void DisplayValue(uint16_t displayLine,
                  int value,
                  uint16_t textLength,
                  uint16_t textOffset)
{
    if ((displayReady != true) ||
        (displayLine >= TEENSY_DISPLAY_CHARACTER_ROWS) ||
        (textOffset >= TEENSY_DISPLAY_CHARACTER_COLUMNS))
    {
        return;
    }

    const uint16_t maxLength =
        TEENSY_DISPLAY_CHARACTER_COLUMNS - textOffset;
    if (textLength > maxLength)
    {
        textLength = maxLength;
    }

    char valueText[TEENSY_DISPLAY_CHARACTER_COLUMNS + 1U];
    memset(valueText, ' ', sizeof(valueText));
    valueText[textLength] = '\0';

    char numberText[16];
    snprintf(numberText, sizeof(numberText), "%d", value);

    for (uint16_t i = 0U; (i < textLength) && (numberText[i] != '\0'); i++)
    {
        valueText[i] = numberText[i];
    }

    DisplayText(displayLine, valueText, textLength, textOffset);
}

void DisplayGraph(uint8_t displayLine,
                  const uint8_t *values,
                  uint16_t valuesCount,
                  uint8_t linesSpan)
{
    if ((displayReady != true) ||
        (values == nullptr) ||
        (valuesCount == 0U) ||
        (regionIsValid(displayLine, linesSpan) != true))
    {
        return;
    }

    if (valuesCount > TEENSY_DISPLAY_WIDTH_PX)
    {
        valuesCount = TEENSY_DISPLAY_WIDTH_PX;
    }

    clearGraphRegion(displayLine, linesSpan);

    int16_t previousY = graphY(displayLine, linesSpan, values[0]);
    oled.drawPixel(0, previousY, SSD1306_WHITE);

    for (uint16_t x = 1U; x < valuesCount; x++)
    {
        const int16_t y = graphY(displayLine, linesSpan, values[x]);
        oled.drawLine((int16_t)x - 1, previousY, (int16_t)x, y, SSD1306_WHITE);
        previousY = y;
    }
}

void DisplaySignedGraph(uint8_t displayLine,
                        const int16_t *values,
                        uint16_t valuesCount,
                        uint8_t linesSpan,
                        uint16_t maxAbsValue)
{
    if ((displayReady != true) ||
        (values == nullptr) ||
        (valuesCount == 0U) ||
        (regionIsValid(displayLine, linesSpan) != true))
    {
        return;
    }

    if (valuesCount > TEENSY_DISPLAY_WIDTH_PX)
    {
        valuesCount = TEENSY_DISPLAY_WIDTH_PX;
    }

    uint16_t scaleAbs = maxAbsValue;
    if (scaleAbs == 0U)
    {
        for (uint16_t i = 0U; i < valuesCount; i++)
        {
            const int32_t v = values[i];
            const uint16_t absValue = (uint16_t)((v < 0) ? -v : v);
            if (absValue > scaleAbs)
            {
                scaleAbs = absValue;
            }
        }
    }
    if (scaleAbs == 0U)
    {
        scaleAbs = 1U;
    }

    const int16_t topY = (int16_t)displayLine * 8;
    const int16_t heightPx = (int16_t)linesSpan * 8;
    const int16_t baselineY = topY + (heightPx / 2);
    const int16_t maxOffset = (heightPx / 2) - 1;

    clearGraphRegion(displayLine, linesSpan);
    oled.drawFastHLine(0, baselineY, TEENSY_DISPLAY_WIDTH_PX, SSD1306_WHITE);

    auto yForValue = [&](int16_t rawValue) -> int16_t {
        int32_t clamped = rawValue;
        if (clamped > (int32_t)scaleAbs)
        {
            clamped = scaleAbs;
        }
        if (clamped < -((int32_t)scaleAbs))
        {
            clamped = -((int32_t)scaleAbs);
        }

        return baselineY -
               (int16_t)((clamped * (int32_t)maxOffset) /
                         (int32_t)scaleAbs);
    };

    int16_t previousY = yForValue(values[0]);
    oled.drawPixel(0, previousY, SSD1306_WHITE);

    for (uint16_t x = 1U; x < valuesCount; x++)
    {
        const int16_t y = yForValue(values[x]);
        oled.drawLine((int16_t)x - 1, previousY, (int16_t)x, y, SSD1306_WHITE);
        previousY = y;
    }
}

void DisplayBarGraph(uint8_t displayLine,
                     const uint8_t *values,
                     uint16_t valuesCount,
                     uint8_t linesSpan)
{
    if ((displayReady != true) ||
        (values == nullptr) ||
        (valuesCount == 0U) ||
        (regionIsValid(displayLine, linesSpan) != true))
    {
        return;
    }

    if (valuesCount > TEENSY_DISPLAY_WIDTH_PX)
    {
        valuesCount = TEENSY_DISPLAY_WIDTH_PX;
    }

    const int16_t topY = (int16_t)displayLine * 8;
    const int16_t heightPx = (int16_t)linesSpan * 8;
    const int16_t bottomY = topY + heightPx - 1;
    const int16_t maxHeightPx = (2 * heightPx) / 3;
    const int16_t referenceY = bottomY - maxHeightPx + 1;

    clearGraphRegion(displayLine, linesSpan);
    oled.drawFastHLine(0, referenceY, TEENSY_DISPLAY_WIDTH_PX, SSD1306_WHITE);

    for (uint16_t x = 0U; x < valuesCount; x++)
    {
        const uint8_t value = clampPercent(values[x]);
        const int16_t height =
            (int16_t)(((uint32_t)value * (uint32_t)maxHeightPx) / 100U);
        if (height > 0)
        {
            oled.drawFastVLine((int16_t)x,
                               bottomY - height + 1,
                               height,
                               SSD1306_WHITE);
        }
    }
}

void DisplayOverlayVerticalLine(uint8_t displayLine,
                                uint8_t linesSpan,
                                uint8_t x)
{
    if ((displayReady != true) ||
        (x >= TEENSY_DISPLAY_WIDTH_PX) ||
        (regionIsValid(displayLine, linesSpan) != true))
    {
        return;
    }

    oled.drawFastVLine(x,
                       (int16_t)displayLine * 8,
                       (int16_t)linesSpan * 8,
                       SSD1306_WHITE);
}

void DisplayOverlayHorizontalLine(uint8_t displayLine,
                                  uint8_t linesSpan,
                                  uint8_t yPx)
{
    if ((displayReady != true) ||
        (regionIsValid(displayLine, linesSpan) != true) ||
        (yPx >= (uint8_t)(linesSpan * 8U)))
    {
        return;
    }

    oled.drawFastHLine(0,
                       ((int16_t)displayLine * 8) + yPx,
                       TEENSY_DISPLAY_WIDTH_PX,
                       SSD1306_WHITE);
}

void DisplayOverlayHorizontalSegment(uint8_t displayLine,
                                     uint8_t linesSpan,
                                     uint8_t yPx,
                                     uint8_t x0,
                                     uint8_t x1)
{
    if ((displayReady != true) ||
        (regionIsValid(displayLine, linesSpan) != true) ||
        (yPx >= (uint8_t)(linesSpan * 8U)) ||
        (x0 >= TEENSY_DISPLAY_WIDTH_PX) ||
        (x0 > x1))
    {
        return;
    }

    if (x1 >= TEENSY_DISPLAY_WIDTH_PX)
    {
        x1 = TEENSY_DISPLAY_WIDTH_PX - 1U;
    }

    oled.drawFastHLine(x0,
                       ((int16_t)displayLine * 8) + yPx,
                       (int16_t)(x1 - x0 + 1U),
                       SSD1306_WHITE);
}
