#pragma once

#include <Arduino.h>

#define LFGX_USE_V1
#include <LovyanGFX.hpp>

#include "font.h"

class LGFX : public lgfx::LGFX_Device {
    lgfx::Bus_SPI _bus_instance;
    lgfx::Panel_ILI9341 _panel_instance;

   public:
    LGFX();
};

extern LGFX tft;
extern LGFX_Sprite canvas;

void drawText(uint16_t x, uint16_t y, uint8_t s, String t, uint32_t color, bool underline);