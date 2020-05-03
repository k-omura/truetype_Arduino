#include "SSD1331_ttf.h"

//constructor
ssd1331_ttf::ssd1331_ttf(SPIClass *_spi, uint16_t _tftWidth, uint16_t _tftHeifht) : ttfSpiFullColor(_spi) {
  this->tftWidth = _tftWidth;
  this->tftHeifht = _tftHeifht;
};

//Rectangle setting required for drawing SSD1331
void ssd1331_ttf::set_rect(uint16_t _x1, uint16_t _x2, uint16_t _y1, uint16_t _y2) {
  uint8_t spi_tx_buff[6];

  spi_tx_buff[0] = CMD_COLUMN_ADDRESS;
  spi_tx_buff[1] = (uint8_t)_x1;
  spi_tx_buff[2] = (uint8_t)_x2;
  spi_tx_buff[3] = CMD_ROW_ADDRESS;
  spi_tx_buff[4] = (uint8_t)_y1;
  spi_tx_buff[5] = (uint8_t)_y2;

  digitalWrite(this->Display_DC, LOW);
  spiSendbuf(spi_tx_buff, 6);
  digitalWrite(this->Display_DC, HIGH);
}

void ssd1331_ttf::fill_rect(uint16_t _x1, uint16_t _x2, uint16_t _y1, uint16_t _y2) {
  uint8_t spi_tx_buff[13];
  rgb16_t color;

  spi_tx_buff[0] = 0x26;
  spi_tx_buff[1] = 0b00000001;
  spi_tx_buff[2] = 0x22;
  spi_tx_buff[3] = (uint8_t)_x1;
  spi_tx_buff[4] = (uint8_t)_y1;
  spi_tx_buff[5] = (uint8_t)_x2;
  spi_tx_buff[6] = (uint8_t)_y2;

  //color.raw = _outline;
  color.raw = this->backgroundColor;
  spi_tx_buff[7] = (uint8_t)(color.rgb.b << 1);
  spi_tx_buff[8] = (uint8_t)(color.rgb.g);
  spi_tx_buff[9] = (uint8_t)(color.rgb.r << 1);

  //color.raw = _innner;
  spi_tx_buff[10] = (uint8_t)(color.rgb.b << 1);
  spi_tx_buff[11] = (uint8_t)(color.rgb.g);
  spi_tx_buff[12] = (uint8_t)(color.rgb.r << 1);

  digitalWrite(this->Display_CS, LOW);
  digitalWrite(this->Display_DC, LOW);
  spiSendbuf(spi_tx_buff, 13);
  digitalWrite(this->Display_DC, HIGH);
  digitalWrite(this->Display_CS, HIGH);
}
