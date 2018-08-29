#if !defined TRUETYPE_H
#include "truetype.h"
#endif /* TRUETYPE_H */

//OLED command list
//#define PORTRAIT
#define LANDSCAPE

#define CMD_DISPLAY_OFF 0xAE
#define CMD_DISPLAY_ON 0xAF
#define CMD_COLUMN_ADDRESS 0x15
#define CMD_ROW_ADDRESS 0x75
#define CMD_SET_POWER_SAVE_MODE 0xB0
#define CMD_POWER_SAVE_EN 0x1A
#define CMD_SET_START_LINE 0xA1
#define CMD_SET_OFFSET 0xA2
#define CMD_SET_MODE_NORMAL 0xA4

#define MAC_PORTRAIT 0xe8
#define MAC_LANDSCAPE 0x48

#if defined(PORTRAIT)
#define MAC_CONFIG MAC_PORTRAIT
#define OLEDWidth 96
#define OLEDHeifht 64
#else
#define MAC_CONFIG MAC_LANDSCAPE
#define OLEDWidth 96
#define OLEDHeifht 64
#endif
//OLED command list end

typedef union {
  uint16_t raw;
  struct {
    unsigned int r : 5;
    unsigned int g : 6;
    unsigned int b : 5;
  } rgb;
} rgb16_t;

class outputttf {
  public:
    outputttf(SPIClass *_spi);

    void setTruetype(truetypeClass *_ttf);
    void setSPIpin(uint8_t _cs, uint8_t _reset, uint8_t _dc);
    void setColor(uint16_t _inside, uint16_t _outline, uint16_t _background);

    uint8_t displayString(uint8_t start_x, uint8_t start_y, const char character[], uint8_t characterSize, uint8_t characterSpace);
    uint8_t displayString(uint8_t start_x, uint8_t start_y, const wchar_t character[], uint8_t characterSize, uint8_t characterSpace);
    uint8_t displayMonospaced(uint8_t start_x, uint8_t start_y, const char character[], uint8_t characterSize, uint8_t monospacedWidth);

  private:
    static SPIClass *spi;
    truetypeClass *font;

    uint8_t TFT_CS;
    uint8_t TFT_RESET;
    uint8_t TFT_DC;

    uint16_t outlineColor;
    uint16_t insideColor;
    uint16_t backgroundColor;

    uint8_t outputTFT(uint8_t _x, uint8_t y_, uint8_t _height, uint8_t monospacedWidth = 0);
    void SSD1331_set_rect(uint8_t _x1, uint8_t _x2, uint8_t _y1, uint8_t _y2);
    void SSD1331_fill_rect(uint8_t _x1, uint8_t _x2, uint8_t _y1, uint8_t _y2, uint16_t _outline, uint16_t _innner);
};
