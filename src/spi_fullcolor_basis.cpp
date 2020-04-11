#include "spi_fullcolor_basis.h"

SPIClass *ttfSpiFullColor::spi;

//constructor
ttfSpiFullColor::ttfSpiFullColor(SPIClass *_spi) {
  this->spi = _spi;
#if defined(DMA_ENABLE)
  //dmaSetting();
#endif
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

uint8_t ttfSpiFullColor::displayString(uint8_t start_x, uint8_t start_y, const char character[], uint8_t characterSize, uint8_t characterSpace) {
  uint8_t c = 0;

  while (character[c]) {
    font->readGlyph(character[c]);
    font->adjustGlyph();
    start_x += outputTFT(start_x, start_y, characterSize);
    start_x += characterSpace; //space between charctor
    fill_rect(start_x - characterSpace, start_x - 1, start_y, start_y + characterSize - 1);
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
    fill_rect(start_x - characterSpace, start_x - 1, start_y, start_y + characterSize - 1);
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

//SPI DMA interrupt hundler
void ttfSpiFullColor::spiDMAHundler() {
  /*while(spi->dev()->regs->SR & SPI_SR_BSY);
  spi->dev()->regs->DR = 0;*/
}

void ttfSpiFullColor::dmaSetting() {
  /*spi->dev()->regs->CR1 |=SPI_CR1_BIDIMODE_1_LINE|SPI_CR1_BIDIOE; // 送信のみ利用の設定
  dma_init(DMA1);
  dma_attach_interrupt(DMA1, DMA_CH3, &spiDMAHundler);
  spi_tx_dma_enable(spi->.dev());*/
}

void ttfSpiFullColor::spiSendbuf(uint8_t *_buf, uint16_t _length) {
#if defined(DMA_ENABLE)
  /*dma_setup_transfer(
    DMA1,DMA_CH3,         // SPI1用DMAチャンネル3を指定
    &SPI.dev()->regs->DR, // 転送先アドレス    ：SPIデータレジスタを指定
    DMA_SIZE_8BITS,       // 転送先データサイズ : 1バイト
    _buf,          // 転送元アドレス     : SRAMアドレス
    DMA_SIZE_8BITS,       // 転送先データサイズ : 1バイト
    DMA_MINC_MODE|        // フラグ: サイクリック
    DMA_FROM_MEM|         //         メモリから周辺機器、転送完了割り込み呼び出しあり
    DMA_TRNS_CMPLT        //         転送完了割り込み呼び出しあり
  );
  dma_set_num_transfers(DMA1, DMA_CH3, _length); // 転送サイズ指定
  dma_enable(DMA1, DMA_CH3);  // DMA有効化*/
#else
  spi->transfer(_buf, _length);
#endif
}

uint8_t ttfSpiFullColor::outputTFT(uint8_t _x, uint8_t _y, uint8_t _height, uint8_t monospacedWidth) {
  //---Code for displaying a bitmap
  uint8_t width = font->generateBitmap(_height);

  //In case of the monospaced, align to the right in the frame
  if (monospacedWidth) {
    uint8_t surplusWidth = monospacedWidth - width;
    fill_rect(_x, _x + surplusWidth - 1, _y, _y + _height - 1);
    _x += surplusWidth;
  }

  //Rectangle setting required for drawing
  digitalWrite(this->Display_CS, LOW);
  set_rect(_x, (_x + width - 1), _y, (_y + _height - 1));

  uint8_t spi_tx_buff[2 * width];
  transmit16 transmitData;

  //Drawing character
  for (uint8_t pixel_y = 0; pixel_y < _height; pixel_y++) {
    for (uint8_t pixel_x = 0; pixel_x < width; pixel_x++) {
      uint16_t pixelColor;

      if (font->getPixel(pixel_x, pixel_y, width)) {
        transmitData.raw = this->outlineColor; //Drawing outline of character
      } else if (font->isInside(pixel_x, pixel_y)) {
        transmitData.raw = this->insideColor; //Fill character
      } else {
        transmitData.raw = this->backgroundColor; //Character background color
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
