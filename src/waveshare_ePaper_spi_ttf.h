#if !defined TRUETYPE_H
#include "truetype_Arduino.h"
#endif /* TRUETYPE_H */

#if !defined SPI_MONO_H
#include "spi_mono_basis.h"
#endif /* SPI_MONO_H */

//waveshare ePaper command list
#define DRIVER_OUTPUT_CONTROL 0x01
#define BOOSTER_SOFT_START_CONTROL 0x0C
#define GATE_SCAN_START_POSITION 0x0F
#define DEEP_SLEEP_MODE 0x10
#define DATA_ENTRY_MODE_SETTING 0x11
#define SW_RESET 0x12
#define TEMPERATURE_SENSOR_CONTROL 0x1A
#define MASTER_ACTIVATION 0x20
#define DISPLAY_UPDATE_CONTROL_1 0x21
#define DISPLAY_UPDATE_CONTROL_2 0x22
#define WRITE_RAM 0x24
#define WRITE_VCOM_REGISTER 0x2C
#define WRITE_LUT_REGISTER 0x32
#define SET_DUMMY_LINE_PERIOD 0x3A
#define SET_GATE_TIME 0x3B
#define BORDER_WAVEFORM_CONTROL 0x3C
#define SET_RAM_X_ADDRESS_START_END_POSITION 0x44
#define SET_RAM_Y_ADDRESS_START_END_POSITION 0x45
#define SET_RAM_X_ADDRESS_COUNTER 0x4E
#define SET_RAM_Y_ADDRESS_COUNTER 0x4F
#define TERMINATE_FRAME_READ_WRITE 0xFF

// Display resolution
#define EPD_WIDTH       128
#define EPD_HEIGHT      296

#if defined(PORTRAIT)
#define ePaperWidth (128 - 1)
#define ePaperHeight (296 - 1)
#define initPortrait true
#else
#define ePaperWidth (296 - 1)
#define ePaperHeight (128 - 1)
#define initPortrait false
#endif
//waveshare ePaper command list end

class waveshare_ePaper_spi_ttf : public ttfSpiMonoColor {
  public:
    waveshare_ePaper_spi_ttf(SPIClass *_spi);
    void setSPIpin(uint8_t _cs, uint8_t _reset, uint8_t _dc, uint8_t _busy);
    void init(bool portrait = initPortrait);
    void setLut();

  protected:
    uint8_t Display_BUSY;
    const unsigned char* lut;
    bool displayPortrait;

    uint8_t outputDisplay(uint8_t _x, uint8_t y_, uint8_t _height, uint8_t monospacedWidth = 0);
    void set_rect(uint16_t _x1, uint16_t _x2, uint16_t _y1, uint16_t _y2);
    void fill_rect(uint16_t _x1, uint16_t _x2, uint16_t _y1, uint16_t _y2, bool _color, bool _reversal = LOW);
    void setMemoryPointer(int _x, int _y);
    void wait_ePaper();
};

const unsigned char lut_full_update[] = {
  0x02, 0x02, 0x01, 0x11, 0x12, 0x12, 0x22, 0x22,
  0x66, 0x69, 0x69, 0x59, 0x58, 0x99, 0x99, 0x88,
  0x00, 0x00, 0x00, 0x00, 0xF8, 0xB4, 0x13, 0x51,
  0x35, 0x51, 0x51, 0x19, 0x01, 0x00
};

const unsigned char lut_partial_update[] = {
  0x10, 0x18, 0x18, 0x08, 0x18, 0x18, 0x08, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x13, 0x14, 0x44, 0x12,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};