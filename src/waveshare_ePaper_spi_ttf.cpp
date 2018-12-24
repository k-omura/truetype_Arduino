#include "waveshare_ePaper_spi_ttf.h"

//constructor
waveshare_ePaper_spi_ttf::waveshare_ePaper_spi_ttf(SPIClass *_spi) : ttfSpiMonoColor(_spi) {};

void waveshare_ePaper_spi_ttf::setSPIpin(uint8_t _cs, uint8_t _reset, uint8_t _dc, uint8_t _busy) {
  this->Display_CS = _cs;
  this->Display_RESET = _reset;
  this->Display_DC = _dc;
  this->Display_BUSY = _busy;
}

void waveshare_ePaper_spi_ttf::init(bool portrait) {
  //module reset
  digitalWrite(this->Display_RESET, LOW);
  delay(200);
  digitalWrite(this->Display_RESET, HIGH);
  delay(200);
  //module reset end

  //init commands
  digitalWrite(this->Display_CS, LOW);

  digitalWrite(this->Display_DC, LOW);
  spi->transfer(DRIVER_OUTPUT_CONTROL);
  digitalWrite(this->Display_DC, HIGH);
  spi->transfer((EPD_HEIGHT - 1) & 0xFF);
  spi->transfer(((EPD_HEIGHT - 1) >> 8) & 0xFF);
  spi->transfer(0x00); // GD = 0; SM = 0; TB = 0;

  digitalWrite(this->Display_DC, LOW);
  spi->transfer(BOOSTER_SOFT_START_CONTROL);
  digitalWrite(this->Display_DC, HIGH);
  spi->transfer(0xD7);
  spi->transfer(0xD6);
  spi->transfer(0x9D);

  digitalWrite(this->Display_DC, LOW);
  spi->transfer(WRITE_VCOM_REGISTER);
  digitalWrite(this->Display_DC, HIGH);
  spi->transfer(0xA8); // VCOM 7C

  digitalWrite(this->Display_DC, LOW);
  spi->transfer(SET_DUMMY_LINE_PERIOD);
  digitalWrite(this->Display_DC, HIGH);
  spi->transfer(0x1A); // 4 dummy lines per gate

  digitalWrite(this->Display_DC, LOW);
  spi->transfer(SET_GATE_TIME);
  digitalWrite(this->Display_DC, HIGH);
  spi->transfer(0x08); // 2us per line

  digitalWrite(this->Display_DC, LOW);
  spi->transfer(DATA_ENTRY_MODE_SETTING);
  digitalWrite(this->Display_DC, HIGH);
  if(portrait){
    spi->transfer(0x03); // X increment; Y increment
  }else{
    spi->transfer(0x05); // X increment; Y decrement, the address counter is updated in the Y direction.
  }

  digitalWrite(this->Display_CS, HIGH);
  //init commands end

  setLut();

  return;
}

//set the look-up table register
void waveshare_ePaper_spi_ttf::setLut(){
  lut = lut_partial_update;

  digitalWrite(this->Display_CS, LOW);

  digitalWrite(this->Display_DC, LOW);
  spi->transfer(WRITE_LUT_REGISTER);
  digitalWrite(this->Display_DC, HIGH);

  //the length of look-up table is 30 bytes
  for (int i = 0; i < 30; i++) {
    spi->transfer(this->lut[i]);
  }

  digitalWrite(this->Display_CS, HIGH);

  return;
}

uint8_t waveshare_ePaper_spi_ttf::outputDisplay(uint8_t _x, uint8_t _y, uint8_t _height, uint8_t monospacedWidth) {
  //---Code for displaying a bitmap
  uint8_t width = font->generateBitmap(_height);

  //Rectangle setting required for drawing
  digitalWrite(this->Display_CS, LOW);
  set_rect(_x, (_x + width - 1), _y, (_y + _height - 1));
  
  //Drawing character


  //Drawing character end
  digitalWrite(this->Display_CS, HIGH);
  //---Code for displaying a bitmap end

  font->freeBitmap();
  font->freeGlyph();

  return width;
}

//Rectangle setting required for drawing waveshare ePaper
void waveshare_ePaper_spi_ttf::set_rect(uint16_t _x1, uint16_t _x2, uint16_t _y1, uint16_t _y2) {
  digitalWrite(this->Display_DC, LOW);
  spi->transfer(SET_RAM_X_ADDRESS_START_END_POSITION);
  digitalWrite(this->Display_DC, HIGH);
  /* x point must be the multiple of 8 or the last 3 bits will be ignored */
  spi->transfer((_x1 >> 3) & 0xFF);
  spi->transfer((_x2 >> 3) & 0xFF);

  digitalWrite(this->Display_DC, LOW);
  spi->transfer(SET_RAM_Y_ADDRESS_START_END_POSITION);
  digitalWrite(this->Display_DC, HIGH);
  spi->transfer(_y1 & 0xFF);
  spi->transfer((_y1 >> 8) & 0xFF);
  spi->transfer(_y2 & 0xFF);
  spi->transfer((_y2 >> 8) & 0xFF);

  return;
}

void waveshare_ePaper_spi_ttf::setMemoryPointer(int _x, int _y) {
  digitalWrite(this->Display_DC, LOW);
  spi->transfer(SET_RAM_X_ADDRESS_COUNTER);
  digitalWrite(this->Display_DC, HIGH);
  /* x point must be the multiple of 8 or the last 3 bits will be ignored */
  spi->transfer((_x >> 3) & 0xFF);

  digitalWrite(this->Display_DC, LOW);
  spi->transfer(SET_RAM_Y_ADDRESS_COUNTER);
  digitalWrite(this->Display_DC, HIGH);
  spi->transfer(_y & 0xFF);
  spi->transfer((_y >> 8) & 0xFF);

  wait_ePaper();

  return;
}

void waveshare_ePaper_spi_ttf::fill_rect(uint16_t _x1, uint16_t _x2, uint16_t _y1, uint16_t _y2, bool _color, bool _reversal) {
  uint8_t seekColumn = _x2 - _x1 + 1;
  int8_t seekPage = (uint8_t)(_y2 / 8) - (uint8_t)(_y1 / 8) - 1;
  uint8_t firstPageBit = (_y1 % 8);
  uint8_t lastPageBit = (_y2 % 8);

  uint8_t fillData, firstPageData, lastPageData;
  fillData = 0b11111111;
  firstPageData = fillData << (firstPageBit);
  lastPageData = fillData >> (7 - lastPageBit);

  if (_reversal) {
    fillData = ~fillData;
    firstPageData = ~firstPageData;
    lastPageData = ~lastPageData;
  } else if (!_color) {
    fillData = 0b00000000;
    firstPageData = 0b00000000;
    lastPageData = 0b00000000;
  }

  digitalWrite(this->Display_CS, LOW);

  set_rect(_x1, _x2, _y1, _y2);

  if (seekPage >= 0) {
    for (uint8_t col = 0; col < seekColumn; col++) {
      spi->transfer(firstPageData);
    }

    while (seekPage-- > 0 ) {
      for (uint8_t col = 0; col < seekColumn; col++) {
        spi->transfer(fillData);
      }
    }

    for (uint8_t col = 0; col < seekColumn; col++) {
      spi->transfer(lastPageData);
    }
  } else {
    fillData = firstPageData & lastPageData;

    for (uint8_t col = 0; col < seekColumn; col++) {
      spi->transfer(fillData);
    }
  }

  digitalWrite(this->Display_CS, HIGH);
}

void waveshare_ePaper_spi_ttf::wait_ePaper() {
  while (digitalRead(this->Display_BUSY) == HIGH) { //LOW: idle, HIGH: busy
    delay(100);
  }

  return;
}