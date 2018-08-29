/*
  TrueType™ Reference Manual
  https://developer.apple.com/fonts/TrueType-Reference-Manual/

  output to SSD1331 sample

*/

#include <FS.h>
#include <SPI.h>
#include <SD.h>

#include "SSD1331_ttf.h"

//OLED command list
//#define PORTRAIT
#define LANDSCAPE

#define CMD_DISPLAY_OFF 0xAE
#define CMD_DISPLAY_ON 0xAF
#define CMD_COLUMN_ADDRESS 0x15
#define CMD_ROW_ADDRESS 0x75
#define CMD_SET_POWER_SAVE_MODE 0xB0
#define CMD_POWER_SAVE_EN 0x1A
#define CMD_SET_START_LINE 0xA1
#define CMD_SET_OFFSET 0xA2
#define CMD_SET_MODE_NORMAL 0xA4

#define MAC_PORTRAIT 0xe8
#define MAC_LANDSCAPE 0x48

#if defined(PORTRAIT)
#define MAC_CONFIG MAC_PORTRAIT
#define OLEDWidth 63
#define OLEDHeifht 95
#else
#define MAC_CONFIG MAC_LANDSCAPE
#define OLEDWidth 95
#define OLEDHeifht 63
#endif
//OLED command list end

//OLED color
#define OLED_background 0b0000000000000000
#define OLED_white 0b1111011111011110
#define OLED_RED 0xF800
#define OLED_yellow 0b1110011110000000
#define OLED_blue 0b0011000110011111
#define OLED_green 0b0011011110000011
//OLED color end

//SPI pin settings
SPIClass OLEDspi = SPIClass(VSPI); //Both HSPI and VSPI are available
ssd1331_ttf ttfout = ssd1331_ttf(&OLEDspi);
enum {
  OLED_CS = 27,
  OLED_RESET = 26,
  OLED_DC = 25,
  SD_CS = 5
};
//SPI pin settings end

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
  OLEDspi.setFrequency(7000000);
  OLEDspi.setDataMode(SPI_MODE3);
  //SPI initilizetion end

  //OLED initilizetion
  digitalWrite(OLED_RESET, LOW);
  delay(1);
  digitalWrite(OLED_RESET, HIGH);

  OLED_init_CMD();
  //LCD initilizetion end

  //fill black
  OLED_fill_rect(0, OLEDWidth, 0, OLEDHeifht, OLED_background, OLED_background);

  font.begin(SD_CS, fontFile);
  font2.begin(SD_CS, fontFile2);

  ttfout.setTruetype(&font); //set font. use with "truetype.h"
  ttfout.setSPIpin(OLED_CS, OLED_RESET, OLED_DC); //set SPI for ILI9341 pin (CS, RESET, DC)
  ttfout.setColor(OLED_RED, OLED_white, OLED_background); //set color (inside, outline, background)

  ttfout.displayString(5, 0, "123ab", 30, 2); //output string (start_x, start_y, string(char), height of charctor, inter-character space)
  font.end();

  ttfout.setTruetype(&font2); //set font. use with "truetype.h"
  ttfout.displayString(5, 31, "123ab", 30, 2); //output string (start_x, start_y, string(char), height of charctor, inter-character space)
  delay(3000);

  //fill black
  OLED_fill_rect(0, OLEDWidth, 0, OLEDHeifht, OLED_background, OLED_background);

  //display kanji
  ttfout.displayString(1, 0, L"埼玉県", 40, 2); //output string (start_x, start_y, string(wchar_t, 2byte), height of charctor, inter-character space)
  delay(1000);

  //scroll using copy
  digitalWrite(OLED_CS, LOW);
  for (uint8_t i = 0; i < 5; i++) {
    for (uint8_t offset_y = 0; offset_y < OLEDHeifht; offset_y++) {
      OLED_copy(0, OLEDWidth, offset_y, offset_y + 40, 0, offset_y + 1);
      delay(10);
    }
  }
  digitalWrite(OLED_CS, HIGH);
  delay(1000);

  //fill black
  OLED_fill_rect(0, OLEDWidth, 0, OLEDHeifht, OLED_background, OLED_background);
  
  //count up
  for (int i = 0; i <= 100; i++) {
    char number[4];
    sprintf(number, "%03d", i);
    ttfout.displayMonospaced(2, 5, number, 50, 30); //output string (start_x, start_y, string(char), height of charctor, Monospace value)
  }
  
  //fill black
  OLED_fill_rect(0, OLEDWidth, 0, OLEDHeifht, OLED_background, OLED_background);
}

//unicode start bit
uint32_t i = 0x4e9c;
//0x21 number
//0x4e9c //kanji in Japanese fonts.
wchar_t charctor[2] = {i, 0};

void loop() {
  //display all unicode charactor
  ttfout.displayString(25, 2, charctor, 60, 0);
  charctor[0]++;

  delay(10);
}

//OLED
void OLED_set_rect(uint8_t x1, uint8_t x2, uint8_t y1, uint8_t y2) {
  digitalWrite(OLED_DC, LOW);
  OLEDspi.transfer(CMD_COLUMN_ADDRESS);
  OLEDspi.transfer(x1);
  OLEDspi.transfer(x2);
  OLEDspi.transfer(CMD_ROW_ADDRESS);
  OLEDspi.transfer(y1);
  OLEDspi.transfer(y2);
  digitalWrite(OLED_DC, HIGH);
}

void OLED_fill_rect(uint8_t _x1, uint8_t _x2, uint8_t _y1, uint8_t _y2, uint16_t _outline, uint16_t _innner) {
  rgb16_t color;
  digitalWrite(OLED_CS, LOW);
  digitalWrite(OLED_DC, LOW);

  OLEDspi.transfer(0x26);
  OLEDspi.transfer(0b00000001);

  OLEDspi.transfer(0x22);
  OLEDspi.transfer(_x1);
  OLEDspi.transfer(_y1);
  OLEDspi.transfer(_x2);
  OLEDspi.transfer(_y2);

  color.raw = _outline;
  OLEDspi.transfer(color.rgb.b << 1);
  OLEDspi.transfer(color.rgb.g);
  OLEDspi.transfer(color.rgb.r << 1);

  color.raw = _innner;
  OLEDspi.transfer(color.rgb.b << 1);
  OLEDspi.transfer(color.rgb.g);
  OLEDspi.transfer(color.rgb.r << 1);

  digitalWrite(OLED_DC, HIGH);
  digitalWrite(OLED_CS, HIGH);
}

void OLED_copy(uint8_t x0, uint8_t x1, uint8_t y0, uint8_t y1, uint8_t X, uint8_t Y) {
  digitalWrite(OLED_CS, LOW);
  digitalWrite(OLED_DC, LOW);
  OLEDspi.transfer(0x23);

  //from
  OLEDspi.transfer(x0);
  OLEDspi.transfer(y0);
  OLEDspi.transfer(x1);
  OLEDspi.transfer(y1);

  //to
  OLEDspi.transfer(X);
  OLEDspi.transfer(Y);

  digitalWrite(OLED_DC, HIGH);
  digitalWrite(OLED_CS, HIGH);
}

void OLED_init_CMD() {
  digitalWrite(OLED_CS, LOW);
  digitalWrite(OLED_DC, LOW);

  OLEDspi.transfer(CMD_DISPLAY_OFF); //Set Display Off
  OLEDspi.transfer(0xA0); //Remap & Color Depth setting　
  OLEDspi.transfer(0b01110010); //A[7:6] = 00; 256 color. A[7:6] = 01; 65k color format
  OLEDspi.transfer(CMD_SET_START_LINE); //Set Display Start Line
  OLEDspi.transfer(0);
  OLEDspi.transfer(CMD_SET_OFFSET); //Set Display Offset
  OLEDspi.transfer(0);
  OLEDspi.transfer(CMD_SET_MODE_NORMAL); //Set Display Mode (Normal)
  OLEDspi.transfer(0xA8); //Set Multiplex Ratio
  OLEDspi.transfer(63); //15-63
  OLEDspi.transfer(0xAD); //Set Master Configration
  OLEDspi.transfer(0b10001110); //a[0]=0 Select external Vcc supply, a[0]=1 Reserved(reset)
  OLEDspi.transfer(CMD_SET_POWER_SAVE_MODE); //Power Save Mode
  OLEDspi.transfer(CMD_POWER_SAVE_EN); //0x1A Enable power save mode. 0x00 Disable
  OLEDspi.transfer(0xB1); //Phase 1 and 2 period adjustment
  OLEDspi.transfer(0x74);
  OLEDspi.transfer(0xB3); //Display Clock DIV
  OLEDspi.transfer(0xF0);
  OLEDspi.transfer(0x8A); //Pre Charge A
  OLEDspi.transfer(0x81);
  OLEDspi.transfer(0x8B); //Pre Charge B
  OLEDspi.transfer(0x82);
  OLEDspi.transfer(0x8C); //Pre Charge C
  OLEDspi.transfer(0x83);
  OLEDspi.transfer(0xBB); //Set Pre-charge level
  OLEDspi.transfer(0x3A);
  OLEDspi.transfer(0xBE); //Set VcomH
  OLEDspi.transfer(0x3E);
  OLEDspi.transfer(0x87); //Set Master Current Control
  OLEDspi.transfer(0x06);
  OLEDspi.transfer(CMD_COLUMN_ADDRESS); //Set Column Address
  OLEDspi.transfer(0);
  OLEDspi.transfer(OLEDWidth);
  OLEDspi.transfer(CMD_ROW_ADDRESS); //Set Row Address
  OLEDspi.transfer(0);
  OLEDspi.transfer(OLEDHeifht);
  OLEDspi.transfer(0x81); //Set Contrast for Color A
  OLEDspi.transfer(255);
  OLEDspi.transfer(0x82); //Set Contrast for Color B
  OLEDspi.transfer(255);
  OLEDspi.transfer(0x83); //Set Contrast for Color C
  OLEDspi.transfer(255);
  OLEDspi.transfer(CMD_DISPLAY_ON); //Set Display On

  digitalWrite(OLED_DC, HIGH);
  digitalWrite(OLED_CS, HIGH);

  delay(150); //datasheet required after command 0xAF(display on)
}
