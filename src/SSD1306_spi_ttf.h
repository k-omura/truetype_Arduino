#if !defined TRUETYPE_H
#include "truetype_Arduino.h"
#include "spi_mono_basis.h"
#endif /* TRUETYPE_H */

#define SPI_MONO_H

//OLED command list
//#define PORTRAIT
#define LANDSCAPE

#define CMD_DISPLAY_OFF 0xAE
#define CMD_DISPLAY_ON 0xAF
#define CMD_MUX_RATIO 0xA8 //0x3F (0x3F = 64d -1d)
#define CMD_SET_OFFSET 0xD3 //0x00 (no offset)
#define CMD_SET_STARTLINE 0x40 //0x00 (no offset)
#define CMD_REMAP 0xA1 //or 0xA0 re-map, SEG0 is mapped to column address 127
#define CMD_SCAN_DIRECTION 0xC8 //or 0xC0 scan direction, reverse up-bottom
#define CMD_SET_COMPIN 0xDA //0x12 (Alternative configuration, Disable L/R remap)
#define CMD_ROW_CONTRAST 0x81 //0x7F
#define CMD_STOP_SCROLLING 0x2E
#define CMD_RESUME_RAM 0xA4
#define CMD_SET_OSC_FREQ 0xD5 //0x00
#define CMD_EN_CHARGE 0x8D //0x14 enable charge pump
#define CMD_ADDR_MODE 0x20 //[1:0] = 00: Horizontal Addressing Mode ,01: Vertical Addressing Mode, 10: Page Addressing Mode (RESET), 11: Invalid
#define CMD_COLUMN_ADDR 0x21 //0x00 = start, 0x7f = end
#define CMD_PAGE_ADDR 0x22 //0x00 = start, 0x07 = end

#define MAC_PORTRAIT 0xe8
#define MAC_LANDSCAPE 0x00

#if defined(PORTRAIT)
#define MAC_CONFIG MAC_PORTRAIT
#define OLEDWidth 64
#define OLEDHeight 128
#else
#define MAC_CONFIG MAC_LANDSCAPE
#define OLEDWidth 128
#define OLEDHeight 64
#define OLEDPage 8
#endif
//OLED command list end

class ssd1306_spi_ttf : public ttfSpiMonoColor {
  public:
    ssd1306_spi_ttf(SPIClass *_spi);

  protected:
    uint8_t outputDisplay(uint8_t _x, uint8_t y_, uint8_t _height, uint8_t monospacedWidth = 0);
    void set_rect(uint16_t _x1, uint16_t _x2, uint16_t _y1, uint16_t _y2);
    void fill_rect(uint16_t _x1, uint16_t _x2, uint16_t _y1, uint16_t _y2, bool _color, bool _reversal = LOW);
};
