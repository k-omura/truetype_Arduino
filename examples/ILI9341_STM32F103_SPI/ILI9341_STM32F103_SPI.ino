/*
  TrueType™ Reference Manual
  https://developer.apple.com/fonts/TrueType-Reference-Manual/

  output to ILI9341 sample

*/

#include <SPI.h>
#include <SD.h>

//TFT config
//#define PORTRAIT
#define LANDSCAPE

#if defined(PORTRAIT)
#define MAC_CONFIG MAC_PORTRAIT
#define TFT_WIDTH 320U
#define TFT_HEIGHT 240U
#define AdafruitTFTOrientation 1
#else
#define MAC_CONFIG MAC_LANDSCAPE
#define TFT_WIDTH 240U
#define TFT_HEIGHT 320U
#define AdafruitTFTOrientation 0
#endif

#include "ILI9341_spi_ttf.h"
//TFT config end

//TFT color
#define TFT_background 0b0000000000000000
#define TFT_white 0b1111011111011110
#define TFT_RED 0xF800
#define TFT_yellow 0b1110011110000000
#define TFT_blue 0b0011000110011111
#define TFT_green 0b0011011110000011

#define TFT_NAVY        0x000F      /*   0,   0, 128 */
#define TFT_DARKGREEN   0x03E0      /*   0, 128,   0 */
#define TFT_DARKCYAN    0x03EF      /*   0, 128, 128 */
#define TFT_MAROON      0x7800      /* 128,   0,   0 */
#define TFT_PURPLE      0x780F      /* 128,   0, 128 */
#define TFT_OLIVE       0x7BE0      /* 128, 128,   0 */
#define TFT_LIGHTGREY   0xC618      /* 192, 192, 192 */
#define TFT_DARKGREY    0x7BEF      /* 128, 128, 128 */
#define TFT_CYAN        0x07FF      /*   0, 255, 255 */
#define TFT_RED         0xF800      /* 255,   0,   0 */
#define TFT_MAGENTA     0xF81F      /* 255,   0, 255 */
#define TFT_ORANGE      0xFD20      /* 255, 165,   0 */
#define TFT_GREENYELLOW 0xAFE5      /* 173, 255,  47 */
#define TFT_PINK        0xF81F
//TFT color end

//SPI pin settings
/*
   MOSI PA7
   MISO PA6
   SCK  PA5
   CS   PA4
*/
SPIClass TFTspi = SPIClass();
ILI9341_spi_ttf ttfout = ILI9341_spi_ttf(&TFTspi, TFT_WIDTH, TFT_HEIGHT);
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
  ttfout.optimisationInit(); //use optimisationed SPI(SPI SCK, MOSI, MISO, CS pins cannot be changed from default)
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
  ttfout.displayString(35, 50, "Hello", 100, 10);
  Serial.println("Hello message");

  delay(1500);

  //fill black
  ttfout.fill_all();
  Serial.println("fill black");

  //Character enlargement animation
  ttfout.setColor(TFT_green, TFT_blue, TFT_background); //set color (inside, outline, background)
  for (uint16_t i = 10; i <= 100; i++) {
    ttfout.displayString(5, 5, L"波", i, 0); //output string (start_x, start_y, string(wchar_t, 2byte), height of charctor, inter-character space)
  }

  //fill black
  ttfout.fill_all();
  Serial.println("fill black");

  //font output to ILI9341
  ttfout.displayString(0, 0, "12369ab", 50, 5); //output string (start_x, start_y, string(char), height of charctor, inter-character space)
  //ttfout.setTruetype(&helvetica);
  ttfout.setColor(TFT_DARKCYAN, TFT_white, TFT_background);
  ttfout.displayString(0, 50, "12369ab", 50, 5);
  ttfout.setTruetype(&Avenir);
  ttfout.setColor(TFT_yellow, TFT_white, TFT_background);
  ttfout.displayString(0, 100, "12369ab", 50, 5);
  //ttfout.setTruetype(&TNR);
  ttfout.setColor(TFT_blue, TFT_white, TFT_background);
  ttfout.displayString(0, 150, "12369ab", 50, 5);
  ttfout.setTruetype(&Myriad);
  ttfout.setColor(TFT_ORANGE, TFT_white, TFT_background);
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

  //count up
  ttfout.setTruetype(&Avenir);
  for (int i = 0; i <= 100; i++) {
    char number[4];
    sprintf(number, "%03d", i);
    ttfout.displayMonospaced(20, 20, number, 100, 60); //output string (start_x, start_y, string(char), height of charctor, Monospace value)
  }

  //fill black
  ttfout.fill_all();
  Serial.println("fill black");

  ttfout.setTruetype(&ipag);
  ttfout.setColor(TFT_RED, TFT_white, TFT_background); //set color (inside, outline, background)
  //font end
  //ipag.end();
  //hiragino.end();
  //helvetica.end();
  Avenir.end();
  //TNR.end();
  Myriad.end();
  //comic.end();
  //Garamond.end();
}

//unicode start bit
uint32_t i = 0x4e9c;
//0x21 number
//0x4e9c kanji in Japanese fonts.

wchar_t charctor[2] = {i, 0};
void loop() {
  char number[8];

  ttfout.fill_all();

  //unicode counter
  sprintf(number, "0x%04X", i++);
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
