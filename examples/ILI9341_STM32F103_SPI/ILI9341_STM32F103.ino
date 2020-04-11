/*
  TrueType™ Reference Manual
  https://developer.apple.com/fonts/TrueType-Reference-Manual/

  output to ILI9341 sample

*/

#include <SPI.h>
#include <SD.h>

#include "ILI9341_spi_ttf.h"

//TFT config
//#define PORTRAIT
#define LANDSCAPE

//#define DMA_ENABLE

//TFT color
#define TFT_background 0b0000000000000000
#define TFT_white 0b1111011111011110
#define TFT_RED 0xF800
#define TFT_yellow 0b1110011110000000
#define TFT_blue 0b0011000110011111
#define TFT_green 0b0011011110000011
//TFT color end

//SPI pin settings
SPIClass TFTspi = SPIClass();
ILI9341_spi_ttf ttfout = ILI9341_spi_ttf(&TFTspi);
enum {
  TFT_CS = PA4,
  TFT_RESET = PA3,
  TFT_DC = PA2,
  SD_CS = PA1
};
//SPI pin settings end

truetypeClass ipag = truetypeClass(&SD);
//truetypeClass hiragino = truetypeClass(&SD);
//truetypeClass helvetica = truetypeClass(&SD);
truetypeClass Avenir = truetypeClass(&SD);
//truetypeClass TNR = truetypeClass(&SD);
truetypeClass Myriad = truetypeClass(&SD);
//truetypeClass comic = truetypeClass(&SD);
//truetypeClass Garamond = truetypeClass(&SD);
const char *fontIpag = "/fonts/ipag.ttf";
const char *fontIpam = "/fonts/ipam.ttf";
const char *fontHiragino = "/fonts/hiraginog.ttf";
const char *fontHelvetica = "/fonts/helvetica.ttf";
const char *fontAvenir = "/fonts/Avenir.ttf";
const char *fontTNR = "/fonts/TimesNewRoman.ttf";
const char *fontMyriad = "/fonts/myriad.ttf";
const char *fontComic = "/fonts/ComicSans.ttf";
const char *fontGaramond = "/fonts/AppleGaramond.ttf";
//TFT ttf font end

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  delay(10);
  Serial.println(F_CPU);

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

  digitalWrite(TFT_CS, HIGH);

  //font begin
  ipag.begin(SD_CS, fontIpag);
  //hiragino.begin(SD_CS, fontHiragino);
  //helvetica.begin(SD_CS, fontHelvetica);
  Avenir.begin(SD_CS, fontAvenir);
  //TNR.begin(SD_CS, fontTNR);
  Myriad.begin(SD_CS, fontMyriad);
  //comic.begin(SD_CS, fontComic);
  //Garamond.begin(SD_CS, fontGaramond);
  Serial.println("read fonts");

  //font output to ILI9341 initialize
  //ttfout.setTruetype(&hiragino); //set font. use with "truetype.h"
  ttfout.setTruetype(&ipag); //set font. use with "truetype.h"
  ttfout.setSPIpin(TFT_CS, TFT_RESET, TFT_DC); //set SPI for ILI9341 pin (CS, RESET, DC)
  ttfout.setColor(TFT_RED, TFT_white, TFT_background); //set color (inside, outline, background)
  Serial.println("output ttf initialized");

  //fill black
  ttfout.fill_all();
  Serial.println("fill black");

  //font output to ILI9341
  ttfout.displayString(30, 30, L"埼玉県", 60, 20); //output string (start_x, start_y, string(wchar_t, 2byte), height of charctor, inter-character space)
  ttfout.displayString(30, 90, L"東京都", 60, 20);
  ttfout.displayString(30, 150, L"千葉県", 60, 20);
  ttfout.displayString(30, 210, L"大阪府", 60, 20);
  Serial.println("Hello message");

  delay(2000);

  //fill black
  ttfout.fill_all();
  Serial.println("fill black");

  //font output to ILI9341
  ttfout.displayString(0, 0, "12369ab", 50, 5); //output string (start_x, start_y, string(char), height of charctor, inter-character space)
  //ttfout.setTruetype(&helvetica);
  ttfout.displayString(0, 50, "12369ab", 50, 5);
  ttfout.setTruetype(&Avenir);
  ttfout.displayString(0, 100, "12369ab", 50, 5);
  //ttfout.setTruetype(&TNR);
  ttfout.displayString(0, 150, "12369ab", 50, 5);
  ttfout.setTruetype(&Myriad);
  ttfout.displayString(0, 200, "12569ab", 50, 5);
  /*
    ttfout.setTruetype(&Garamond); //read error
    ttfout.displayString(0, 300, "12569ab", 50, 5);
    ttfout.setTruetype(&comic); //read error
    ttfout.displayString(0, 250, "12569ab", 50, 5);
  */
  Serial.println("Display in various fonts");

  delay(1000);

  //fill black
  ttfout.fill_all();
  Serial.println("fill black");

  ttfout.setTruetype(&ipag); //set font. use with "truetype.h"

  //count up
  for (int i = 0; i <= 100; i++) {
    char number[4];
    sprintf(number, "%03d", i);
    ttfout.displayMonospaced(20, 20, number, 100, 60); //output string (start_x, start_y, string(char), height of charctor, Monospace value)
  }

  //fill black
  ttfout.fill_all();
  Serial.println("fill black");
}

//unicode start bit
uint32_t i = 0x21;
//0x21 number
//0x4e9c kanji in Japanese fonts.

wchar_t charctor[2] = {i, 0};
void loop() {
  char number[8];

  ttfout.fill_all();

  //unicode counter
  sprintf(number, "0x%x", i++);
  ttfout.displayString(10, 10, number, 70, 5);

  //display all unicode charactor
  ttfout.displayString(30, 100, charctor, 200, 5);
  charctor[0]++;

  //delay(300);
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
