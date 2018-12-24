#include "SSD1306_spi_ttf.h"

//constructor
ssd1306_spi_ttf::ssd1306_spi_ttf(SPIClass *_spi) : ttfSpiMonoColor(_spi) {};

uint8_t ssd1306_spi_ttf::outputDisplay(uint8_t _x, uint8_t _y, uint8_t _height, uint8_t monospacedWidth) {
  //---Code for displaying a bitmap
  uint8_t width = font->generateBitmap(_height);

  //In case of the monospaced, align to the right in the frame
  if (monospacedWidth) {
    uint8_t surplusWidth = monospacedWidth - width;
    fill_rect(_x, _x + surplusWidth - 1, _y, _y + _height, this->backgroundColor);
    _x += surplusWidth;
  }

  uint8_t lastPage = (uint8_t)((_height) / 8) + 1;
  int8_t seekPageNum = lastPage - 2;
  uint8_t page_y = 0;

  //Rectangle setting required for drawing
  digitalWrite(this->Display_CS, LOW);
  set_rect(_x, (_x + width - 1), _y, (_y + _height - 1));
  
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

      fillData >>= 1;
      if (pixel) {
        fillData += 0b10000000;
      }
    }
    spi->transfer(fillData);
  }
  page_y += firstPageBit;

  for (int8_t page = 0; page < seekPageNum; page++) {
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

        fillData >>= 1;
        if (pixel) {
          fillData += 0b10000000;
        }
      }
      spi->transfer(fillData);
    }
    page_y += 8;
  }

  uint8_t lastPageBit =  8 - ((_y + _height) % 8);
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

      fillData >>= 1;
      if (pixel) {
        fillData += 0b10000000;
      }
    }
    fillData >>= (8 - lastPageBit);
    spi->transfer(fillData);
  }

  //Drawing character end
  digitalWrite(this->Display_CS, HIGH);
  //---Code for displaying a bitmap end

  font->freeBitmap();
  font->freeGlyph();

  return width;
}

//Rectangle setting required for drawing SSD1331
void ssd1306_spi_ttf::set_rect(uint16_t _x1, uint16_t _x2, uint16_t _y1, uint16_t _y2) {
  uint8_t startPage, endPage;
  startPage = (uint8_t)(_y1 / 8);
  endPage = (uint8_t)(_y2 / 8);

  digitalWrite(this->Display_DC, LOW);

  spi->transfer(CMD_COLUMN_ADDR);
  spi->transfer((uint8_t)_x1);
  spi->transfer((uint8_t)_x2);
  spi->transfer(CMD_PAGE_ADDR);
  spi->transfer(startPage);
  spi->transfer(endPage);

  digitalWrite(this->Display_DC, HIGH);

  return;
}

void ssd1306_spi_ttf::fill_rect(uint16_t _x1, uint16_t _x2, uint16_t _y1, uint16_t _y2, bool _color, bool _reversal) {
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
