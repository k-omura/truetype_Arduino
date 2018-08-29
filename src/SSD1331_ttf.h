#if !defined TRUETYPE_H
#include "truetype_Arduino.h"
#endif /* TRUETYPE_H */

#if !defined SPI_FULLCOLOR_H
#include "spi_fullcolor_basis.h"
#endif /* SPI_FULLCOLOR_H */

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

class ssd1331_ttf : public ttfSpiFullColor {
  public:
    ssd1331_ttf(SPIClass *_spi);

  protected:
    virtual void set_rect(uint16_t _x1, uint16_t _x2, uint16_t _y1, uint16_t _y2) override;
    virtual void fill_rect(uint16_t _x1, uint16_t _x2, uint16_t _y1, uint16_t _y2, uint16_t _color) override;
};
