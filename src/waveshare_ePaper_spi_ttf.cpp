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
  displayPortrait = portrait;
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
  if (displayPortrait) {
    spi->transfer(0x03); // X increment; Y increment
  } else {
    spi->transfer(0x05); // X increment; Y decrement, the address counter is updated in the Y direction.
  }

  digitalWrite(this->Display_CS, HIGH);
  wait_ePaper();
  //init commands end

  lut = lut_partial_update;
  setLut();

  return;
}

//set the look-up table register
void waveshare_ePaper_spi_ttf::setLut() {
  digitalWrite(this->Display_CS, LOW);

  digitalWrite(this->Display_DC, LOW);
  spi->transfer(WRITE_LUT_REGISTER);
  digitalWrite(this->Display_DC, HIGH);

  //the length of look-up table is 30 bytes
  for (int i = 0; i < 30; i++) {
    spi->transfer(this->lut[i]);
  }

  digitalWrite(this->Display_CS, HIGH);
  wait_ePaper();

  return;
}

uint8_t waveshare_ePaper_spi_ttf::outputDisplay(uint8_t _x, uint8_t _y, uint8_t _height, uint8_t monospacedWidth) {
  //---Code for displaying a bitmap
  uint8_t width = font->generateBitmap(_height);

  //In case of the monospaced, align to the right in the frame
  if (monospacedWidth) {
    uint8_t surplusWidth = monospacedWidth - width;
    fill_rect(_x, _x + surplusWidth - 1, _y, _y + _height - 1, this->backgroundColor);
    _x += surplusWidth;
  }

  int8_t midSeekPageNum = (uint8_t)((_y + _height - 1) / 8) - (uint8_t)(_y / 8) - 1;
  uint8_t page_y = 0;

  //Rectangle setting required for drawing
  digitalWrite(this->Display_CS, LOW);
  set_rect(_y, (_y + _height - 1), ePaperWidth - _x, ePaperWidth - (_x + width - 1));
  setMemoryPointer(_y, ePaperWidth - _x);

  digitalWrite(this->Display_DC, LOW);
  spi->transfer(WRITE_RAM);
  digitalWrite(this->Display_DC, HIGH);

  //Drawing character
  uint8_t firstPageBit = 8 - (_y % 8);
  for (uint8_t pixel_x = 0; pixel_x < width; pixel_x++) {
    uint8_t fillData = 0b00000000;
    for (uint8_t bitCount = 0; bitCount < firstPageBit; bitCount++) {
      bool pixel;
      if (font->getPixel(pixel_x, bitCount, width)) {
        pixel = this->outlineColor; //Drawing outline of character
      } else if (font->isInside(pixel_x, bitCount)) {
        pixel = this->insideColor; //Fill character
      } else {
        pixel = this->backgroundColor; //Character background color
      }

      fillData <<= 1;
      if (pixel) {
        fillData += 0b00000001;
      }
    }
    spi->transfer(~fillData);
  }
  page_y += firstPageBit;

  for (int8_t page = 0; page < midSeekPageNum; page++) {
    for (uint8_t pixel_x = 0; pixel_x < width; pixel_x++) {
      uint8_t fillData = 0b00000000;
      for (uint8_t bitCount = 0; bitCount < 8; bitCount++) {
        bool pixel;
        if (font->getPixel(pixel_x, page_y + bitCount, width)) {
          pixel = this->outlineColor; //Drawing outline of character
        } else if (font->isInside(pixel_x, page_y + bitCount)) {
          pixel = this->insideColor; //Fill character
        } else {
          pixel = this->backgroundColor; //Character background color
        }

        fillData <<= 1;
        if (pixel) {
          fillData += 0b00000001;
        }
      }
      spi->transfer(~fillData);
    }
    page_y += 8;
  }

  uint8_t lastPageBit =  (_y + _height - 1) % 8;
  for (uint8_t pixel_x = 0; pixel_x < width; pixel_x++) {
    uint8_t fillData = 0b00000000;
    for (uint8_t bitCount = 0; bitCount < lastPageBit; bitCount++) {
      bool pixel;
      if (font->getPixel(pixel_x, page_y + bitCount, width)) {
        pixel = this->outlineColor; //Drawing outline of character
      } else if (font->isInside(pixel_x, page_y + bitCount)) {
        pixel = this->insideColor; //Fill character
      } else {
        pixel = this->backgroundColor; //Character background color
      }

      fillData <<= 1;
      if (pixel) {
        fillData += 0b00000001;
      }
    }
    fillData <<= (8 - lastPageBit);
    spi->transfer(~fillData);
  }

  //Drawing character end
  digitalWrite(this->Display_CS, HIGH);
  wait_ePaper();
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
  uint8_t seekColumn;
  int8_t seekPage;
  uint8_t firstPageBit, lastPageBit;
  uint8_t fillData, firstPageData, lastPageData;

  seekColumn = _x2 - _x1 + 1;
  seekPage = (uint8_t)(_y2 / 8) - (uint8_t)(_y1 / 8) - 1;
  firstPageBit = (_y1 % 8);
  lastPageBit = (_y2 % 8);

  fillData = 0b11111111;
  firstPageData = fillData >> (firstPageBit);
  lastPageData = fillData << (7 - lastPageBit);

  if (_reversal) {
    fillData = ~fillData;
    firstPageData = ~firstPageData;
    lastPageData = ~lastPageData;
  }

  if (!_color) {
    fillData = 0b11111111;
    firstPageData = 0b11111111;
    lastPageData = 0b11111111;
  }

  digitalWrite(this->Display_CS, LOW);
  set_rect(_y1, _y2, ePaperWidth - _x1, ePaperWidth - _x2);
  setMemoryPointer(_y1, ePaperWidth - _x1);

  digitalWrite(this->Display_DC, LOW);
  spi->transfer(WRITE_RAM);
  digitalWrite(this->Display_DC, HIGH);

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
  wait_ePaper();

  return;
}

void waveshare_ePaper_spi_ttf::wait_ePaper() {
  while (digitalRead(this->Display_BUSY) == HIGH) { //LOW: idle, HIGH: busy
    delay(100);
  }

  return;
}
