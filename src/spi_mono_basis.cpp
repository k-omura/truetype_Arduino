#include "spi_mono_basis.h"

SPIClass *ttfSpiMonoColor::spi;

//constructor
ttfSpiMonoColor::ttfSpiMonoColor(SPIClass *_spi) {
  this->spi = _spi;
}

void ttfSpiMonoColor::setTruetype(truetypeClass *_ttf) {
  this->font = _ttf;
}

void ttfSpiMonoColor::setSPIpin(uint8_t _cs, uint8_t _reset, uint8_t _dc) {
  this->Display_CS = _cs;
  this->Display_RESET = _reset;
  this->Display_DC = _dc;
}

void ttfSpiMonoColor::setColor(bool _inside, bool _outline, bool _background) {
  this->insideColor = _inside;
  this->outlineColor = _outline;
  this->backgroundColor = _background;
}

void ttfSpiMonoColor::setUnderLine(bool _allowUnderLine) {
  this->underLine = _allowUnderLine;
}

uint8_t ttfSpiMonoColor::displayString(uint8_t start_x, uint8_t start_y, const char character[], uint8_t characterSize, uint8_t characterSpace) {
  uint8_t c = 0;

  while (character[c]) {
    font->readGlyph(character[c]);
    font->adjustGlyph();
    start_x += outputDisplay(start_x, start_y, characterSize);
    start_x += characterSpace; //space between charctor
    fill_rect(start_x - characterSpace, start_x - 1, start_y, start_y + characterSize - 1, this->backgroundColor);
    c++;
  }
  return start_x;
}

uint8_t ttfSpiMonoColor::displayString(uint8_t start_x, uint8_t start_y, const wchar_t character[], uint8_t characterSize, uint8_t characterSpace) {
  uint8_t c = 0;

  while (character[c]) {
    font->readGlyph(character[c]);
    font->adjustGlyph();
    start_x += outputDisplay(start_x, start_y, characterSize);
    start_x += characterSpace; //space between charctor
    fill_rect(start_x - characterSpace, start_x - 1, start_y, start_y + characterSize - 1, this->backgroundColor);
    c++;
  }
  return start_x;
}

uint8_t ttfSpiMonoColor::displayMonospaced(uint8_t start_x, uint8_t start_y, const char character[], uint8_t characterSize, uint8_t monospacedWidth) {
  uint8_t c = 0;

  while (character[c]) {
    uint8_t width;
    font->readGlyph(character[c]);
    font->adjustGlyph();
    outputDisplay(start_x, start_y, characterSize, monospacedWidth);
    start_x += monospacedWidth;
    c++;
  }
  return start_x;
}

uint8_t ttfSpiMonoColor::outputDisplay(uint8_t _x, uint8_t _y, uint8_t _height, uint8_t monospacedWidth) {
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

        fillData >>= 1;
        if (pixel) {
          fillData += 0b10000000;
        }
      }
      spi->transfer(fillData);
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
