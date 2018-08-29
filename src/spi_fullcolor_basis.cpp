#include "spi_fullcolor_basis.h"

SPIClass *ttfSpiFullColor::spi;

//constructor
ttfSpiFullColor::ttfSpiFullColor(SPIClass *_spi) {
  this->spi = _spi;
}

void ttfSpiFullColor::setTruetype(truetypeClass *_ttf) {
  this->font = _ttf;
}

void ttfSpiFullColor::setSPIpin(uint8_t _cs, uint8_t _reset, uint8_t _dc) {
  this->TFT_CS = _cs;
  this->TFT_RESET = _reset;
  this->TFT_DC = _dc;
}

void ttfSpiFullColor::setColor(uint16_t _inside, uint16_t _outline, uint16_t _background) {
  this->insideColor = _inside;
  this->outlineColor = _outline;
  this->backgroundColor = _background;
}

uint8_t ttfSpiFullColor::displayString(uint8_t start_x, uint8_t start_y, const char character[], uint8_t characterSize, uint8_t characterSpace) {
  uint8_t c = 0;
  
  while (character[c]) {
    font->readGlyph(character[c]);
    font->adjustGlyph();
    start_x += outputTFT(start_x, start_y, characterSize);
    start_x += characterSpace; //space between charctor
    fill_rect(start_x - characterSpace, start_x - 1, start_y, start_y + characterSize, this->backgroundColor);
    c++;
  }
  return start_x;
}

uint8_t ttfSpiFullColor::displayString(uint8_t start_x, uint8_t start_y, const wchar_t character[], uint8_t characterSize, uint8_t characterSpace) {
  uint8_t c = 0;
  
  while (character[c]) {
    font->readGlyph(character[c]);
    font->adjustGlyph();
    start_x += outputTFT(start_x, start_y, characterSize);
    start_x += characterSpace; //space between charctor
    fill_rect(start_x - characterSpace, start_x - 1, start_y, start_y + characterSize, this->backgroundColor);
    c++;
  }
  return start_x;
}

uint8_t ttfSpiFullColor::displayMonospaced(uint8_t start_x, uint8_t start_y, const char character[], uint8_t characterSize, uint8_t monospacedWidth) {
  uint8_t c = 0;
  
  while (character[c]) {
    uint8_t width;
    font->readGlyph(character[c]);
    font->adjustGlyph();
    outputTFT(start_x, start_y, characterSize, monospacedWidth);
    start_x += monospacedWidth;
    c++;
  }
  return start_x;
}

uint8_t ttfSpiFullColor::outputTFT(uint8_t _x, uint8_t _y, uint8_t _height, uint8_t monospacedWidth) {
  //---Code for displaying a bitmap
  uint8_t width = font->generateBitmap(_height);

  //In case of the monospaced, align to the right in the frame
  if(monospacedWidth){
    uint8_t surplusWidth = monospacedWidth - width;
    fill_rect(_x, _x + surplusWidth - 1, _y, _y + _height, this->backgroundColor);
    _x += surplusWidth;
  }

  //Rectangle setting required for drawing
  digitalWrite(this->TFT_CS, LOW);
  set_rect(_x, (_x + width - 1), _y, (_y + _height - 1));

  //Drawing character
  for (uint8_t pixel_y = 0; pixel_y < _height; pixel_y++) {
    for (uint8_t pixel_x = 0; pixel_x < width; pixel_x++) {
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
  //---Code for displaying a bitmap end

  font->freeBitmap();
  font->freeGlyph();

  return width;
}

void ttfSpiFullColor::fill_rect(uint16_t _x1, uint16_t _x2, uint16_t _y1, uint16_t _y2, uint16_t _color) {
  uint32_t repeat = (_x2 - _x1 + 1) * (_y2 - _y1 + 1);
  
  digitalWrite(this->TFT_CS, LOW);
  set_rect(_x1, _x2, _y1, _y2);
  for (uint32_t i = 0; i < repeat; i++) {
    spi->transfer16(_color);
  }
  digitalWrite(this->TFT_CS, HIGH);
}
