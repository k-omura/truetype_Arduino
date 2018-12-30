#include "SSD1331_ttf.h"

//constructor
ssd1331_ttf::ssd1331_ttf(SPIClass *_spi) : ttfSpiFullColor(_spi) {};

//Rectangle setting required for drawing SSD1331
void ssd1331_ttf::set_rect(uint16_t _x1, uint16_t _x2, uint16_t _y1, uint16_t _y2) {
  digitalWrite(this->Display_DC, LOW);
  spi->transfer(CMD_COLUMN_ADDRESS);
  spi->transfer((uint8_t)_x1);
  spi->transfer((uint8_t)_x2);
  spi->transfer(CMD_ROW_ADDRESS);
  spi->transfer((uint8_t)_y1);
  spi->transfer((uint8_t)_y2);
  digitalWrite(this->Display_DC, HIGH);
}

void ssd1331_ttf::fill_rect(uint16_t _x1, uint16_t _x2, uint16_t _y1, uint16_t _y2) {
  rgb16_t color;
  digitalWrite(this->Display_CS, LOW);
  digitalWrite(this->Display_DC, LOW);

  spi->transfer(0x26);
  spi->transfer(0b00000001);

  spi->transfer(0x22);
  spi->transfer(_x1);
  spi->transfer(_y1);
  spi->transfer(_x2);
  spi->transfer(_y2);

  //color.raw = _outline;
  color.raw = this->backgroundColor;
  spi->transfer(color.rgb.b << 1);
  spi->transfer(color.rgb.g);
  spi->transfer(color.rgb.r << 1);

  //color.raw = _innner;
  spi->transfer(color.rgb.b << 1);
  spi->transfer(color.rgb.g);
  spi->transfer(color.rgb.r << 1);

  digitalWrite(this->Display_DC, HIGH);
  digitalWrite(this->Display_CS, HIGH);
}
