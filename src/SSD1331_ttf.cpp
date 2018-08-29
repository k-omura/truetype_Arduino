#include "SSD1331_ttf.h"

//constructor
ssd1331_ttf::ssd1331_ttf(SPIClass *_spi) : outputttf(_spi) {};

//Rectangle setting required for drawing SSD1331
void ssd1331_ttf::set_rect(uint16_t _x1, uint16_t _x2, uint16_t _y1, uint16_t _y2) {
  digitalWrite(this->TFT_DC, LOW);
  spi->transfer(CMD_COLUMN_ADDRESS);
  spi->transfer((uint8_t)_x1);
  spi->transfer((uint8_t)_x2);
  spi->transfer(CMD_ROW_ADDRESS);
  spi->transfer((uint8_t)_y1);
  spi->transfer((uint8_t)_y2);
  digitalWrite(this->TFT_DC, HIGH);
}

void ssd1331_ttf::fill_rect(uint8_t _x1, uint8_t _x2, uint8_t _y1, uint8_t _y2, uint16_t _color) {
  rgb16_t color;
  digitalWrite(this->TFT_CS, LOW);
  digitalWrite(this->TFT_DC, LOW);

  spi->transfer(0x26);
  spi->transfer(0b00000001);

  spi->transfer(0x22);
  spi->transfer(_x1);
  spi->transfer(_y1);
  spi->transfer(_x2);
  spi->transfer(_y2);

  //color.raw = _outline;
  color.raw = _color;
  spi->transfer(color.rgb.b << 1);
  spi->transfer(color.rgb.g);
  spi->transfer(color.rgb.r << 1);

  //color.raw = _innner;
  spi->transfer(color.rgb.b << 1);
  spi->transfer(color.rgb.g);
  spi->transfer(color.rgb.r << 1);

  digitalWrite(this->TFT_DC, HIGH);
  digitalWrite(this->TFT_CS, HIGH);
}
