#if !defined TRUETYPE_H
#include "truetype_Arduino.h"
#endif /* TRUETYPE_H */

#define SPI_FULLCOLOR_H

typedef union {
  uint16_t raw;
  struct {
    unsigned int r : 5;
    unsigned int g : 6;
    unsigned int b : 5;
  } rgb;
} rgb16_t;

class ttfSpiFullColor {
  public:
    ttfSpiFullColor(SPIClass *_spi);

    void setTruetype(truetypeClass *_ttf);
    void setSPIpin(uint8_t _cs, uint8_t _reset, uint8_t _dc);
    void setColor(uint16_t _inside, uint16_t _outline, uint16_t _background);
    void setUnderLine(bool _allowUnderLine);

    uint8_t displayString(uint8_t start_x, uint8_t start_y, const char character[], uint8_t characterSize, uint8_t characterSpace);
    uint8_t displayString(uint8_t start_x, uint8_t start_y, const wchar_t character[], uint8_t characterSize, uint8_t characterSpace);
    uint8_t displayMonospaced(uint8_t start_x, uint8_t start_y, const char character[], uint8_t characterSize, uint8_t monospacedWidth);

  protected:
    static SPIClass *spi;
    truetypeClass *font;

    uint8_t Display_CS;
    uint8_t Display_RESET;
    uint8_t Display_DC;

    uint16_t outlineColor;
    uint16_t insideColor;
    uint16_t backgroundColor;
    bool underLine = false;

    virtual uint8_t outputTFT(uint8_t _x, uint8_t y_, uint8_t _height, uint8_t monospacedWidth = 0);

    virtual void set_rect(uint16_t _x1, uint16_t _x2, uint16_t _y1, uint16_t _y2) = 0;
    virtual void fill_rect(uint16_t _x1, uint16_t _x2, uint16_t _y1, uint16_t _y2);
};
