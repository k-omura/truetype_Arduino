#include "SSD1331_ttf.h"

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

uint8_t outputttf::displayString(uint8_t start_x, uint8_t start_y, const char character[], uint8_t characterSize, uint8_t characterSpace) {
  uint8_t c = 0;
  
  while (character[c]) {
    font->readGlyph(character[c]);
    font->adjustGlyph();
    start_x += outputTFT(start_x, start_y, characterSize);
    start_x += characterSpace; //space between charctor
    font->freeGlyph();
    SSD1331_fill_rect(start_x - characterSpace, start_x - 1, start_y, start_y + characterSize, this->backgroundColor, this->backgroundColor);
    c++;
  }
  return start_x;
}

uint8_t outputttf::displayString(uint8_t start_x, uint8_t start_y, const wchar_t character[], uint8_t characterSize, uint8_t characterSpace) {
  uint8_t c = 0;
  
  while (character[c]) {
    font->readGlyph(character[c]);
    font->adjustGlyph();
    start_x += outputTFT(start_x, start_y, characterSize);
    start_x += characterSpace; //space between charctor
    font->freeGlyph();
    SSD1331_fill_rect(start_x - characterSpace, start_x - 1, start_y, start_y + characterSize, this->backgroundColor, this->backgroundColor);
    c++;
  }
  return start_x;
}

uint8_t outputttf::displayMonospaced(uint8_t start_x, uint8_t start_y, const char character[], uint8_t characterSize, uint8_t monospacedWidth) {
  uint8_t c = 0;
  
  while (character[c]) {
    uint8_t width;
    font->readGlyph(character[c]);
    font->adjustGlyph();
    outputTFT(start_x, start_y, characterSize, monospacedWidth);
    start_x += monospacedWidth;
    font->freeGlyph();
    c++;
  }
  return start_x;
}

uint8_t outputttf::outputTFT(uint8_t _x, uint8_t _y, uint8_t _height, uint8_t monospacedWidth) {
  //---Code for displaying a bitmap in SSD1331
  uint8_t width = font->generateBitmap(_height);

  //In case of the monospaced, align to the right in the frame
  if(monospacedWidth){
    uint8_t surplusWidth = monospacedWidth - width;
    SSD1331_fill_rect(_x, _x + surplusWidth - 1, _y, _y + _height, this->backgroundColor, this->backgroundColor);
    _x += surplusWidth;
  }

  //Rectangle setting required for drawing SSD1331
  digitalWrite(this->TFT_CS, LOW);
  SSD1331_set_rect(_x, (_x + width - 1), _y, (_y + _height - 1));

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
  //---Code for displaying a bitmap in SSD1331 end

  font->freeBitmap();

  return width;
}

//Rectangle setting required for drawing SSD1331
void outputttf::SSD1331_set_rect(uint8_t _x1, uint8_t _x2, uint8_t _y1, uint8_t _y2) {
  digitalWrite(this->TFT_DC, LOW);
  spi->transfer(CMD_COLUMN_ADDRESS);
  spi->transfer(_x1);
  spi->transfer(_x2);
  spi->transfer(CMD_ROW_ADDRESS);
  spi->transfer(_y1);
  spi->transfer(_y2);
  digitalWrite(this->TFT_DC, HIGH);
}

void outputttf::SSD1331_fill_rect(uint8_t _x1, uint8_t _x2, uint8_t _y1, uint8_t _y2, uint16_t _outline, uint16_t _innner) {
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

  color.raw = _outline;
  spi->transfer(color.rgb.b << 1);
  spi->transfer(color.rgb.g);
  spi->transfer(color.rgb.r << 1);

  color.raw = _innner;
  spi->transfer(color.rgb.b << 1);
  spi->transfer(color.rgb.g);
  spi->transfer(color.rgb.r << 1);

  digitalWrite(this->TFT_DC, HIGH);
  digitalWrite(this->TFT_CS, HIGH);
}
