/*
  TrueType™ Reference Manual
  https://developer.apple.com/fonts/TrueType-Reference-Manual/

  output to SSD1331 sample

*/

#include <SPI.h>
#include <SD.h>

//OLED config
//#define PORTRAIT
#define LANDSCAPE

#define MAC_PORTRAIT 0xe8
#define MAC_LANDSCAPE 0x48

#if defined(PORTRAIT)
#define MAC_CONFIG MAC_PORTRAIT
#define OLED_WIDTH 63
#define OLED_HEIGHT 95
#else
#define MAC_CONFIG MAC_LANDSCAPE
#define OLED_WIDTH 95
#define OLED_HEIGHT 63
#endif

#include "SSD1331_ttf.h"

//OLED config end

//OLED color
#define OLED_background 0b0000000000000000
#define OLED_white 0b1111011111011110
#define OLED_RED 0xF800
#define OLED_yellow 0b1110011110000000
#define OLED_blue 0b0011000110011111
#define OLED_green 0b0011011110000011

#define OLED_NAVY        0x000F      /*   0,   0, 128 */
#define OLED_DARKGREEN   0x03E0      /*   0, 128,   0 */
#define OLED_DARKCYAN    0x03EF      /*   0, 128, 128 */
#define OLED_MAROON      0x7800      /* 128,   0,   0 */
#define OLED_PURPLE      0x780F      /* 128,   0, 128 */
#define OLED_OLIVE       0x7BE0      /* 128, 128,   0 */
#define OLED_LIGHTGREY   0xC618      /* 192, 192, 192 */
#define OLED_DARKGREY    0x7BEF      /* 128, 128, 128 */
#define OLED_CYAN        0x07FF      /*   0, 255, 255 */
#define OLED_RED         0xF800      /* 255,   0,   0 */
#define OLED_MAGENTA     0xF81F      /* 255,   0, 255 */
#define OLED_ORANGE      0xFD20      /* 255, 165,   0 */
#define OLED_GREENYELLOW 0xAFE5      /* 173, 255,  47 */
#define OLED_PINK        0xF81F
//OLED color end

//SPI pin settings
/*
   MOSI PA7
   MISO PA6
   SCK  PA5
   CS   PA4
*/
SPIClass OLEDspi = SPIClass();
ssd1331_ttf ttfout = ssd1331_ttf(&OLEDspi, OLED_WIDTH, OLED_HEIGHT);
enum {
  OLED_CS = PA4,
  OLED_RESET = PA3,
  OLED_DC = PA2,
  SD_CS = PA1
};
//SPI pin settings end

truetypeClass font = truetypeClass(&SD);
truetypeClass font2 = truetypeClass(&SD);
//const char *fontFile2 = "/fonts/ipag.ttf";
//const char *fontFile2 = "/fonts/ipam.ttf";
const char *fontFile2 = "/fonts/hirag.ttf";
//const char *fontFile = "/fonts/helve.ttf";
const char *fontFile = "/fonts/Avenir.ttf";
//const char *fontFile = "/fonts/TNR.ttf";
//const char *fontFile = "/fonts/AppleGaramond.ttf";
//const char *fontFile = "/fonts/myriad.ttf";
//const char *fontFile = "/fonts/Comic.ttf";
//OLED ttf font end

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  delay(10);
  Serial.println(F_CPU);

  //OLED pin settings
  pinMode(OLED_CS, OUTPUT);
  pinMode(OLED_RESET, OUTPUT);
  pinMode(OLED_DC, OUTPUT);
  //OLED pin settings end

  //SPI initilizetion
  OLEDspi.begin();
  OLEDspi.setClockDivider(SPI_CLOCK_DIV2);
  OLEDspi.setDataMode(SPI_MODE3);
  ttfout.optimisationInit(); //use optimisationed SPI(SPI SCK, MOSI, MISO, CS pins cannot be changed from default)
  //SPI initilizetion end

  //LCD initilizetion
  digitalWrite(OLED_RESET, LOW);
  delay(10);
  digitalWrite(OLED_RESET, HIGH);

  OLED_init_CMD();
  OLED_clear(0, OLED_WIDTH, 0, OLED_HEIGHT);
  //LCD initilizetion end

  ttfout.fill_all();
  OLED_fill_rect(20, 50, 30, 40, OLED_white, OLED_background); //fill black
  OLED_fill_rect(30, 40, 20, 35, OLED_RED, OLED_background); //fill black
  OLED_fill_rect(10, 90, 5, 10, OLED_yellow, OLED_background); //fill black
  delay(1000);

  //font begin
  font.begin(SD_CS, fontFile);
  font2.begin(SD_CS, fontFile2);
  Serial.println("read fonts");

  //font output to SSD1331 initialize
  ttfout.setTruetype(&font); //set font. use with "truetype.h"
  ttfout.setSPIpin(OLED_CS, OLED_RESET, OLED_DC); //set SPI for SSD1331 pin (CS, RESET, DC)
  ttfout.setColor(OLED_RED, OLED_white, OLED_background); //set color (inside, outline, background)

  //font output to SSD1331
  ttfout.fill_all();
  ttfout.displayString(5, 0, "123ab", 30, 2); //output string (start_x, start_y, string(char), height of charctor, inter-character space)
  font.end();

  ttfout.setTruetype(&font2); //set font. use with "truetype.h"
  ttfout.displayString(5, 31, "123ab", 30, 2); //output string (start_x, start_y, string(char), height of charctor, inter-character space)
  delay(3000);

  //fill black
  ttfout.fill_all();
  Serial.println("fill black");

  //Character enlargement animation
  ttfout.setColor(OLED_green, OLED_blue, OLED_background); //set color (inside, outline, background)
  for (uint16_t i = 10; i <= 60; i++) {
    ttfout.displayString(0, 0, L"波", i, 0); //output string (start_x, start_y, string(wchar_t, 2byte), height of charctor, inter-character space)
  }

  //fill black
  ttfout.fill_all();
  Serial.println("fill black");

  //count up
  for (int i = 0; i <= 100; i++) {
    char number[4];
    sprintf(number, "%03d", i);
    ttfout.displayMonospaced(2, 5, number, 50, 30); //output string (start_x, start_y, string(char), height of charctor, Monospace value)
  }

  //fill black
  ttfout.fill_all();
  Serial.println("fill black");

  ttfout.setColor(OLED_RED, OLED_white, OLED_background); //set color (inside, outline, background)
}

//unicode start bit
uint32_t i = 0x4e9c;
//0x21 number
//0x4e9c kanji in Japanese fonts.

wchar_t charctor[2] = {i, 0};
void loop() {
  //display all unicode charactor
  ttfout.displayString(25, 2, charctor, 60, 10);
  charctor[0]++;

  //delay(300);
}

//---OLED

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

void OLED_clear(uint8_t _x0, uint8_t _x1, uint8_t _y0, uint8_t _y1) {
  digitalWrite(OLED_CS, LOW);
  digitalWrite(OLED_DC, LOW);

  delay(1);
  OLEDspi.transfer(0x25); //Clear Window
  OLEDspi.transfer(_x0); //Column Address of Start
  OLEDspi.transfer(_y0); //Row Address of Start
  OLEDspi.transfer(_x1); //Column Address of End
  OLEDspi.transfer(_y1); //Row Address of End
  delay(1);

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
  OLEDspi.transfer(OLED_HEIGHT);
  OLEDspi.transfer(CMD_ROW_ADDRESS); //Set Row Address
  OLEDspi.transfer(0);
  OLEDspi.transfer(OLED_HEIGHT);
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
