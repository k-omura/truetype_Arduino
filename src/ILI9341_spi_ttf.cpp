#include "ILI9341_spi_ttf.h"

//constructor
ILI9341_spi_ttf::ILI9341_spi_ttf(SPIClass *_spi) : ttfSpiFullColor(_spi) {};

void ILI9341_spi_ttf::fill_all() {
  fill_rect(0, tftWidth, 0, tftHeifht);
}

//Rectangle setting required for drawing ILI9341
void ILI9341_spi_ttf::set_rect(uint16_t _x1, uint16_t _x2, uint16_t _y1, uint16_t _y2) {
  uint8_t spi_tx_buff[4];
  transmit16 transmitData;

  //Data following CMD_MEMORY_WRITE is written in the area, when a write rectangular area in VRAM is set.
  digitalWrite(this->Display_DC, LOW);
  spi_tx_buff[0] = CMD_COLUMN_ADDRESS_SET;
  spiSendbuf(spi_tx_buff, 1);
  digitalWrite(this->Display_DC, HIGH);
  transmitData.raw = _x1;
  spi_tx_buff[0] = transmitData.split.low;
  spi_tx_buff[1] = transmitData.split.high;
  transmitData.raw = _x2;
  spi_tx_buff[2] = transmitData.split.low;
  spi_tx_buff[3] = transmitData.split.high;
  spiSendbuf(spi_tx_buff, 4);
  digitalWrite(this->Display_DC, LOW);
  spi_tx_buff[0] = CMD_PAGE_ADDRESS_SET;
  spiSendbuf(spi_tx_buff, 1);
  digitalWrite(this->Display_DC, HIGH);
  transmitData.raw = _y1;
  spi_tx_buff[0] = transmitData.split.low;
  spi_tx_buff[1] = transmitData.split.high;
  transmitData.raw = _y2;
  spi_tx_buff[2] = transmitData.split.low;
  spi_tx_buff[3] = transmitData.split.high;
  spiSendbuf(spi_tx_buff, 4);
  digitalWrite(this->Display_DC, LOW);
  spi_tx_buff[0] = CMD_MEMORY_WRITE;
  spiSendbuf(spi_tx_buff, 1);
  digitalWrite(this->Display_DC, HIGH);
}
