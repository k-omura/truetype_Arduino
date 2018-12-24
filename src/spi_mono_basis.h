#if !defined TRUETYPE_H
#include "truetype_Arduino.h"
#endif /* TRUETYPE_H */

#define SPI_MONO_H

class ttfSpiMonoColor {
  public:
    ttfSpiMonoColor(SPIClass *_spi);

    void setTruetype(truetypeClass *_ttf);
    void setSPIpin(uint8_t _cs, uint8_t _reset, uint8_t _dc);
    void setColor(bool _inside, bool _outline, bool _background);

    uint8_t displayString(uint8_t start_x, uint8_t start_y, const char character[], uint8_t characterSize, uint8_t characterSpace);
    uint8_t displayString(uint8_t start_x, uint8_t start_y, const wchar_t character[], uint8_t characterSize, uint8_t characterSpace);
    uint8_t displayMonospaced(uint8_t start_x, uint8_t start_y, const char character[], uint8_t characterSize, uint8_t monospacedWidth);

  protected:
    static SPIClass *spi;
    truetypeClass *font;

    uint8_t Display_CS;
    uint8_t Display_RESET;
    uint8_t Display_DC;

    bool outlineColor;
    bool insideColor;
    bool backgroundColor;

    virtual uint8_t outputDisplay(uint8_t _x, uint8_t y_, uint8_t _height, uint8_t monospacedWidth = 0);

    virtual void set_rect(uint16_t _x1, uint16_t _x2, uint16_t _y1, uint16_t _y2) = 0;
    virtual void fill_rect(uint16_t _x1, uint16_t _x2, uint16_t _y1, uint16_t _y2, bool _color, bool _reversal = LOW) = 0;
};
