/*
  output to SSD1306 sample

*/
#include <FS.h>
#include <SPI.h>
#include <SD.h>

#include "SSD1306_spi_ttf.h"

//OLED command list
//#define PORTRAIT
#define LANDSCAPE

#define CMD_DISPLAY_OFF 0xAE
#define CMD_DISPLAY_ON 0xAF
#define CMD_MUX_RATIO 0xA8 //0x3F (0x3F = 64d -1d)
#define CMD_SET_OFFSET 0xD3 //0x00 (no offset)
#define CMD_SET_STARTLINE 0x40 //0x00 (no offset)
#define CMD_REMAP 0xA1 //or 0xA0 re-map, SEG0 is mapped to column address 127
#define CMD_SCAN_DIRECTION 0xC8 //or 0xC0 scan direction, reverse up-bottom
#define CMD_SET_COMPIN 0xDA //0x12 (Alternative configuration, Disable L/R remap)
#define CMD_ROW_CONTRAST 0x81 //0x7F
#define CMD_STOP_SCROLLING 0x2E
#define CMD_RESUME_RAM 0xA4
#define CMD_SET_OSC_FREQ 0xD5 //0x00
#define CMD_EN_CHARGE 0x8D //0x14 enable charge pump
#define CMD_ADDR_MODE 0x20 //[1:0] = 00: Horizontal Addressing Mode ,01: Vertical Addressing Mode, 10: Page Addressing Mode (RESET), 11: Invalid
#define CMD_COLUMN_ADDR 0x21 //0x00 = start, 0x7f = end
#define CMD_PAGE_ADDR 0x22 //0x00 = start, 0x07 = end

#define MAC_PORTRAIT 0xe8
#define MAC_LANDSCAPE 0x00
#define OLED_Buffer_CNT 1024

#if defined(PORTRAIT)
#define MAC_CONFIG MAC_PORTRAIT
#define OLEDWidth 64
#define OLEDHeight 128
#define OLEDPage 8
#else
#define MAC_CONFIG MAC_LANDSCAPE
#define OLEDWidth 128
#define OLEDHeight 64
#define OLEDPage 8
#endif
//OLED command list end

//SPI pin settings
SPIClass OLEDspi = SPIClass(VSPI);
ssd1306_spi_ttf ttfout = ssd1306_spi_ttf(&OLEDspi);
enum {
  OLED_CS = 27,
  OLED_RESET = 26,
  OLED_DC = 25,
  SD_CS = 5
};
//SPI pin settings end

//OLED ttf font
truetypeClass font = truetypeClass(&SD);
truetypeClass font2 = truetypeClass(&SD);
//const char *fontFile2 = "/fonts/ipag.ttf";
//const char *fontFile2 = "/fonts/ipam.ttf";
const char *fontFile2 = "/fonts/hiraginog.ttf";
//const char *fontFile = "/fonts/helvetica.ttf";
const char *fontFile = "/fonts/Avenir.ttf";
//const char *fontFile = "/fonts/TimesNewRoman.ttf";
//const char *fontFile = "/fonts/AppleGaramond.ttf";
//const char *fontFile = "/fonts/myriad.ttf";
//const char *fontFile = "/fonts/ComicSans.ttf";
//OLED ttf font end

void OLED_fill_rect(uint16_t _x1, uint16_t _x2, uint16_t _y1, uint16_t _y2, bool _color, bool _reversal = LOW);

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial); // wait for serial port to connect. Needed for native USB port only

  //OLED pin settings
  pinMode(OLED_CS, OUTPUT);
  pinMode(OLED_RESET, OUTPUT);
  pinMode(OLED_DC, OUTPUT);
  //OLED pin settings end

  //SPI initilizetion
  OLEDspi.begin();
  OLEDspi.setFrequency(10000000);
  OLEDspi.setDataMode(SPI_MODE0);
  //SPI initilizetion end

  //OLED initilizetion
  digitalWrite(OLED_RESET, LOW);
  delay(1);
  digitalWrite(OLED_RESET, HIGH);

  OLED_init_CMD();
  OLED_fill_rect(0, OLEDWidth - 1, 0, OLEDHeight - 1, LOW); //fill black
  //LCD initilizetion end

  font.begin(SD_CS, fontFile);
  font2.begin(SD_CS, fontFile2);

  ttfout.setTruetype(&font); //set font. use with "truetype.h"
  ttfout.setSPIpin(OLED_CS, OLED_RESET, OLED_DC); //set SPI for ILI9341 pin (CS, RESET, DC)
  ttfout.setColor(HIGH, HIGH, LOW); //set color (inside, outline, background)

  ttfout.displayString(5, 0, "12359ab", 30, 2); //output string (start_x, start_y, string(char), height of charctor, inter-character space)
  font.end();

  ttfout.setTruetype(&font2); //set font. use with "truetype.h"
  ttfout.displayString(5, 32, "12359ab", 30, 2); //output string (start_x, start_y, string(char), height of charctor, inter-character space)
  delay(3000);

  //fill black
  OLED_fill_rect(0, OLEDWidth - 1, 0, OLEDHeight - 1, LOW);

  //display kanji
  //ttfout.displayString(10, 1, L"埼玉県", 40, 2); //output string (start_x, start_y, string(wchar_t, 2byte), height of charctor, inter-character space)
  //delay(1000);

  //fill black
  OLED_fill_rect(0, OLEDWidth - 1, 0, OLEDHeight - 1, LOW);

  //count up
  ttfout.setColor(LOW, HIGH, LOW); //set color (inside, outline, background)
  for (int i = 0; i <= 100; i++) {
    char number[4];
    sprintf(number, "%03d", i);
    ttfout.displayMonospaced(2, 2, number, 60, 40); //output string (start_x, start_y, string(char), height of charctor, Monospace value)
  }
  ttfout.setColor(HIGH, HIGH, LOW); //set color (inside, outline, background)

  //fill black
  OLED_fill_rect(0, OLEDWidth - 1, 0, OLEDHeight - 1, LOW);
}

//unicode start bit
uint32_t i = 0x4e9c;
//0x21 number
//0x4e9c //kanji in Japanese fonts.
wchar_t charctor[2] = {i, 0};

void loop() {
  //display all unicode charactor
  ttfout.displayString(40, 2, charctor, 60, 10);
  charctor[0]++;

  delay(10);
}

void OLED_init_CMD() {
  digitalWrite(OLED_CS, LOW);
  digitalWrite(OLED_DC, LOW);

  OLEDspi.transfer(CMD_DISPLAY_OFF); //Set Display Off
  OLEDspi.transfer(CMD_MUX_RATIO);
  OLEDspi.transfer(0x3F);
  OLEDspi.transfer(CMD_SET_OFFSET);
  OLEDspi.transfer(0x00);
  OLEDspi.transfer(CMD_SET_STARTLINE);
  OLEDspi.transfer(CMD_REMAP);
  OLEDspi.transfer(CMD_SCAN_DIRECTION);
  OLEDspi.transfer(CMD_SET_COMPIN);
  OLEDspi.transfer(0x12);
  OLEDspi.transfer(CMD_ROW_CONTRAST);
  OLEDspi.transfer(0x7F);
  OLEDspi.transfer(CMD_STOP_SCROLLING);
  OLEDspi.transfer(CMD_RESUME_RAM);
  OLEDspi.transfer(CMD_SET_OSC_FREQ);
  OLEDspi.transfer(0x00);
  OLEDspi.transfer(CMD_EN_CHARGE);
  OLEDspi.transfer(0x14);
  OLEDspi.transfer(CMD_ADDR_MODE);
  OLEDspi.transfer(0x00);
  OLEDspi.transfer(CMD_COLUMN_ADDR);
  OLEDspi.transfer(0x00);
  OLEDspi.transfer(0x7f);
  OLEDspi.transfer(CMD_PAGE_ADDR);
  OLEDspi.transfer(0x00);
  OLEDspi.transfer(0x07);
  OLEDspi.transfer(CMD_DISPLAY_ON);

  digitalWrite(OLED_DC, HIGH);
  digitalWrite(OLED_CS, HIGH);

  delay(150);
}

void OLED_fill_rect(uint16_t _x1, uint16_t _x2, uint16_t _y1, uint16_t _y2, bool _color, bool _reversal) {
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

  digitalWrite(OLED_CS, LOW);

  OLED_set_rect(_x1, _x2, _y1, _y2);

  if (seekPage >= 0) {
    for (uint8_t col = 0; col < seekColumn; col++) {
      OLEDspi.transfer(firstPageData);
    }

    while (seekPage-- > 0 ) {
      for (uint8_t col = 0; col < seekColumn; col++) {
        OLEDspi.transfer(fillData);
      }
    }

    for (uint8_t col = 0; col < seekColumn; col++) {
      OLEDspi.transfer(lastPageData);
    }
  } else {
    fillData = firstPageData & lastPageData;

    for (uint8_t col = 0; col < seekColumn; col++) {
      OLEDspi.transfer(fillData);
    }
  }

  digitalWrite(OLED_CS, HIGH);
}

void OLED_set_rect(uint16_t _x1, uint16_t _x2, uint16_t _y1, uint16_t _y2) {
  uint8_t startPage, endPage;
  startPage = (uint8_t)(_y1 / 8);
  endPage = (uint8_t)(_y2 / 8);

  digitalWrite(OLED_DC, LOW);

  OLEDspi.transfer(CMD_COLUMN_ADDR);
  OLEDspi.transfer((uint8_t)_x1);
  OLEDspi.transfer((uint8_t)_x2);
  OLEDspi.transfer(CMD_PAGE_ADDR);
  OLEDspi.transfer(startPage);
  OLEDspi.transfer(endPage);

  digitalWrite(OLED_DC, HIGH);
}
