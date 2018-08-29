/*
  TrueType™ Reference Manual
  https://developer.apple.com/fonts/TrueType-Reference-Manual/

  output to ILI9341 sample

*/

#include <FS.h>
#include <SPI.h>
#include <SD.h>

#include "ILI9341_spi_ttf.h"

//TFT command list
//#define PORTRAIT
#define LANDSCAPE

#define CMD_SLEEP_OUT 0x11
#define CMD_DISPLAY_ON 0x29
#define CMD_COLUMN_ADDRESS_SET 0x2a
#define CMD_PAGE_ADDRESS_SET 0x2b
#define CMD_MEMORY_WRITE 0x2c
#define CMD_MEMORY_ACCESS_CONTROL 0x36
#define CMD_COLMOD 0x3a

#define MAC_PORTRAIT 0xe8
#define MAC_LANDSCAPE 0x48
#define COLMOD_16BIT 0x55
#define COLMOD_18BIT 0x66

#if defined(PORTRAIT)
#define MAC_CONFIG MAC_PORTRAIT
#define tftWidth 320
#define tftHeifht 240
#define AdafruitTFTOrientation 1
#else
#define MAC_CONFIG MAC_LANDSCAPE
#define tftWidth 240
#define tftHeifht 320
#define AdafruitTFTOrientation 0
#endif
//TFT command list end

//TFT color
#define TFT_background 0b0000000000000000
#define TFT_white 0b1111011111011110
#define TFT_RED 0xF800
#define TFT_yellow 0b1110011110000000
#define TFT_blue 0b0011000110011111
#define TFT_green 0b0011011110000011
//TFT color end

//SPI pin settings
SPIClass TFTspi = SPIClass(HSPI);
ILI9341_spi_ttf ttfout = ILI9341_spi_ttf(&TFTspi);
enum {
  TFT_CS = 27,
  TFT_RESET = 26,
  TFT_DC = 25,
  SD_CS = 5
};
//SPI pin settings end

truetypeClass hiragino = truetypeClass(&SD);
//truetypeClass genshin = truetypeClass(&SD);
truetypeClass helvetica = truetypeClass(&SD);
truetypeClass Avenir = truetypeClass(&SD);
truetypeClass TNR = truetypeClass(&SD);
truetypeClass Myriad = truetypeClass(&SD);
//truetypeClass comic = truetypeClass(&SD);
//const char *fontFile = "/fonts/ipag.ttf";
//const char *fontFile = "/fonts/ipam.ttf";
const char *fontHiragino = "/fonts/hiraginog.ttf";
//const char *fontGenshin = "/fonts/gennokaku/GenShinGothic-Medium.ttf";
const char *fontHelvetica = "/fonts/helvetica.ttf";
const char *fontAvenir = "/fonts/Avenir.ttf";
const char *fontTNR = "/fonts/TimesNewRoman.ttf";
const char *fontMyriad = "/fonts/myriad.ttf";
//const char *fontComic = "/fonts/ComicSans.ttf";
//CheekFont-Regular
//TFT ttf font end

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  delay(10);

  //TFT pin settings
  pinMode(TFT_CS, OUTPUT);
  pinMode(TFT_RESET, OUTPUT);
  pinMode(TFT_DC, OUTPUT);
  //TFT pin settings end

  //SPI initilizetion
  TFTspi.begin();
  TFTspi.setClockDivider(SPI_CLOCK_DIV2);
  TFTspi.setDataMode(SPI_MODE0);
  //SPI initilizetion end

  //LCD initilizetion
  digitalWrite(TFT_RESET, LOW); //Datasheet is request 20 ms wait after reset
  delay(20);
  digitalWrite(TFT_RESET, HIGH);
  delay(20);

  digitalWrite(TFT_CS, LOW);
  write_command(CMD_MEMORY_ACCESS_CONTROL);
  write_data(MAC_CONFIG); //Screen orientation setting

  write_command(CMD_COLMOD);
  write_data(COLMOD_16BIT); //16bits mode

  write_command(CMD_SLEEP_OUT);
  delay(60); //Datasheet requests 60 ms wait after waking up
  write_command(CMD_DISPLAY_ON);
  Serial.println("LCD initialized");
  //LCD initilizetion end

  //fill black
  TFT_fillRect(0, tftWidth, 0, tftHeifht, TFT_background);
  Serial.println("fill black");
  digitalWrite(TFT_CS, HIGH);

  //font begin
  hiragino.begin(SD_CS, fontHiragino);
  //genshin.begin(SD_CS, fontGenshin); //error
  helvetica.begin(SD_CS, fontHelvetica);
  Avenir.begin(SD_CS, fontAvenir);
  TNR.begin(SD_CS, fontTNR);
  Myriad.begin(SD_CS, fontMyriad);
  //comic.begin(SD_CS, fontComic); //error
  Serial.println("read fonts");

  //font output to ILI9341 initialize
  ttfout.setTruetype(&hiragino); //set font. use with "truetype.h"
  ttfout.setSPIpin(TFT_CS, TFT_RESET, TFT_DC); //set SPI for ILI9341 pin (CS, RESET, DC)
  ttfout.setColor(TFT_RED, TFT_white, TFT_background); //set color (inside, outline, background)
  Serial.println("output ttf initialized");

  //font output to ILI9341
  ttfout.displayString(30, 30, L"埼玉県", 60, 20); //output string (start_x, start_y, string(wchar_t, 2byte), height of charctor, inter-character space)
  ttfout.displayString(30, 90, L"東京都", 60, 20);
  ttfout.displayString(30, 150, L"千葉県", 60, 20);
  ttfout.displayString(30, 210, L"三鷹市", 60, 20);
  Serial.println("Hello message");

  delay(2000);
  
  //fill black
  TFT_fillRect(0, tftWidth, 0, tftHeifht, TFT_background);
  Serial.println("fill black");
  
  //font output to ILI9341
  ttfout.displayString(0, 0, "12369ab", 50, 5); //output string (start_x, start_y, string(char), height of charctor, inter-character space)
  ttfout.setTruetype(&helvetica);
  ttfout.displayString(0, 50, "12369ab", 50, 5);
  ttfout.setTruetype(&Avenir);
  ttfout.displayString(0, 100, "12369ab", 50, 5);
  ttfout.setTruetype(&TNR);
  ttfout.displayString(0, 150, "12369ab", 50, 5);
  ttfout.setTruetype(&Myriad);
  ttfout.displayString(0, 200, "12569ab", 50, 5);
  
  /*
  ttfout.setTruetype(&comic); //read error
  ttfout.displayString(0, 250, "12569ab", 50, 5);
  //*/

  Serial.println("Display in various fonts");
  delay(1000);

  //fill black
  TFT_fillRect(0, tftWidth, 0, tftHeifht, TFT_background);
  Serial.println("fill black");

  ttfout.setTruetype(&hiragino); //set font. use with "truetype.h"
}

//unicode start bit
uint32_t i = 0x4e9c;
//0x21 number
//0x4e9c //kanji in Japanese fonts.

wchar_t charctor[2] = {i, 0};
void loop() {
  char number[8];

  TFT_fillRect(0, tftWidth, 0, tftHeifht, TFT_background);

  //unicode counter
  sprintf(number, "0x%x", i++);
  ttfout.displayString(10, 10, number, 70, 5);

  //display all unicode charactor
  ttfout.displayString(30, 100, charctor, 200, 5);
  charctor[0]++;

  delay(10);
}

//---TFT
void write_command(uint8_t c) {
  digitalWrite(TFT_DC, LOW);
  TFTspi.transfer(c);
}

void write_data(uint8_t d) {
  digitalWrite(TFT_DC, HIGH);
  TFTspi.transfer(d);
}

void write_data16(uint16_t d) {
  digitalWrite(TFT_DC, HIGH);
  TFTspi.transfer16(d);
}

void TFT_set_rect(uint16_t x1, uint16_t x2, uint16_t y1, uint16_t y2) {
  //Data following CMD_MEMORY_WRITE is written in the area, when a write rectangular area in VRAM is set.
  digitalWrite(TFT_DC, LOW);
  TFTspi.transfer(CMD_COLUMN_ADDRESS_SET);
  digitalWrite(TFT_DC, HIGH);
  TFTspi.transfer16(x1);
  TFTspi.transfer16(x2);
  digitalWrite(TFT_DC, LOW);
  TFTspi.transfer(CMD_PAGE_ADDRESS_SET);
  digitalWrite(TFT_DC, HIGH);
  TFTspi.transfer16(y1);
  TFTspi.transfer16(y2);
  digitalWrite(TFT_DC, LOW);
  TFTspi.transfer(CMD_MEMORY_WRITE);
  digitalWrite(TFT_DC, HIGH);
}

void TFT_fillRect(uint16_t x1, uint16_t x2, uint16_t y1, uint16_t y2, uint16_t color) {
  uint32_t repeat = (x2 - x1 + 1) * (y2 - y1 + 1);
  digitalWrite(TFT_CS, LOW);
  TFT_set_rect(x1, x2, y1, y2);
  for (uint32_t i = 0; i < repeat; i++) {
    TFTspi.transfer16(color);
  }
  digitalWrite(TFT_CS, HIGH);
}
