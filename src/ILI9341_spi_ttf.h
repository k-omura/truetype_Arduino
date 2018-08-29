#if !defined TRUETYPE_H
#include "truetype.h"
#endif /* TRUETYPE_H */

//TFT command list
//#define PORTRAIT
#define LANDSCAPE

#define CMD_SLEEP_OUT 0x11
#define CMD_DISPLAY_ON 0x29
#define CMD_COLUMN_ADDRESS_SET 0x2a
#define CMD_PAGE_ADDRESS_SET 0x2b
#define CMD_MEMORY_WRITE 0x2c
#define CMD_MEMORY_ACCESS_CONTROL 0x36
#define CMD_COLMOD 0x3a

#define MAC_PORTRAIT 0xe8
#define MAC_LANDSCAPE 0x48
#define COLMOD_16BIT 0x55
#define COLMOD_18BIT 0x66

#if defined(PORTRAIT)
#define MAC_CONFIG MAC_PORTRAIT
#define tftWidth 320
#define tftHeifht 240
#define AdafruitTFTOrientation 1
#else
#define MAC_CONFIG MAC_LANDSCAPE
#define tftWidth 240
#define tftHeifht 320
#define AdafruitTFTOrientation 0
#endif
//TFT command list end

class outputttf {
  public:
    outputttf(SPIClass *_spi);

    void setTruetype(truetypeClass *_ttf);
    void setSPIpin(uint8_t _cs, uint8_t _reset, uint8_t _dc);
    void setColor(uint16_t _inside, uint16_t _outline, uint16_t _background);

    uint16_t displayString(uint16_t start_x, uint16_t start_y, const char character[], char characterSize, char characterSpace);
    uint16_t displayString(uint16_t start_x, uint16_t start_y, const wchar_t character[], char characterSize, char characterSpace);

  private:
    static SPIClass *spi;
    truetypeClass *font;

    uint8_t TFT_CS;
    uint8_t TFT_RESET;
    uint8_t TFT_DC;

    uint16_t outlineColor;
    uint16_t insideColor;
    uint16_t backgroundColor;

    uint8_t outputTFT(uint16_t x, uint16_t y, uint16_t height);
    void ILI9341_set_rect(uint16_t x1, uint16_t x2, uint16_t y1, uint16_t y2);
};
