#include "ILI9341_ttf.h"

SPIClass *outputttf::spi;

/* constructor */
outputttf::outputttf(SPIClass *_spi) {
  this->spi = _spi;
}

void outputttf::setTruetype(truetypeClass *_ttf) {
  this->font = _ttf;
}

void outputttf::setSPIpin(uint8_t _cs, uint8_t _reset, uint8_t _dc) {
  this->TFT_CS = _cs;
  this->TFT_RESET = _reset;
  this->TFT_DC = _dc;
}

void outputttf::setColor(uint16_t _inside, uint16_t _outline, uint16_t _background) {
  this->insideColor = _inside;
  this->outlineColor = _outline;
  this->backgroundColor = _background;
}

uint16_t outputttf::displayString(uint16_t start_x, uint16_t start_y, const char character[], char characterSize, char characterSpace) {
  uint8_t c = 0;
  
  while (character[c]) {
    font->readGlyph(character[c]);
    font->adjustGlyph();
    start_x += outputTFT(start_x, start_y, characterSize);
    start_x += characterSpace; //space between charctor
    font->freeGlyph();

    c++;
  }
  return start_x;
}

uint16_t outputttf::displayString(uint16_t start_x, uint16_t start_y, const wchar_t character[], char characterSize, char characterSpace) {
  uint8_t c = 0;
  
  while (character[c]) {
    font->readGlyph(character[c]);
    font->adjustGlyph();
    start_x += outputTFT(start_x, start_y, characterSize);
    start_x += characterSpace; //space between charctor
    font->freeGlyph();

    c++;
  }
  return start_x;
}

uint8_t outputttf::outputTFT(uint16_t _x, uint16_t _y, uint16_t _height) {
  uint8_t width = font->generateBitmap(_height);

  //---Code for displaying a bitmap in ILI9341
  //Rectangle setting required for drawing ILI9341
  digitalWrite(this->TFT_CS, LOW);
  ILI9341_set_rect(_x, (_x + width - 1), _y, (_y + _height - 1));

  //Drawing character
  for (uint16_t pixel_y = 0; pixel_y < _height; pixel_y++) {
    for (uint16_t pixel_x = 0; pixel_x < width; pixel_x++) {
      uint16_t pixelColor;

      if (font->getPixel(pixel_x, pixel_y, width)) {
        pixelColor = this->outlineColor; //Drawing outline of character
      } else if (font->isInside(pixel_x, pixel_y)) {
        pixelColor = this->insideColor; //Fill character
      } else {
        pixelColor = this->backgroundColor; //Character background color
      }

      spi->transfer16(pixelColor);
    }
  }
  digitalWrite(this->TFT_CS, HIGH);
  //Drawing character end
  //---Code for displaying a bitmap in ILI9341 end

  font->freeBitmap();

  return width;
}

//Rectangle setting required for drawing ILI9341
void outputttf::ILI9341_set_rect(uint16_t _x1, uint16_t _x2, uint16_t _y1, uint16_t _y2) {
  digitalWrite(this->TFT_DC, LOW);
  spi->transfer(CMD_COLUMN_ADDRESS_SET);
  digitalWrite(this->TFT_DC, HIGH);
  spi->transfer16(_x1);
  spi->transfer16(_x2);
  digitalWrite(this->TFT_DC, LOW);
  spi->transfer(CMD_PAGE_ADDRESS_SET);
  digitalWrite(this->TFT_DC, HIGH);
  spi->transfer16(_y1);
  spi->transfer16(_y2);
  digitalWrite(this->TFT_DC, LOW);
  spi->transfer(CMD_MEMORY_WRITE);
  digitalWrite(this->TFT_DC, HIGH);
}
