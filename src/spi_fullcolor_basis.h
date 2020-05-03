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

typedef union {
  uint16_t raw;
  struct {
    unsigned int high : 8;
    unsigned int low : 8;
  } split;
} transmit16;

class ttfSpiFullColor {
  public:
    ttfSpiFullColor(SPIClass *_spi);

    void optimisationInit();

    void setTruetype(truetypeClass *_ttf);
    void setSPIpin(uint8_t _cs, uint8_t _reset, uint8_t _dc);
    void setColor(uint16_t _inside, uint16_t _outline, uint16_t _background);
    void setUnderLine(bool _allowUnderLine);

    uint8_t displayString(uint16_t start_x, uint16_t start_y, const char character[], uint16_t characterSize, uint8_t characterSpace);
    uint8_t displayString(uint16_t start_x, uint16_t start_y, const wchar_t character[], uint16_t characterSize, uint8_t characterSpace);
    uint8_t displayMonospaced(uint16_t start_x, uint16_t start_y, const char character[], uint16_t characterSize, uint8_t monospacedWidth);

    void fill_all();
  protected:
    static SPIClass *spi;

    bool optimisation;
#if defined(_VARIANT_ARDUINO_STM32_)
    SPI_HandleTypeDef hspi1;
    DMA_HandleTypeDef hdma_spi1_tx;
#endif

    truetypeClass *font;

    uint16_t tftWidth;
    uint16_t tftHeifht;

    uint8_t Display_CS;
    uint8_t Display_RESET;
    uint8_t Display_DC;

    uint16_t outlineColor;
    uint16_t insideColor;
    uint16_t backgroundColor;
    bool underLine = false;

    void spiDMAHundler();
    void optimisationSetting();
    void spiSendbuf(uint8_t *_buf, uint16_t _length);

    virtual uint8_t outputTFT(uint16_t _x, uint16_t y_, uint16_t _height, uint8_t monospacedWidth = 0);
    virtual void set_rect(uint16_t _x1, uint16_t _x2, uint16_t _y1, uint16_t _y2) = 0;
    virtual void fill_rect(uint16_t _x1, uint16_t _x2, uint16_t _y1, uint16_t _y2);
};
