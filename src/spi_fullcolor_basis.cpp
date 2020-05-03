#include "spi_fullcolor_basis.h"



SPIClass *ttfSpiFullColor::spi;

//constructor
ttfSpiFullColor::ttfSpiFullColor(SPIClass *_spi) {
  this->spi = _spi;
}

void ttfSpiFullColor::optimisationInit() {
  this->optimisation = true;
  optimisationSetting();
}

void ttfSpiFullColor::setTruetype(truetypeClass *_ttf) {
  this->font = _ttf;
}

void ttfSpiFullColor::setSPIpin(uint8_t _cs, uint8_t _reset, uint8_t _dc) {
  this->Display_CS = _cs;
  this->Display_RESET = _reset;
  this->Display_DC = _dc;
}

void ttfSpiFullColor::setColor(uint16_t _inside, uint16_t _outline, uint16_t _background) {
  this->insideColor = _inside;
  this->outlineColor = _outline;
  this->backgroundColor = _background;
}

void ttfSpiFullColor::setUnderLine(bool _allowUnderLine) {
  this->underLine = _allowUnderLine;
}

uint8_t ttfSpiFullColor::displayString(uint16_t start_x, uint16_t start_y, const char character[], uint16_t characterSize, uint8_t characterSpace) {
  uint8_t c = 0;

  while (character[c]) {
    font->readGlyph(character[c]);
    font->adjustGlyph();
    start_x += outputTFT(start_x, start_y, characterSize);
    start_x += characterSpace; //space between charctor
    if(characterSpace){
      fill_rect(start_x - characterSpace, start_x - 1, start_y, start_y + characterSize - 1);
    }
    c++;
  }
  return start_x;
}

uint8_t ttfSpiFullColor::displayString(uint16_t start_x, uint16_t start_y, const wchar_t character[], uint16_t characterSize, uint8_t characterSpace) {
  uint8_t c = 0;

  while (character[c]) {
    font->readGlyph(character[c]);
    font->adjustGlyph();
    start_x += outputTFT(start_x, start_y, characterSize);
    start_x += characterSpace; //space between charctor
    if(characterSpace){
      fill_rect(start_x - characterSpace, start_x - 1, start_y, start_y + characterSize - 1);
    }
    c++;
  }
  return start_x;
}

uint8_t ttfSpiFullColor::displayMonospaced(uint16_t start_x, uint16_t start_y, const char character[], uint16_t characterSize, uint8_t monospacedWidth) {
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

//SPI DMA interrupt hundler
void ttfSpiFullColor::spiDMAHundler() {
  /*while(spi->dev()->regs->SR & SPI_SR_BSY);
  spi->dev()->regs->DR = 0;*/
}

void ttfSpiFullColor::optimisationSetting() {
#if defined(_VARIANT_ARDUINO_STM32_)
  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel3_IRQn);

  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_HARD_OUTPUT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;//MIN: SSD1331 64, ILI9341 8
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK){
    Error_Handler();
  }

  //Serial.println("SPI optimisation set!!!!!!!!!!!");
#endif
}

void ttfSpiFullColor::spiSendbuf(uint8_t *_buf, uint16_t _length) {
  if(this->optimisation){
    #if defined(_VARIANT_ARDUINO_STM32_)
      //while (__HAL_DMA_GET_COUNTER(&hdma_spi1_tx) > 0);
      HAL_SPI_Init(&hspi1);
      //HAL_SPI_Transmit_DMA(&hspi1, _buf, _length);
      HAL_SPI_Transmit(&hspi1, _buf, _length, 1000);
    #endif
  }else{
    spi->transfer(_buf, _length);
  }
}

uint8_t ttfSpiFullColor::outputTFT(uint16_t _x, uint16_t _y, uint16_t _height, uint8_t monospacedWidth) {
  //---Code for displaying a bitmap
  uint16_t width = font->generateBitmap(_height);

  //In case of the monospaced, align to the right in the frame
  if (monospacedWidth) {
    uint16_t surplusWidth = monospacedWidth - width;
    fill_rect(_x, _x + surplusWidth - 1, _y, _y + _height - 1);
    _x += surplusWidth;
  }

  //Rectangle setting required for drawing
  digitalWrite(this->Display_CS, LOW);
  set_rect(_x, (_x + width - 1), _y, (_y + _height - 1));

  uint8_t spi_tx_buff[2 * width];
  transmit16 transmitData;
  bool prevOnline = false;
  bool insideNow = false;

  //Drawing character
  for (uint16_t pixel_y = 0; pixel_y < _height; pixel_y++) {
    for (uint16_t pixel_x = 0; pixel_x < width; pixel_x++) {
      uint16_t pixelColor;

      transmitData.raw = this->backgroundColor; //Character background color
      if (font->getPixel(pixel_x, pixel_y, width) == 1) {
        transmitData.raw = this->outlineColor; //Drawing outline of character
        prevOnline = true;
        insideNow = false;
      } else if (insideNow){
        transmitData.raw = this->insideColor; //Fill character
        prevOnline = false;
      } else if (prevOnline) {
        if(font->isInside(pixel_x, pixel_y)){
          transmitData.raw = this->insideColor; //Fill character
          insideNow = true;
        }
        prevOnline = false;
      }

      spi_tx_buff[2 * pixel_x] = transmitData.split.low;
      spi_tx_buff[2 * pixel_x + 1] = transmitData.split.high;
    }
    spiSendbuf(spi_tx_buff, 2 * width);
  }
  digitalWrite(this->Display_CS, HIGH);
  //Drawing character end
  //---Code for displaying a bitmap end

  font->freeBitmap();
  font->freeGlyph();

  return width;
}

void ttfSpiFullColor::fill_rect(uint16_t _x1, uint16_t _x2, uint16_t _y1, uint16_t _y2) {
  uint16_t repeatCol = 2 * (_x2 - _x1 + 1);
  uint16_t repeatPage = _y2 - _y1 + 1;
  uint8_t spi_tx_buff[repeatCol];

  transmit16 transmitData;
  transmitData.raw = this->backgroundColor;

  digitalWrite(this->Display_CS, LOW);
  set_rect(_x1, _x2, _y1, _y2);
  for (uint16_t page = 0; page < repeatPage; page++) {
    for (uint32_t col = 0; col < repeatCol; col += 2) {
      spi_tx_buff[col] = transmitData.split.low;
      spi_tx_buff[col + 1] = transmitData.split.high;
    }
    spiSendbuf(spi_tx_buff, repeatCol);
  }
  digitalWrite(this->Display_CS, HIGH);
}

void ttfSpiFullColor::fill_all() {
  fill_rect(0, this->tftWidth, 0, this->tftHeifht);
}
