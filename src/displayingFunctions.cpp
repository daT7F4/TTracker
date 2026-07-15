#include "displayingFunctions.hpp"

LGFX tft;
LGFX_Sprite canvas(&tft);

LGFX::LGFX(void) {
    auto bic = _bus_instance.config();
    bic.spi_host = SPI2_HOST;
    bic.spi_mode = 0;
    bic.freq_write = 40000000;
    bic.freq_read = 16000000;
    bic.pin_sclk = 26;
    bic.pin_mosi = 23;
    bic.pin_miso = 27;
    bic.pin_dc = 30;
    _bus_instance.config(bic);
    _panel_instance.setBus(&_bus_instance);

    auto pic = _panel_instance.config();
    pic.pin_cs = 28;
    pic.pin_rst = 29;
    pic.panel_width = 240;
    pic.panel_height = 320;
    pic.offset_x = 0;
    pic.offset_y = 0;
    _panel_instance.config(pic);
    setPanel(&_panel_instance);
}

void drawText(uint16_t x, const uint16_t y, const uint8_t s,
                                String t, const uint32_t color,
                                const bool underline) {
    canvas.setColor(color);
    for (uint8_t i = 0; i < t.length(); i++) {
        uint8_t w = 3;
        uint8_t idx = t[i] - 0x20;
        if (idx < 95) {
            w = fontData[idx] >> 6;
            uint8_t h = fontData[idx] & 7;
            uint16_t o = fontOffsets[idx];
            uint8_t yo = (fontData[idx] >> 3) & 7;
            for (uint8_t yp = 0; yp < h; yp++) {
                for (uint8_t xp = 0; xp < w; xp++) {
                    if (fontBitmaps[o >> 3] & (0x80 >> (o & 7)))
                        canvas.drawRect(x + xp * s, y + (yp + yo) * s, s, s);
                    o++;
                }
            }
            if (underline) {
                uint8_t uy = y + 6 * s;
                switch (t[i]) {
                    case ',':
                        break;
                    case 'y':
                        canvas.drawFastVLine(x, uy, s);
                        break;
                    case 'p':
                        canvas.drawRect(x + s + 1, uy, 2 * s - 1, s);
                        canvas.drawFastVLine(x + w * s, uy, s);
                        break;
                    case 'q':
                        canvas.drawRect(x, uy, 2 * s - 1, s);
                        break;
                    default:
                        canvas.drawRect(x, uy, w * s, s);
                        if (t[i + 1] && t[i + 1] != ',' && t[i + 1] != 'p')
                            canvas.drawFastVLine(x + w * s, uy, s);
                        break;
                };
            }
        }
        x += w * s + 1;
    }
}