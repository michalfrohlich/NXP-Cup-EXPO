#pragma once

#include <Arduino.h>
#include <stdint.h>

#include "config/display_config.h"

bool DisplayInit();
bool DisplayIsReady();
bool DisplayRefreshInProgress();
void DisplayService();
void DisplayRefresh();
void DisplayClear();
void DisplayText(uint16_t displayLine,
                 const char *text,
                 uint16_t textLength,
                 uint16_t textOffset);
void DisplayValue(uint16_t displayLine,
                  int value,
                  uint16_t textLength,
                  uint16_t textOffset);
void DisplayGraph(uint8_t displayLine,
                  const uint8_t *values,
                  uint16_t valuesCount,
                  uint8_t linesSpan);
void DisplaySignedGraph(uint8_t displayLine,
                        const int16_t *values,
                        uint16_t valuesCount,
                        uint8_t linesSpan,
                        uint16_t maxAbsValue);
void DisplayBarGraph(uint8_t displayLine,
                     const uint8_t *values,
                     uint16_t valuesCount,
                     uint8_t linesSpan);
void DisplayOverlayVerticalLine(uint8_t displayLine,
                                uint8_t linesSpan,
                                uint8_t x);
void DisplayOverlayHorizontalLine(uint8_t displayLine,
                                  uint8_t linesSpan,
                                  uint8_t yPx);
void DisplayOverlayHorizontalSegment(uint8_t displayLine,
                                     uint8_t linesSpan,
                                     uint8_t yPx,
                                     uint8_t x0,
                                     uint8_t x1);
